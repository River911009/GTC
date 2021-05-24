#include "UI.h"


extern DMA_HandleTypeDef hdma_spi4_rx;

ui_TypeDef ui;// __attribute__((section("0x20002950")));
frame_BufferTypeDef img;
control_BufferTypeDef i2c_buffer;


/**
 * GoalTop logo
 */
const uint8_t gImage_goaltop_64x48[384]={
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X07,0XFF,0XFF,0XF0,0X00,0X00,0X00,0X00,
0X1F,0XFF,0XFF,0XFE,0X80,0X00,0X00,0X00,0X1F,0XFF,0XFF,0XFF,0XC0,0X00,0X00,0X00,
0X1F,0XFF,0XFF,0XFF,0XF8,0X00,0X00,0X00,0X1F,0XFF,0XFF,0XFF,0XFE,0X00,0X00,0X00,
0X0F,0XFC,0X03,0XFF,0XFF,0X00,0X00,0X00,0X0F,0X00,0X00,0X7F,0XFF,0XC0,0X00,0X00,
0X00,0X00,0X00,0X0F,0XFF,0XE0,0X00,0X00,0X00,0X00,0X00,0X03,0XFF,0XF0,0X00,0X00,
0X00,0X00,0X00,0X00,0XFF,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,0X7F,0XFC,0X00,0X00,
0X00,0X00,0X00,0X00,0X3F,0XFE,0X00,0X00,0X00,0X00,0X00,0X00,0X1F,0XFF,0X00,0X00,
0X00,0X00,0X00,0X00,0X1F,0XFF,0X80,0X00,0X00,0X00,0X00,0X00,0X0F,0XFF,0X80,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X20,0X20,0X1F,0X01,0XF0,0X00,0X00,0X00,0X60,0X70,0X31,0X83,0X18,
0X00,0X00,0X00,0X60,0XD8,0X20,0XC0,0X0C,0X00,0X00,0X00,0X20,0XD8,0X20,0X43,0X0C,
0X00,0X00,0X00,0X21,0XFC,0X20,0XC7,0X0C,0X00,0X00,0X00,0X61,0X0C,0X31,0X86,0X18,
0X00,0X00,0X03,0XE1,0X04,0X07,0X02,0XF0,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X7F,0XF8,0X00,0X00,0X7F,0XF8,0X00,0X00,0X3F,0XF8,0X00,0X00,0X7F,0XFC,0X00,
0X00,0X3F,0XF8,0X00,0X00,0X7F,0XFC,0X00,0X00,0X3F,0XF8,0X00,0X00,0X7F,0XFC,0X00,
0X00,0X3F,0XFC,0X00,0X00,0X7F,0XFC,0X00,0X00,0X1F,0XFC,0X00,0X00,0X7F,0XFC,0X00,
0X00,0X1F,0XFC,0X00,0X00,0X7F,0XFC,0X00,0X00,0X1F,0XFC,0X00,0X00,0XFF,0XFC,0X00,
0X00,0X1F,0XFE,0X00,0X00,0XFF,0XF8,0X00,0X00,0X0F,0XFE,0X00,0X01,0XFF,0XF8,0X00,
0X00,0X0F,0XFE,0X00,0X01,0XFF,0XF8,0X00,0X00,0X0F,0XFF,0X00,0X03,0XFF,0XF8,0X00,
0X00,0X07,0XFF,0X00,0X0F,0XFF,0XF0,0X00,0X00,0X07,0XFF,0X00,0X1F,0XFF,0XE0,0X00,
0X00,0X07,0XFF,0X01,0XFF,0XFF,0XE0,0X00,0X00,0X07,0XFF,0XFF,0XFF,0XFF,0XC0,0X00,
0X00,0X03,0XFF,0XFF,0XFF,0XFF,0X80,0X00,0X00,0X03,0XFF,0XFF,0XFF,0XFE,0X00,0X00,
0X00,0X03,0XFF,0XFF,0XFF,0XFC,0X00,0X00,0X00,0X01,0XFF,0XFF,0XFF,0XF0,0X00,0X00,
0X00,0X00,0X0F,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};



/**
 * function
 */
// 90 degree rotate display string
void BSP_LCD_DisplayStringAtLine90Degree(
	uint16_t X,
	uint16_t Y,
	uint8_t *pText,
	uint8_t len,
	uint32_t colour,
	uint32_t background
){
	// "character" loop for characters
	for(uint8_t character=0;character<len;character++){
		// "scan_y" loop for vertical scan
		for(uint8_t scan_y=0;scan_y<BSP_LCD_GetFont()->Height;scan_y++){
			// get font table
			uint8_t tmp=BSP_LCD_GetFont()->table[(pText[character]-' ')*(BSP_LCD_GetFont()->Height)+scan_y];
			// "scan_x" loop for horizontal scan
			for(uint8_t scan_x=0;scan_x<BSP_LCD_GetFont()->Width;scan_x++){
				// avoid if draw at negative position
				if(scan_x+X>character*7){
					BSP_LCD_DrawPixel(scan_y+Y,scan_x+X-character*7,(tmp&0x01)? colour:background);
				}
				// next pixel
				tmp>>=1;
			}
		}
	}
}


