/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define FRAME_TO_SYNC 1

#define FLASH_USER_START_ADDR   (FLASH_BASE + (32 * FLASH_PAGE_SIZE))   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (FLASH_BASE + FLASH_SIZE - 1)   /* End @ of user Flash area */

#define DATA_64                 ((uint64_t)0x1234567812345678)
#define DATA_32                 ((uint32_t)0x12345678)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t ntcRaw=0;

uint8_t  I2C_cmd[2];

uint8_t  I2C_IF_TxStatus=0;
uint8_t  I2C_IF_RxBuffer[2];
uint8_t  I2C_IF_TxBuffer[968];

uint8_t  sync=0;
uint8_t  readyFrame=1;
uint16_t image[2][484];

uint8_t  avgCounter=0;
uint32_t buffer[484];
uint16_t image_FIFO[484];

// for debug
uint16_t p=1,pp=0;
uint16_t std[22];
uint8_t  state;
uint8_t  I2C_IF_Restart=0;
uint16_t I2C_IF_SCL_LEVEL;
uint32_t data[10];
uint64_t src[5]={\
	0x0000000000000000,\
	0x1111111111111111,\
	0x2222222222222222,\
	0x3333333333333333,\
	0x4444444444444444};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/*------- Callback function block start  -------*/

/* I2C_IF callbacks to handle I2C Slave interface */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c==&hi2c2){
		HAL_I2C_EnableListen_IT(hi2c);
	}
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if(hi2c==&hi2c2){
//  	__HAL_I2C_CLEAR_FLAG(hi2c,I2C_FLAG_ADDR);
		if(TransferDirection==I2C_DIRECTION_TRANSMIT){
			HAL_I2C_Slave_Seq_Receive_IT(hi2c,I2C_IF_RxBuffer,2,I2C_FIRST_AND_NEXT_FRAME);
		}
		else{
			uint16_t cmd=I2C_IF_RxBuffer[0];//(I2C_IF_RxBuffer[0]<<8)+I2C_IF_RxBuffer[1];
			if(cmd<44){	// 11 pixels
				HAL_I2C_Slave_Seq_Transmit_IT(hi2c,&I2C_IF_TxBuffer[cmd*22],24,I2C_FIRST_FRAME);
			}
			else if(cmd==100){	// one full frame
				HAL_I2C_Slave_Seq_Transmit_IT(hi2c,I2C_IF_TxBuffer,968,I2C_FIRST_FRAME);
//				HAL_I2C_Slave_Transmit_DMA(hi2c,I2C_IF_TxBuffer,968);
			}
			else{
				I2C_IF_TxBuffer[0]=0xEE;
				HAL_I2C_Slave_Seq_Transmit_IT(hi2c,I2C_IF_TxBuffer,1,I2C_FIRST_FRAME);
			}
		}
	}
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
//	HAL_I2C_Slave_Transmit_IT(&hi2c2, I2C_IF_TxBuffer, 2);
//	HAL_I2C_Slave_Transmit_DMA(&hi2c2, I2C_IF_TxBuffer, 22);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
//	HAL_I2C_Slave_Receive_IT(&hi2c2, I2C_IF_RxBuffer, 1); // Size set 1 for Slave_Transmit_DMA, set 2 for Slave_Transmit_IT
//	HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C_IF_TxBuffer, 22, I2C_FIRST_FRAME);

	if(hi2c==&hi2c2){
		/* Copy 8-frames average image to I2C_IF transmit buffer */
		for(uint16_t i=0;i<484;i++){
			I2C_IF_TxBuffer[i*2+1]=(uint8_t)image_FIFO[i];
			I2C_IF_TxBuffer[i*2]=(uint8_t)(image_FIFO[i]>>8);
		}
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
//	HAL_GPIO_TogglePin(test_GPIO_Port,test_Pin);
}

/* VSYNC callback to handdle frame synchronize */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==VSYNC_Pin){
		if(sync>FRAME_TO_SYNC){
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
			HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
			HAL_SPI_Receive_DMA(&hspi1,(uint8_t*)image,sizeof(image)/2);
			HAL_NVIC_DisableIRQ(VSYNC_EXTI_IRQn);
		}
		sync++;
	}
}
/*-------- Callback function block end  --------*/

/* debug callbacks zone */

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//	if(htim==&htim3){
//		HAL_GPIO_TogglePin(test_GPIO_Port,test_Pin);
//	}
//}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	if(htim==&htim3){
//		__HAL_TIM_SetCounter(htim,0);
		if(HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_2)>199){
			HAL_GPIO_TogglePin(test_GPIO_Port,test_Pin);
//			I2C_IF_Restart=1;
		}
	}
}

/*---------- Sub-function block start ----------*/

