#ifndef __UI_H__
#define __UI_H__


/**
 * import
 */
#include "stm32f429i_discovery_lcd.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usbd_cdc_if.h"


/**
 * static number
 */
#define LOGO_COLOUR LCD_COLOR_BLUE
#define FONT_COLOUR LCD_COLOR_BLACK
#define BACK_COLOUR LCD_COLOR_LIGHTCYAN
#define BASE_COLOUR 0x00010101
#define LABEL_SHIFT 158
#define RADIO_SHIFT 178
#define FRAME_SHIFT 70


/**
 * type define
 */
// I2C address type define
typedef struct{
	uint8_t PID;							// [7:0] Product ID (19)
	uint8_t VID;							// [7:0] Version ID (0)
	uint8_t SlaveID;					// [7:0] I2C slave ID (32)
	uint8_t Reset;						// [0]
	uint8_t WriteProtect;			// [7:0] register write protection (90:write enable)
	uint8_t DigNp_LB;					// [7:0] PCLK=Sys_ck/DigNp=17.5MHz/1170=14.95kHz
	uint8_t DigNp_HB;					// [3:0]
	uint8_t SysNp;						// [7:0] Sys_ck=70MHz/SysNp=70MHz/4=17.5MHz
	uint8_t SPI_MsNp;					// [7:0] SPI_SCK=Sys_ck/SPI_MsNp=17.5MHz/(DigNp*18)=17.5MHz/65=269KHz
	uint8_t PCLKMask_enh;			// [2]   Mask PCLK clock in Vsync or Hsync period
	uint8_t SPICSMode;				// [3]   (0:Toggle CS pin on each pixal,1:Toggle CS pin when changing frame)
	uint8_t Hstart_PreSync;		// [4:0]
	uint8_t Vstart_PreSync;		// [4:0]
	uint8_t Hend_PreSync;			// [4:0]
	uint8_t Vend_PreSync;			// [4:0]
	uint8_t CRstart_PreSync;	// [4:0] start position of calibration row
	uint8_t Cend_PreSync;			// [4:0] end position of calibration row
	uint8_t SYNC_length;			// [4:0]
	uint8_t PowerDown;				// [6]   software power down control by register, (0:active,1:power down)
	uint8_t Analog_Output;		// [6]   Analog output enable , (0:disable,1:enable)
	uint8_t Ext_Vref_ratio;		// [1:0] External ADC Vref ratio selection, (0:1X,1:0.75X,2:0.5X,3:0.25X)
	uint8_t Ipix_current;			// [5:0] pixel current selection, (0:1uA,1:2uA,...63:64uA)
	uint8_t ADC_Vref_select;	// [4]   ADC reference selection, (0:internal,1:external)
	uint8_t PGA_gain;					// [2:0] CADC PGA gain, (0:1X,1:2X,...7:8X)
}control_BufferTypeDef;

// frame type define
typedef struct{
	uint8_t startX;
	uint8_t startY;
	uint8_t endX;
	uint8_t endY;
	uint16_t pixel;
	uint8_t frame;
	uint16_t image[2][22*22];	  // double buffer * 22x22 pixels with 2bytes/pxl
	uint16_t calibration[2*18];	// single buffer *  2x18 pixels with 2bytes/pxl
}frame_BufferTypeDef;


typedef struct{
	uint32_t logoColour;
	uint32_t fontColour;
	uint32_t backColour;
}ui_TypeDef;

// ADC_SRC type define
typedef enum{
	ADC_Test,
	ADC_Internal,
	ADC_External
}ADC_SRC;


/**
 * public function prototype
 */
void UI_Initial(uint32_t logoColour,uint32_t fontColour,uint32_t backColour);
void setADC(ADC_SRC adc);
void ScreenDrawLoop(frame_BufferTypeDef* src);

//uint16_t* uint16Array(uint8_t* Buff,uint16_t u16Length);
//uint8_t*  uint8Array(uint16_t* Buff,uint16_t u16Length);


#endif
