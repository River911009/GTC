/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_cdc_if.h"
#include "UI.h"

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


uint8_t shift_=10,direct_=0;


ADC_SRC ADC_select=ADC_External;//Test;
extern frame_BufferTypeDef img;
extern ui_TypeDef ui;

USER_CDC_PACKAGE_TYPE_t user_cdc_buffer;	// cdc package buffer
uint8_t commands[4];	// cdc command buffer


// i2c control value buffer
uint8_t ctrlBuffer[100];	// i2c read/write all (addr 0 to 99)
uint16_t imageValue[22];

// current frame size
uint8_t frameSizeWidth=22;
uint8_t frameSizeHeight=22;

// 2 times image buffer from spi
uint8_t imagBuffer[2*22*22*2+2];	// double buffer * 22x22 pixels * 2bytes/pxl

// image pointer
uint8_t imgRead=0;
// double frame buffer selector
uint8_t frameReady=1;
uint8_t ready=0;
// uint8_t sync=0;


																	

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	for(uint16_t i=0;i<484;i+=23){
		imageValue[i/23]=img.image[0][i];
	}
}

// first read frame size on system startup
void getFrameSize(){
  commands[0]=28;
  HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,1,10);
  HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[28],1,10);
  commands[0]=30;
  HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,1,10);
  HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[30],1,10);
  commands[0]=32;
  HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,1,10);
  HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[32],1,10);
  commands[0]=34;
  HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,1,10);
  HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[34],1,10);

  ctrlBuffer[32]=21;
  ctrlBuffer[34]=21;
  ctrlBuffer[28]=0;
  ctrlBuffer[30]=0;

  if(0<ctrlBuffer[32]-ctrlBuffer[28]+1 && ctrlBuffer[32]-ctrlBuffer[28]+1<23){
    frameSizeWidth=ctrlBuffer[32]-ctrlBuffer[28]+1;
  }
  if(0<ctrlBuffer[34]-ctrlBuffer[30]+1 && ctrlBuffer[34]-ctrlBuffer[30]+1<23){
    frameSizeHeight=ctrlBuffer[34]-ctrlBuffer[30]+1;
  }
}