/* frameRate only 50 and 100 fps so far */
void GTM016AN_Init(uint8_t frameRate){
	I2C_cmd[0]=0x09;I2C_cmd[1]=0x5a;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x0e;I2C_cmd[1]=0x30;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x12;I2C_cmd[1]=0xea;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x13;I2C_cmd[1]=0x0;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x14;I2C_cmd[1]=0x02;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x15;I2C_cmd[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x19;I2C_cmd[1]=0x04;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x26;I2C_cmd[1]=0x03;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
//	I2C_cmd[0]=0x27;I2C_cmd[1]=0x1f;
	I2C_cmd[0]=0x27;I2C_cmd[1]=0x1d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x3c;I2C_cmd[1]=0x8f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x3d;I2C_cmd[1]=0x92;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x3e;I2C_cmd[1]=0x0f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x3f;I2C_cmd[1]=0x0f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x40;I2C_cmd[1]=0x05;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x42;I2C_cmd[1]=0x0c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x44;I2C_cmd[1]=0x01;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x47;I2C_cmd[1]=0x7c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x48;I2C_cmd[1]=0x06;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x4a;I2C_cmd[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x4b;I2C_cmd[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x4c;I2C_cmd[1]=0x0e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x4d;I2C_cmd[1]=0x0e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x50;I2C_cmd[1]=0x02;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x52;I2C_cmd[1]=0x80;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x53;I2C_cmd[1]=0x7c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x54;I2C_cmd[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x55;I2C_cmd[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x5a;I2C_cmd[1]=0x44;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x5d;I2C_cmd[1]=0x89;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x5d;I2C_cmd[1]=0xa7;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x60;I2C_cmd[1]=0xb7;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x26;I2C_cmd[1]=0x08;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	I2C_cmd[0]=0x0e;I2C_cmd[1]=0x3e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);

	if(frameRate==50){
		I2C_cmd[0]=0x14;I2C_cmd[1]=0x04;
		HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	}
	else if(frameRate==100){
		I2C_cmd[0]=0x14;I2C_cmd[1]=0x02;
		HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
	}

	I2C_cmd[0]=0x09;I2C_cmd[1]=0x0;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,I2C_cmd,2,10);
}

/* The following four subfunction for flash access use */
static uint32_t GetPage(uint32_t Addr){
  return((Addr-FLASH_BASE)/FLASH_PAGE_SIZE);
}

void Flash_Erase(uint32_t pAddress,uint16_t pSize){
	static FLASH_EraseInitTypeDef EraseInitStruct={0};
	uint32_t PageError=0;
	if((pAddress>(FLASH_USER_END_ADDR-FLASH_USER_START_ADDR)/FLASH_PAGE_SIZE)\
		||(pSize>(FLASH_USER_END_ADDR-FLASH_USER_START_ADDR)/FLASH_PAGE_SIZE)){
		return;
	}
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page      = GetPage(FLASH_USER_START_ADDR)+pAddress;
  EraseInitStruct.NbPages   = pSize;

	HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&EraseInitStruct,&PageError);
	HAL_FLASH_Lock();
}

void Flash_Write(uint32_t address,uint64_t *data,uint16_t dSize){
	uint32_t StartAddr=address;
	uint32_t EndAddr=address+dSize*8;
	if((StartAddr<FLASH_USER_START_ADDR)||(EndAddr>FLASH_USER_END_ADDR)){
		return;
	}
	HAL_FLASH_Unlock();
	for(uint16_t i=0;i<dSize;i++){
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,(StartAddr+i*8),data[i])==HAL_OK){
    }
	}
	HAL_FLASH_Lock();
}

void Flash_Read(uint32_t address,__IO uint32_t * data,uint16_t dSize){
	for(uint32_t addr=address;addr<(address+dSize*4);addr+=4){
		*data=*(__IO uint32_t *)addr;
		data++;
	}
}
/*---------- Subfunction block end    ----------*/


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */


	/* Disable VSYNC interrupt to initial GTM016AN */
	HAL_NVIC_DisableIRQ(VSYNC_EXTI_IRQn);
	GTM016AN_Init(100);
	HAL_Delay(1000);
	HAL_NVIC_EnableIRQ(VSYNC_EXTI_IRQn);

//	for(uint16_t i=0;i<484;i++){
//		I2C_IF_TxBuffer[i*2]=i/256;
//		I2C_IF_TxBuffer[i*2+1]=i%256;
//	}

	HAL_I2C_EnableListen_IT(&hi2c2);
//	HAL_I2C_Slave_Seq_Receive_IT(&hi2c2, I2C_IF_RxBuffer, 1, I2C_LAST_FRAME);
//	HAL_I2C_Slave_Receive_IT(&hi2c2, I2C_IF_RxBuffer, 1);


//	Flash_Erase(0,32);
//	Flash_Write(FLASH_USER_START_ADDR,src,5);
//	Flash_Read(FLASH_USER_START_ADDR,(uint32_t*)data,10);


	// HAL_TIM_Base_Start_IT();
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim14,TIM_CHANNEL_1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(pp){
			MX_TIM14_Init();
			HAL_TIM_PWM_Start(&htim14,TIM_CHANNEL_1);
			pp=0;
		}
		
		if(I2C_IF_Restart==1){
			HAL_I2C_DeInit(&hi2c2);
			HAL_I2C_Init(&hi2c2);
			HAL_I2C_EnableListen_IT(&hi2c2);
			I2C_IF_Restart=0;
		}
//		HAL_ADC_Start(&hadc1);
//		HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);
//		ntcRaw=HAL_ADC_GetValue(&hadc1);
		state=HAL_I2C_GetState(&hi2c2);
		HAL_Delay(10);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC
                              |RCC_PERIPHCLK_TIM1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
