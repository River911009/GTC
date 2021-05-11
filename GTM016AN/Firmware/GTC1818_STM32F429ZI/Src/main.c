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
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_cdc_if.h"
#include "stdint.h"

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
I2C_HandleTypeDef hi2c3;
DMA_HandleTypeDef hdma_i2c3_rx;

LTDC_HandleTypeDef hltdc;

SPI_HandleTypeDef hspi4;
DMA_HandleTypeDef hdma_spi4_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C3_Init(void);
static void MX_LTDC_Init(void);
static void MX_SPI4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// cdc package buffer
USER_CDC_PACKAGE_TYPE_t user_cdc_buffer;

// current frame size
uint8_t frameSizeWidth=0;
uint8_t frameSizeHeight=0;

// 2 times image buffer from spi
uint8_t imgBuffer[2*22*22*2+2];
uint8_t calibrationPixel[2*18*2]={0x00,0x00,0x0F,0x0F,0x1E,0x1E,0x2D,0x2D,\
                                  0x3C,0x3C,0x4B,0x4B,0x5A,0x5A,0x69,0x69,\
                                  0x78,0x78,0x87,0x87,0x96,0x96,0xA5,0xA5,\
                                  0xB4,0xB4,0xC3,0xC3,0xD2,0xD2,0xE1,0xE1,\
                                  0xF0,0xF0,0xFF,0xFF,0x00,0x00,0x0F,0x0F,\
                                  0x1E,0x1E,0x2D,0x2D,0x3C,0x3C,0x4B,0x4B,\
                                  0x5A,0x5A,0x69,0x69,0x78,0x78,0x87,0x87,\
                                  0x96,0x96,0xA5,0xA5,0xB4,0xB4,0xC3,0xC3,\
                                  0xD2,0xD2,0xE1,0xE1,0xF0,0xF0,0xFF,0xFF};

// i2c control value buffer
uint8_t ctrlBuffer[100];

// image pointer
uint8_t imgRead=0;
// cdc command buffer
uint8_t commands[4];
// double frame buffer selector
uint8_t frameReady=1;
uint8_t ready=0;
// uint8_t sync=0;

