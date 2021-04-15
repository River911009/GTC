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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t  sync=0;
uint16_t ntcRaw=0;
uint8_t  commands[2];

uint8_t  I2C_IF_RxBuffer[2];
uint8_t  I2C_IF_TxBuffer[44];

uint8_t  readyFrame=1;
uint16_t image[2][484];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	HAL_GPIO_TogglePin(test_GPIO_Port,test_Pin);
//	HAL_I2C_EnableListen_IT(hi2c); // slave is ready again
//}

//void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
//{
////	HAL_GPIO_WritePin( test_GPIO_Port, test_Pin, GPIO_PIN_SET );
////	if( TransferDirection==I2C_DIRECTION_TRANSMIT ) {
//////		if( first ) {
////			HAL_I2C_Slave_Receive_IT(hi2c,I2C_IF_RxBuffer,2);
//////		} else {
//////			HAL_I2C_Slave_Seq_Receive_IT(hi2c, &ram[offset], 1, I2C_NEXT_FRAME);
//////		}
////	} else {
////		HAL_I2C_Slave_Transmit_IT(hi2c,I2C_IF_RxBuffer,2);
////	}
////	HAL_GPIO_WritePin( test_GPIO_Port, test_Pin, GPIO_PIN_RESET );
//}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
//	HAL_I2C_Slave_Transmit_IT(&hi2c2, I2C_IF_TxBuffer, 2);
	HAL_I2C_Slave_Transmit_DMA(&hi2c2, I2C_IF_TxBuffer, 22);
	
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_Slave_Receive_IT(&hi2c2, I2C_IF_RxBuffer, 1); // Size set 1 for Slave_Transmit_DMA, set 2 for Slave_Transmit_IT
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==VSYNC_Pin){
		if(sync>1){
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
			HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
			HAL_SPI_Receive_DMA(&hspi1,(uint8_t*)image,sizeof(image)/2);
			HAL_NVIC_DisableIRQ(VSYNC_EXTI_IRQn);
		}
		sync++;
	}
}

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
  /* USER CODE BEGIN 2 */


	// Disable VSYNC interrupt
	HAL_NVIC_DisableIRQ(VSYNC_EXTI_IRQn);

	// GTM016AN 100fps setting script
	commands[0]=0x09;commands[1]=0x5a;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x0e;commands[1]=0x30;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x12;commands[1]=0xea;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x13;commands[1]=0x0;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x14;commands[1]=0x02;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x15;commands[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x19;commands[1]=0x04;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x26;commands[1]=0x03;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x27;commands[1]=0x1d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x3c;commands[1]=0x8f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x3d;commands[1]=0x92;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x3e;commands[1]=0x0f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x3f;commands[1]=0x0f;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x40;commands[1]=0x05;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x42;commands[1]=0x0c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x44;commands[1]=0x01;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x47;commands[1]=0x7c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x48;commands[1]=0x06;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x4a;commands[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x4b;commands[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x4c;commands[1]=0x0e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x4d;commands[1]=0x0e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x50;commands[1]=0x02;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x52;commands[1]=0x80;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x53;commands[1]=0x7c;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x54;commands[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x55;commands[1]=0x0d;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x5a;commands[1]=0x44;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x5d;commands[1]=0x89;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x5d;commands[1]=0xa7;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x60;commands[1]=0xb7;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x26;commands[1]=0x08;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x0e;commands[1]=0x3e;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);
	commands[0]=0x09;commands[1]=0x0;
	HAL_I2C_Master_Transmit(&hi2c1,0x40,commands,2,10);

	HAL_Delay(1000);
	HAL_NVIC_EnableIRQ(VSYNC_EXTI_IRQn);

//	HAL_I2C_EnableListen_IT(&hi2c2);
	HAL_I2C_Slave_Receive_IT(&hi2c2, I2C_IF_RxBuffer, 1);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		HAL_ADC_Start(&hadc1);
//		HAL_ADC_PollForConversion(&hadc1,HAL_MAX_DELAY);
//		ntcRaw=HAL_ADC_GetValue(&hadc1);
		HAL_Delay(1000);

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