// 90 degree rotate display image
void BSP_LCD_DrawBitmap90Degree(
	uint16_t X,
	uint16_t Y,
	uint16_t WIDTH,
  uint16_t HEIGHT,
	uint8_t *pBmp,
	uint32_t colour
){
	for(uint8_t scan_y=0;scan_y<HEIGHT;scan_y++){
		for(uint8_t scan_x=0;scan_x<(WIDTH/8);scan_x++){
			uint8_t tmp=pBmp[scan_y*8+scan_x];
			for(uint8_t b=0;b<8;b++){
				if(tmp&0x80){
					BSP_LCD_DrawPixel(scan_y+Y,scan_x*8+X+b,colour);
				}
				tmp<<=1;
			}
		}
	}
}
// UI start up drawing
void UI_Initial(
	uint32_t logoColour,
	uint32_t fontColour,
	uint32_t backColour
){
	// initial ui colour
	ui.logoColour=logoColour;
	ui.fontColour=fontColour;
	ui.backColour=backColour;

	BSP_LCD_Init();
//  BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER,LCD_FRAME_BUFFER);
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER,LCD_FRAME_BUFFER);
	
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
	BSP_LCD_DisplayOn();
	BSP_LCD_Clear(ui.backColour);
	BSP_LCD_SetBackColor(ui.backColour);

	// draw logo image
	BSP_LCD_DrawBitmap90Degree(3,10,64,48,(uint8_t*)gImage_goaltop_64x48,ui.logoColour);
	// draw text
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAtLine90Degree(50,60,(uint8_t*)"GoalTop",7,ui.logoColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(53,72,(uint8_t*)"Tech.Co.",8,ui.logoColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(48,90,(uint8_t*)"Series",6,ui.fontColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(55,102,(uint8_t*)"GTM016AN",8,ui.fontColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(55,LABEL_SHIFT,(uint8_t*)"=setADC=",8,ui.fontColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT,(uint8_t*)"   Test   ",10,ui.fontColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+20,(uint8_t*)" Internal ",10,ui.fontColour,ui.backColour);
	BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+40,(uint8_t*)" External ",10,ui.fontColour,ui.backColour);
}


// create test frame
void createFrame(frame_BufferTypeDef* src){
	for(uint8_t i=0;i<sizeof(src->calibration)/2;i++){
		src->calibration[i]=(i*1799);// uint16 (i*7)*0x0101
	}
	for(uint8_t i=0;i<11;i++){
		for(uint8_t j=i;j<(22-i);j++){
			for(uint8_t k=i;k<(22-i);k++){
				src->image[0][k+j*22]=(i*5911);// uint16 (i*23)*0x0101;
				src->image[1][k+j*22]=src->image[0][k+j*22];
			}
		}
	}
}



// select Int/Ext ADC source
void setADC(ADC_SRC src){
	// stop external adc cnv clock and pclk
	HAL_NVIC_DisableIRQ(GTM016AN_VSYNC_EXTI_IRQn);
	HAL_NVIC_DisableIRQ(GTM016AN_PCLK_EXTI_IRQn);
	__HAL_GPIO_EXTI_CLEAR_IT(GTM016AN_PCLK_Pin);
	__HAL_GPIO_EXTI_CLEAR_IT(GTM016AN_VSYNC_Pin);
//	HAL_SPI_DMAStop(&hspi4);
//	HAL_DMA_Abort(&hdma_spi4_rx);
	HAL_SPI_DeInit(&hspi3);
	HAL_SPI_DeInit(&hspi4);
	HAL_DMA_DeInit(&hdma_spi4_rx);
	// start correspond function
	switch(src){
		case ADC_Test:
			// update radio box on screen
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT,(uint8_t*)">  Test  <",10,ui.backColour,ui.fontColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+20,(uint8_t*)" Internal ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+40,(uint8_t*)" External ",10,ui.fontColour,ui.backColour);
			// create test frame
			createFrame(&img);
			break;
		case ADC_Internal:
			// update radio box on screen
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT,(uint8_t*)"   Test   ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+20,(uint8_t*)">Internal<",10,ui.backColour,ui.fontColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+40,(uint8_t*)" External ",10,ui.fontColour,ui.backColour);
			// stop and reload spi4 and dma
//			HAL_SPI_DeInit(&hspi4);
//			HAL_DMA_DeInit(&hdma_spi4_rx);
			img.frame=0;
		
			HAL_DMA_Init(&hdma_spi4_rx);
			HAL_SPI_Init(&hspi4);
			// sync with vsync input
    HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);
    HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
			HAL_NVIC_SetPriority(EXTI3_IRQn,1,0);
		
//			HAL_NVIC_EnableIRQ(GTM016AN_PCLK_EXTI_IRQn);
			HAL_NVIC_EnableIRQ(GTM016AN_VSYNC_EXTI_IRQn);
			break;
		case ADC_External:
			// update radio box on screen
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT,(uint8_t*)"   Test   ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+20,(uint8_t*)" Internal ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+40,(uint8_t*)">External<",10,ui.backColour,ui.fontColour);
			// reload spi3
			HAL_SPI_Init(&hspi3);
			img.frame=0;
			// sync with vsync input
			
			HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
			HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
			HAL_NVIC_SetPriority(EXTI3_IRQn,0,0);
		
			HAL_NVIC_EnableIRQ(GTM016AN_PCLK_EXTI_IRQn);
			HAL_NVIC_EnableIRQ(GTM016AN_VSYNC_EXTI_IRQn);
			break;
		default:
			// update radio box on screen all off
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT,(uint8_t*)"   Test   ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+20,(uint8_t*)" Internal ",10,ui.fontColour,ui.backColour);
			BSP_LCD_DisplayStringAtLine90Degree(62,RADIO_SHIFT+40,(uint8_t*)" External ",10,ui.fontColour,ui.backColour);
			break;
	}
}