// create testFrame
void createFrame(){
  for(int i=0;i<sizeof(imgBuffer);i++){
    imgBuffer[i]=i%256;
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

// preset spi and dma interface then wait the vsync fail
void resyncFrame(){
  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  HAL_GPIO_WritePin(SPI_SS_GPIO_Port,SPI_SS_Pin,GPIO_PIN_SET);
  HAL_SPI_DeInit(&hspi4);
  HAL_DMA_DeInit(&hdma_spi4_rx);
  HAL_DMA_Init(&hdma_spi4_rx);
  HAL_SPI_Init(&hspi4);
  HAL_SPI_Receive_DMA(&hspi4,(uint8_t*)imgBuffer,2*frameSizeWidth*frameSizeHeight);
  // frameReady=4;

  // clear imgBuffer to check if frame size changed
  // for(int i=0;i<sizeof(imgBuffer);i++) imgBuffer[i]=0;

  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin==SENSER_VSYNC_Pin){
    HAL_GPIO_TogglePin(LD3_GPIO_Port,LD3_Pin);
  }
  if(GPIO_Pin==B1_Pin){
    HAL_GPIO_TogglePin(LD4_GPIO_Port,LD4_Pin);
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
  }
  // getFrameSize();
  // resyncFrame();
}


// spi complete indicator
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
 HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_13);
 HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_13);
	// ;
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
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_USB_DEVICE_Init();
  MX_SPI4_Init();
  /* USER CODE BEGIN 2 */


  // read frame size for gui screen
  getFrameSize();
  // first frame sync on system startup
  // resyncFrame();
  createFrame();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){


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
            resyncFrame();
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

    if(imgRead==1 && ready==1){
      if(frameReady==0){
        CDC_Transmit_HS(&imgBuffer[2],2*frameSizeWidth*frameSizeHeight);
      }
      else if(frameReady==1){
        imgBuffer[4*frameSizeWidth*frameSizeHeight]=imgBuffer[0];
        imgBuffer[4*frameSizeWidth*frameSizeHeight+1]=imgBuffer[1];
        CDC_Transmit_HS(&imgBuffer[2+2*frameSizeWidth*frameSizeHeight],2*frameSizeWidth*frameSizeHeight);
      }
      imgRead=0;
    }

    if(imgRead==2){
      CDC_Transmit_HS(calibrationPixel,36);
      imgRead=0;
    }

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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
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
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 100;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 9;
  hltdc.Init.VerticalSync = 1;
  hltdc.Init.AccumulatedHBP = 29;
  hltdc.Init.AccumulatedVBP = 3;
  hltdc.Init.AccumulatedActiveW = 269;
  hltdc.Init.AccumulatedActiveH = 323;
  hltdc.Init.TotalWidth = 279;
  hltdc.Init.TotalHeigh = 327;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 240;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 320;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = 0xD0000000;
  pLayerCfg.ImageWidth = 240;
  pLayerCfg.ImageHeight = 320;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_SLAVE;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
  hspi4.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_SS_GPIO_Port, SPI_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ACP_RST_GPIO_Port, ACP_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RDX_Pin|WRX_DCX_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SPI_SS_Pin */
  GPIO_InitStruct.Pin = SPI_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : A0_Pin A1_Pin A2_Pin A3_Pin 
                           A4_Pin A5_Pin SDNRAS_Pin A6_Pin 
                           A7_Pin A8_Pin A9_Pin */
  GPIO_InitStruct.Pin = A0_Pin|A1_Pin|A2_Pin|A3_Pin 
                          |A4_Pin|A5_Pin|SDNRAS_Pin|A6_Pin 
                          |A7_Pin|A8_Pin|A9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI5_SCK_Pin SPI5_MISO_Pin SPI5_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI5_SCK_Pin|SPI5_MISO_Pin|SPI5_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : SDNWE_Pin */
  GPIO_InitStruct.Pin = SDNWE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(SDNWE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NCS_MEMS_SPI_Pin CSX_Pin OTG_FS_PSO_Pin */
  GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MEMS_INT1_Pin MEMS_INT2_Pin TP_INT1_Pin */
  GPIO_InitStruct.Pin = MEMS_INT1_Pin|MEMS_INT2_Pin|TP_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SENSER_VSYNC_Pin */
  GPIO_InitStruct.Pin = SENSER_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SENSER_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ACP_RST_Pin */
  GPIO_InitStruct.Pin = ACP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ACP_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : A10_Pin A11_Pin BA0_Pin BA1_Pin 
                           SDCLK_Pin SDNCAS_Pin */
  GPIO_InitStruct.Pin = A10_Pin|A11_Pin|BA0_Pin|BA1_Pin 
                          |SDCLK_Pin|SDNCAS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : D4_Pin D5_Pin D6_Pin D7_Pin 
                           D8_Pin D9_Pin D10_Pin D11_Pin 
                           D12_Pin NBL0_Pin NBL1_Pin */
  GPIO_InitStruct.Pin = D4_Pin|D5_Pin|D6_Pin|D7_Pin 
                          |D8_Pin|D9_Pin|D10_Pin|D11_Pin 
                          |D12_Pin|NBL0_Pin|NBL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : D13_Pin D14_Pin D15_Pin D0_Pin 
                           D1_Pin D2_Pin D3_Pin */
  GPIO_InitStruct.Pin = D13_Pin|D14_Pin|D15_Pin|D0_Pin 
                          |D1_Pin|D2_Pin|D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : TE_Pin */
  GPIO_InitStruct.Pin = TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RDX_Pin WRX_DCX_Pin */
  GPIO_InitStruct.Pin = RDX_Pin|WRX_DCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : STLINK_RX_Pin STLINK_TX_Pin */
  GPIO_InitStruct.Pin = STLINK_RX_Pin|STLINK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD4_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : SDCKE1_Pin SDNE1_Pin */
  GPIO_InitStruct.Pin = SDCKE1_Pin|SDNE1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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