//--------------
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin==GTM016AN_PCLK_Pin){
		HAL_TIM_PWM_Start_IT(&htim4,TIM_CHANNEL_2);
	}
  if(GPIO_Pin==GTM016AN_VSYNC_Pin){
		if(ADC_select==ADC_Internal){
			HAL_SPI_Receive_DMA(&hspi4,(uint8_t*)img.image,484);
			HAL_NVIC_DisableIRQ(GTM016AN_VSYNC_EXTI_IRQn);
		}
		else if(ADC_select==ADC_External){
			HAL_SPI_DeInit(&hspi4);
			// preset external adc pointer
			img.pixel=483;	// PCLK rising after Vsync fell is pixel (21,21)
		}
  }
  if(GPIO_Pin==B1_Pin){
		// change source
		ADC_select=(ADC_select<2)? (ADC_select+1):ADC_Test;
		// update ADC source
		setADC(ADC_select);
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
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_SPI3_Init();
  MX_SPI4_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */


	// stop external adc cnv clock and pclk
	HAL_NVIC_DisableIRQ(GTM016AN_VSYNC_EXTI_IRQn);
	HAL_NVIC_DisableIRQ(GTM016AN_PCLK_EXTI_IRQn);
	HAL_GPIO_WritePin(GTM016AN_nRST_GPIO_Port,GTM016AN_nRST_Pin,GPIO_PIN_SET);
	UI_Initial(LOGO_COLOUR,FONT_COLOUR,BACK_COLOUR);
	setADC(ADC_select);
	// Fake PCLK clock generator
//	HAL_TIM_Base_Start_IT(&htim3);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    // cdc command handler
		if(user_cdc_buffer.len>0){
      switch(user_cdc_buffer.data[0]){
      // read frame size
        case 10:
          if(ctrlBuffer[32]-ctrlBuffer[28]+1>0){
            frameSizeWidth=ctrlBuffer[32]-ctrlBuffer[28]+1;
          }
          commands[0]=10; //ack
          commands[1]=frameSizeWidth;
          commands[2]=ctrlBuffer[28];
          commands[3]=ctrlBuffer[32];
          CDC_Transmit_HS(commands,4);
          break;
        case 11:
          if(ctrlBuffer[34]-ctrlBuffer[30]+1>0){
            frameSizeHeight=ctrlBuffer[34]-ctrlBuffer[30]+1;
          }
          commands[0]=11; //ack
          commands[1]=frameSizeHeight;
          commands[2]=ctrlBuffer[30];
          commands[3]=ctrlBuffer[34];
          CDC_Transmit_HS(commands,4);
          break;
      // read imgBuffer
        case 17:
          imgRead=1;
          break;
      // read calibration
        case 20:
          imgRead=2;
          break;
      /**
       * i2c write special address
       *   cdc => ctrlBuffer[x]
       *   ack + ctrlBuffer[x] => cdc ack(1 bytes)
       */
        case 33:
          ctrlBuffer[user_cdc_buffer.data[1]]=user_cdc_buffer.data[2];
          commands[0]=user_cdc_buffer.data[1];
          commands[1]=user_cdc_buffer.data[2];
          HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
          if(commands[0]==18\
          || commands[0]==19\
          || commands[0]==20\
          || commands[0]==21\
          || commands[0]==28\
          || commands[0]==30\
          || commands[0]==32\
          || commands[0]==34){
            if(ctrlBuffer[32]-ctrlBuffer[28]+1>0){
              frameSizeWidth=ctrlBuffer[32]-ctrlBuffer[28]+1;
            }
            if(ctrlBuffer[34]-ctrlBuffer[30]+1>0){
              frameSizeHeight=ctrlBuffer[34]-ctrlBuffer[30]+1;
            }
          }

          // user_cdc_buffer.data[0]=ack;
          CDC_Transmit_HS(user_cdc_buffer.data,1);
          break;
      /**
       * i2c write all
       *   cdc => ctrlBuffer => I2C(maximum 256 bytes)
       *   ack => cdc 36(1byte)
       */
        case 36:
          // ctrlBuffer[user_cdc_buffer.data[1]]=user_cdc_buffer.data[2];
          // user_cdc_buffer.data[0]=ack;
          CDC_Transmit_HS(user_cdc_buffer.data,1);
          break;
      /**
       * i2c read special address
       *   I2C => ctrlBuffer[x]
       *   ack + ctrlBuffer[x] => cdc ack+data(2 bytes)
       */
        case 97:
          // user_cdc_buffer.data[0]=ack=>97;
          HAL_I2C_Master_Transmit(&hi2c3,0x40,&user_cdc_buffer.data[1],1,10);
          HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[user_cdc_buffer.data[1]],1,10);
          user_cdc_buffer.data[1]=ctrlBuffer[user_cdc_buffer.data[1]];
          CDC_Transmit_HS(user_cdc_buffer.data,2);
          break;
      /**
       * i2c read all
       *   ack => cdc 100(1 byte)
       *   I2C => ctrlBuffer => cdc(maximum 256 bytes)
       */
        case 100:
          // user_cdc_buffer.data[0]=ack=>100;
          CDC_Transmit_HS(user_cdc_buffer.data,1);
          for(commands[0]=0;commands[0]<sizeof(ctrlBuffer);commands[0]++){
            HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,1,10);
            HAL_I2C_Master_Receive(&hi2c3,0x40,&ctrlBuffer[commands[0]],1,10);
          }
          CDC_Transmit_HS(ctrlBuffer,sizeof(ctrlBuffer));
          break;
        default:
          break;
      }
      user_cdc_buffer.len = 0;
    }

//    if(imgRead==1 && ready==1){
//      if(frameReady==0){
//        CDC_Transmit_HS(&imgBuffer[2],2*frameSizeWidth*frameSizeHeight);
//      }
//      else if(frameReady==1){
//        imgBuffer[4*frameSizeWidth*frameSizeHeight]=imgBuffer[0];
//        imgBuffer[4*frameSizeWidth*frameSizeHeight+1]=imgBuffer[1];
//        CDC_Transmit_HS(&imgBuffer[2+2*frameSizeWidth*frameSizeHeight],2*frameSizeWidth*frameSizeHeight);
//      }
//      imgRead=0;
//    }

    if(imgRead==2){
      CDC_Transmit_HS((uint8_t*)img.calibration,36);
      imgRead=0;
    }

		ScreenDrawLoop(&img);


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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 50;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance==TIM4){
		HAL_SPI_Receive(&hspi3,(uint8_t*)img.image+img.pixel*2,1,1);
		if( img.pixel==  0 || img.pixel== 23 ||
				img.pixel== 46 || img.pixel== 69 ||
				img.pixel== 92 || img.pixel==115 ||
				img.pixel==138 || img.pixel==161 ||
				img.pixel==184 || img.pixel==207 ||
				img.pixel==230 || img.pixel==253 ||
				img.pixel==276 || img.pixel==299 ||
				img.pixel==322 || img.pixel==345 ||
				img.pixel==368 || img.pixel==391 ||
				img.pixel==414 || img.pixel==437 ||
				img.pixel==460 || img.pixel==483){
			imageValue[img.pixel/23]=img.image[0][img.pixel];
			img.image[0][img.pixel]=0;
		}
		img.pixel=(img.pixel<483)? img.pixel+1:0;
	}
}

/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

	// Timer 4 PWM Channel 2 generates ADC_CNV signal to Ext ADC
	if(htim->Instance==TIM3){
		HAL_TIM_PWM_Start_IT(&htim4,TIM_CHANNEL_2);
	}

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
