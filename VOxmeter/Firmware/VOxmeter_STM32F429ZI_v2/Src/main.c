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
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdint.h"
#include "math.h"
#include "usbd_cdc_if.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// #define 

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t buffer[5000];
uint32_t i=300,j=1000,k=70,l=200,psc=999,arr=83,delay=60000;
int slope=0;
uint8_t str[13]="";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void getValue_ADC(uint16_t* dist){
  uint8_t adc_tmp[2];
  // start and wait about 1us for ADC converting
  HAL_GPIO_WritePin(ADC_CNV_GPIO_Port,ADC_CNV_Pin,GPIO_PIN_SET);
  for(uint8_t wait=0;wait<30;wait++);
  // stop ADC convert to receive data
  HAL_GPIO_WritePin(ADC_CNV_GPIO_Port,ADC_CNV_Pin,GPIO_PIN_RESET);
  HAL_SPI_Receive(&hspi3,adc_tmp,1,10);
  // reshape adc_tmp[2] to dist
  *dist=adc_tmp[1];
  *dist<<=8;
  *dist+=adc_tmp[0];
}

void delay_us(uint16_t us_cnt){
  htim2.Instance->CNT=0;
  while(htim2.Instance->CNT<us_cnt);
}

void shortToString(uint16_t num,uint16_t value){
  str[0]=((num/10000)%10)? ((num/10000)%10)+48:48;
  str[1]=((num/1000)%10)? ((num/1000)%10)+48:48;
  str[2]=((num/100)%10)? ((num/100)%10)+48:48;
  str[3]=((num/10)%10)? ((num/10)%10)+48:48;
  str[4]=(num%10)? (num%10)+48:48;
  str[5]=',';
  str[6]=((value/10000)%10)? ((value/10000)%10)+48:48;
  str[7]=((value/1000)%10)? ((value/1000)%10)+48:48;
  str[8]=((value/100)%10)? ((value/100)%10)+48:48;
  str[9]=((value/10)%10)? ((value/10)%10)+48:48;
  str[10]=(value%10)? (value%10)+48:48;
  str[11]='\n';
  str[12]='\r';
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
  MX_SPI3_Init();
  MX_TIM1_Init();
  MX_TIM7_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  // clear output GPIO 
  HAL_GPIO_WritePin(ADC_CNV_GPIO_Port,ADC_CNV_Pin,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Res_Cali_GPIO_Port,Res_Cali_Pin,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(Res_Test_GPIO_Port,Res_Test_Pin,GPIO_PIN_RESET);
  // start delay microsecond timer
  HAL_TIM_Base_Start(&htim2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // timer 2 delay us test
    HAL_GPIO_WritePin(LD4_GPIO_Port,LD4_Pin,GPIO_PIN_SET);
    delay_us(delay);
    HAL_GPIO_WritePin(LD4_GPIO_Port,LD4_Pin,GPIO_PIN_RESET);

    // preset timer 7 parameters
    TIM7->PSC=psc;
    TIM7->ARR=arr;

    // clear buffer
    for(uint16_t index=0;index<(sizeof(buffer)/2);index++){
      buffer[index]=0;
    }

    // turn on calibration resistor
    HAL_GPIO_WritePin(Res_Test_GPIO_Port,Res_Test_Pin,GPIO_PIN_SET);
    for(uint32_t delay_count=0;delay_count<delay;delay_count++);
    // HAL_Delay(delay);

    // continuously get ADC value until circuit stable maximum time (<1ms@200Ohm)
    for(uint16_t cnv_count=0;cnv_count<i;cnv_count++){
      // get new data
      getValue_ADC(&buffer[cnv_count]);
      // break if the value in buffer was increase
      if( buffer[0]>buffer[1] && (buffer[0]-buffer[1])>i &&\
          buffer[1]>buffer[2] && (buffer[1]-buffer[2])>i &&\
          buffer[2]>buffer[3] && (buffer[2]-buffer[3])>i &&\
          buffer[3]>buffer[4] && (buffer[3]-buffer[4])>i \
      ){
        HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_SET);
        HAL_TIM_Base_Start_IT(&htim7);
        if(cnv_count>=20){
          break;
        }
      }

      // // find maximum value
      // if(buffer[0]>=j){
      //   k=buffer[0];
      //   HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_SET);
      // }
      // else{
      //   k=0;
      //   HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_RESET);
      // }

      // moving window to next time
      for(uint16_t mov_count=(sizeof(buffer)/2)-1;mov_count>0;mov_count--){
        buffer[mov_count]=buffer[mov_count-1];
      }

      // wait a while
      for(int x=0;x<k;x++);
    }

    // turn off calibration resistor
    // for(int x=0;x<100;x++);
    HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_RESET);
    // HAL_Delay(5);
    HAL_GPIO_WritePin(Res_Test_GPIO_Port,Res_Test_Pin,GPIO_PIN_RESET);

    for(uint16_t x=0;x<i;x++){
      shortToString(buffer[x],buffer[x]);
      CDC_Transmit_HS(str,sizeof(str));
      HAL_Delay(1);
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_Delay(1000);
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
}

/* USER CODE BEGIN 4 */

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
