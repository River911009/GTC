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
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stdint.h"
#include "usbd_cdc_if.h"
#include "LCD160.h"
#include "stdlib.h"

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

uint16_t del=150;
uint8_t i=2;
volatile uint8_t dma_cplt=0;
uint16_t tmp[1320];
uint8_t str[1320*7];

// offset using (unit:vref is V, iref is uA, rref is ohm)
double vref=3.2994,iref=20.0,rref=0.0;
uint32_t calibration_R=146420;

// for calculate move average using
float average1=0.0,average2=0.0,resistor=0.0,slope=0.0;
float tmpRes=0.0,tmpSlope=0.0,avgRes=0.0,avgSlope=0.0;
uint16_t start=0,end=1320;

// for average using
uint8_t resetValue=0;
uint16_t maxValue=0,minValue=65535,avgValue=0,countValue=0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void switchResistor(uint8_t target,uint8_t status){
  if(target==1){  // calibrate resistor
    if(status==0){  // off
      HAL_GPIO_WritePin(P1_GPIO_Port,P1_Pin,GPIO_PIN_SET);
      HAL_GPIO_WritePin(N1_GPIO_Port,N1_Pin,GPIO_PIN_SET);
    }
    else{ // on
      HAL_GPIO_WritePin(N1_GPIO_Port,N1_Pin,GPIO_PIN_RESET);
      HAL_GPIO_WritePin(P1_GPIO_Port,P1_Pin,GPIO_PIN_RESET);
    }
  }
  else if(target==2){  // VOx resistor
    if(status==0){  // off
      HAL_GPIO_WritePin(P2_GPIO_Port,P2_Pin,GPIO_PIN_SET);
      HAL_GPIO_WritePin(N2_GPIO_Port,N2_Pin,GPIO_PIN_SET);
    }
    else{ // on
      HAL_GPIO_WritePin(N2_GPIO_Port,N2_Pin,GPIO_PIN_RESET);
      HAL_GPIO_WritePin(P2_GPIO_Port,P2_Pin,GPIO_PIN_RESET);
    }
  }
  else{ // all discharge and off
    HAL_GPIO_WritePin(P1_GPIO_Port,P1_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(P2_GPIO_Port,P2_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(N1_GPIO_Port,N1_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(N2_GPIO_Port,N2_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(N1_GPIO_Port,N1_Pin,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(N2_GPIO_Port,N2_Pin,GPIO_PIN_RESET);
  }
}

void getADCArray(uint16_t length,uint8_t avg_range){
  uint16_t timeout=64300;
  HAL_SPI_Receive_DMA(&hspi2,(uint8_t*)tmp,length);
  HAL_TIM_Base_Start(&htim4);
  while(dma_cplt==0 && timeout--);
  HAL_TIM_Base_Stop(&htim4);
  switchResistor(0,0);
  dma_cplt=0;

  // calculate average1 from 660 to 710
  average1=tmp[660];
  for(uint8_t k=1;k<avg_range;k++){
    average1=(average1+tmp[660+k])/2;
  }

  // calculate average2 from 1250 to 1300
  average2=tmp[1200];
  for(uint8_t k=1;k<avg_range;k++){
    average2=(average2+tmp[1200+k])/2;
  }
}

void calibration(uint32_t R){
  switchResistor(1,1);
  getADCArray(1320,10);
  // iref=(((double)average1+(double)average2)/2.0)*(vref/65536.0)*1000*1000/(double)R;
  iref=((double)(average1+average2)/2.0)*(vref*15.2587890625)/(double)R;
  // for procedure accelerating
  // vref is fixed to about 3.3v
  // iref is Vadc_read/calibration_R about 20uA
  // rref is vref/iref which is variable
  rref=vref/iref;
}

void systemInitial(){
  // switch off all resistors
  switchResistor(0,0);

  // AD7980 reading sequence generator timers
  // timer 1 pwm for SCK_B high speed
  HAL_TIMEx_PWMN_Start(&htim1,TIM_CHANNEL_1);
  // timer 3 pwm for ADC_CNV pin
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  // timer 12 pwm with low polarity for SCK_A and gate input
  HAL_TIM_PWM_Start(&htim12,TIM_CHANNEL_1);
  // timer 4 is a master node to trigger timer 1, 3 and 12. (start)
  // HAL_TIM_Base_Start(&htim4);

  // calibrate current source iref
  calibration(calibration_R);

  // ST7735 initialise
  LCD_Initial();    // ST7735_Init();
  All_Color(0,0,0); // ST7735_FillScreen(0x0000);
  LCD_DisplayString_Vertical(120,48,"Ohmmeter");
  for(uint8_t x=16;x<151;x++){
    LCD_DrawPoint(x,118,0xFFFF);
    LCD_DrawPoint(x,18,0xFFFF);
  }
  for(uint8_t y=18;y<119;y++){
    LCD_DrawPoint(151,y,0xFFFF);
    LCD_DrawPoint(16,y,0xFFFF);
  }
  LCD_DisplayString_Vertical(8,0,"  R  =           Ohm");
  LCD_DisplayString_Vertical(0,0,"Slope=          V/mS");
  LCD_DisplayString_Vertical(44,0,"1V");
  LCD_DisplayString_Vertical(74,0,"2V");
  LCD_DisplayString_Vertical(104,0,"3V");
  LCD_DisplayNumber(8,64,0);
  LCD_DisplayFloat(0,48,0.000);
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port,LCD_BL_Pin,GPIO_PIN_SET);
}

void reDraw(uint32_t resistor,float slope){
  uint32_t average=0;
  // redraw curve
  for(uint8_t x=0;x<132;x++){
    for(uint8_t y=1;y<=99;y++){
      if((y%30==0||x==66)&&(x%2==0)&&(y%2==0)){
        LCD_DrawPoint(x+18,y+18,0x4208);
      }
      else
      {
        LCD_DrawPoint(x+18,y+18,0x0000);
      }
    }
    for(uint8_t a=0;a<5;a++){
      average+=tmp[660+x*5+a];
    }
    average=(average*99)/327675;
    if(average>0){
      LCD_DrawPoint(x+18,average+18,0xF800);
    }
  }

  // check overload or short then display value
  if(average1>=65535){
    LCD_DisplayString_Vertical(8,56,"OverLoad");
  }
  else if(resistor<=425){
    LCD_DisplayString_Vertical(8,56,"  <  500 ");
  }
  else{
    LCD_DisplayString_Vertical(8,56,"       ");
    LCD_DisplayNumber(8,64,resistor);
  }
  LCD_DisplayFloat(0,48,slope);
}

void shortToString(void){
  for(uint16_t x=start;x<end;x++){
    str[x*7+0]=((tmp[x]/10000)%10)? ((tmp[x]/10000)%10)+48:48;
    str[x*7+1]=((tmp[x]/1000)%10)? ((tmp[x]/1000)%10)+48:48;
    str[x*7+2]=((tmp[x]/100)%10)? ((tmp[x]/100)%10)+48:48;
    str[x*7+3]=((tmp[x]/10)%10)? ((tmp[x]/10)%10)+48:48;
    str[x*7+4]=(tmp[x]%10)? (tmp[x]%10)+48:48;
    str[x*7+5]='\n';
    str[x*7+6]='\r';
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
  MX_USB_DEVICE_Init();
  MX_SPI4_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM12_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  systemInitial();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(i){ // debug mode
      switchResistor(i,1);
      getADCArray(1320,10);

      if(resetValue==1 || countValue>100){
        avgValue=(maxValue+minValue)/2;
        // avgValue=average1;
        maxValue=0;
        minValue=65535;
        countValue=0;
        calibration(calibration_R);
        if(resetValue==0) continue;
      }
      if(maxValue<average1){
        maxValue=average1;
      }
      if(minValue>average1){
        minValue=average1;
      }
      countValue++;

      // resistor=average1*(vref/65536.0)*((3465.0*1000000.0)/(300.0*227.0));
      // resistor=(double)(average1)*2.562065;
      resistor=(double)(average1)*15.2587890625*rref;
      // calibrated function
      // resistor=(double)average1*(double)average1*0.0000006+(double)average1*2.3839+4.7023;
      // slope=(average2-average1)*(vref/65536.0)*1000000.0/((1200-660)*1.5);
      slope=(double)(((average2-average1)/(average1*average1))*1000000);//*0.061728; // unit: (V/S)
//      slope/=1000;  // V/S -> V/mS

      // shortToString();
      // CDC_Transmit_FS((uint8_t*)str,sizeof(str));
			
			tmpRes=(tmpRes+resistor)/2.0;
			tmpSlope=(tmpSlope+slope)/2.0;
			
			if(countValue%10==0){
				avgRes=tmpRes;
				avgSlope=tmpSlope;
				tmpRes=0.0;
				tmpSlope=0.0;
				reDraw((uint32_t)avgRes,avgSlope);
			}
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_Delay(del);
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
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 144;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV6;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enables the Clock Security System 
  */
  HAL_RCC_EnableCSS();
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