// Draw loop on screen
void ScreenDrawLoop(frame_BufferTypeDef* src){
	for(uint8_t y=0;y<22;y++){
		for(uint8_t x=0;x<22;x++){
			BSP_LCD_SetTextColor(((src->image[src->frame][(y*22+x)]/1)%256)*BASE_COLOUR|0xFF000000);
			BSP_LCD_FillRect(x*10+10,y*10+FRAME_SHIFT,10,10);
		}
	}
	for(uint8_t y=0;y<2;y++){
		for(uint8_t x=0;x<18;x++){
			BSP_LCD_SetTextColor(src->calibration[y*18+x]/256*BASE_COLOUR|0xFF000000);
			BSP_LCD_FillRect(x*10+30,y*10+FRAME_SHIFT+220,10,10);
		}
	}
}

void GTM016AN_setup(int fps){
	uint8_t commands[2]={0};
	switch(fps){
		case 100:
			commands[0]=0x9;commands[1]=0x5a;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0xe;commands[1]=0x3e;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x12;commands[1]=0xea;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x13;commands[1]=0x0;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x14;commands[1]=0x2;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x15;commands[1]=0xd;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x19;commands[1]=0x4;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x26;commands[1]=0x8;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x27;commands[1]=0x1f;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x3c;commands[1]=0x8f;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x3d;commands[1]=0x92;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x3e;commands[1]=0xf;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x3f;commands[1]=0xf;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x40;commands[1]=0x5;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x42;commands[1]=0xc;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x44;commands[1]=0x1;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x47;commands[1]=0x7c;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x48;commands[1]=0x6;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x4a;commands[1]=0xd;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x4b;commands[1]=0xd;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x4c;commands[1]=0xe;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x4d;commands[1]=0xe;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x50;commands[1]=0x2;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x52;commands[1]=0x80;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x53;commands[1]=0x7c;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x54;commands[1]=0xd;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x55;commands[1]=0xd;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x5a;commands[1]=0x44;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x5d;commands[1]=0xa7;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x60;commands[1]=0xb7;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			commands[0]=0x9;commands[1]=0x0;
			HAL_I2C_Master_Transmit(&hi2c3,0x40,commands,2,10);
			break;
		default:
			break;
	}
}


//// uint8Array to uint16Array
//uint16_t* uint16Array(uint8_t* Buff,uint16_t u16Length){
//	uint16_t data[u16Length];
//	for(uint16_t i=0;i<u16Length;i++){
//		data[i]=(Buff[i*2]<<8)+(Buff[i*2+1]);
//	}
//	return(data);
//}
//// uint16Array to uint8Array
//uint8_t*  uint8Array(uint16_t* Buff,uint16_t u8Length){
//	uint8_t data[u8Length];
//	for(uint16_t i=0;i<(u8Length/2);i++){
//		data[i*2]  =Buff[i];
//		data[i*2+1]=Buff[i];
//	}
//	return(data);
//}
