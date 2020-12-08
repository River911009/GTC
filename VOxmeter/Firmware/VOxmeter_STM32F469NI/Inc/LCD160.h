#ifndef __LCD160_H_
#define	__LCD160_H_

#include "stm32f4xx_hal.h"

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        160
#define	PANEL_SIZE					X_MAX_PIXEL*Y_MAX_PIXEL

#define	USB_PIXEL_X					160
#define	USB_PIXEL_Y					120

#define	SLEEP_IN						0x10
#define	SLEEP_OUT						0x11
#define	COLUMN_ADDRESS_SET	0x2A
#define	ROW_ADDRESS_SET			0x2B
#define	MEMORY_WRITE				0x2C
#define	DISPLAY_ON					0x29
#define	MEMORY_DATA_ACCESS	0x36
#define	PIXEL_FORMAT				0x3A

#define	COLOR_BLACK				0x0000
#define	COLOR_RED					0x00F8
#define	COLOR_GREEN				0xE007
#define	COLOR_BLUE				0x1F00
#define COLOR_WHITE				0xFFFF
#define	COLOR_FONT				COLOR_WHITE
#define	COLOR_FONT_BACK		COLOR_BLACK

extern const uint16_t THERMAL_COLOR[256];
extern uint16_t font_color;

void LCD_WriteIndex(uint8_t index);
void LCD_WriteData(uint8_t data);
void LCD_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end);
void LCD_Initial(void);

void All_Color(uint8_t color_red, uint8_t color_green, uint8_t color_blue);
void LCD_ColorBar(void);
void LCD_GrayBarArea(uint16_t add_sx, uint16_t add_ex, uint16_t add_sy, uint16_t add_ey);
void LCD_TempBarArea(uint16_t add_sx, uint16_t add_ex, uint16_t add_sy, uint16_t add_ey);
void LCD_PutChar_Vertical(uint16_t adx, uint16_t ady, const char *p);
void LCD_DisplayString_Vertical(uint16_t addx, uint16_t addy, const char *p );
void LCD_DisplayTemperature(uint16_t addx, uint16_t addy, int16_t temp);
void LCD_DisplayNumber(uint16_t addx, uint16_t addy, uint32_t num);
void LCD_DisplayFloat(uint16_t addx, uint16_t addy, float num);
void LCD_ShowLogo(uint16_t adx, uint16_t ady, const uint16_t *bitmap);
void LCD_DrawPoint(uint16_t ady, uint16_t adx,uint16_t colour);

void Buffer_PutChar_Vertical(uint16_t pos, const char *p, uint16_t *buffer, uint16_t color);
void Buffer_AddString_Vertical(uint16_t pos_s, const char *p, uint16_t *buffer, uint16_t color_s);
void Buffer_AddTemperature(uint16_t pos_t, int16_t temp, uint16_t *buffer);
void Buffer_AddCenterFrame(uint8_t length, uint16_t *buffer);

extern uint8_t display_buffer_usb[180][160];
void USB_AddString(uint16_t pos_x, uint16_t pos_y, const char *p);
void USB_PutChar(uint16_t pos_x, uint16_t pos_y, const char *p);

#endif
