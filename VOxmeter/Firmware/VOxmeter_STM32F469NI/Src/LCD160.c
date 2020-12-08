#include "main.h"
#include "LCD160.h"
#include "fonts.h"
#include "spi.h"
//#include "Sensor.h"

uint16_t font_color=COLOR_FONT;

/* RGB565 thermal color table, 512 bytes */
/*const uint16_t THERMAL_COLOR[256] __attribute__((at(FLASH_ADDRESS_THERMAL)))={
	0x0000, 0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 0x0700, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00,
	0x0E00, 0x0E00, 0x0E08, 0x0F08, 0x0F08, 0x0F10, 0x0F10, 0x1018, 0x1018, 0x1018, 0x1020, 0x1120, 0x1120, 0x1128, 0x1128, 0x1230,
	0x1230, 0x1230, 0x1238, 0x1238, 0x1238, 0x1240, 0x1240, 0x1248, 0x1248, 0x1248, 0x1250, 0x1250, 0x1250, 0x1258, 0x1258, 0x1260,
	0x1260, 0x1260, 0x1268, 0x1268, 0x1268, 0x1270, 0x1270, 0x1278, 0x1278, 0x1278, 0x1280, 0x1280, 0x1280, 0x1288, 0x1288, 0x1290,
	0x1290, 0x1290, 0x1290, 0x1298, 0x1298, 0x1298, 0x1298, 0x12A0, 0x12A0, 0x12A0, 0x12A0, 0x12A8, 0x12A8, 0x12A8, 0x12A8, 0x12B0,
	0x12B0, 0x12B0, 0x12B0, 0x12B8, 0x12B8, 0x12B8, 0x12B8, 0x12B8, 0x12C0, 0x12C0, 0x12C0, 0x12C0, 0x12C8, 0x12C8, 0x12C8, 0x12D0,
	0x11D0, 0x31D0, 0x51D0, 0x71D0, 0x70D0, 0x90D0, 0xB0D0, 0xD0D8, 0xCFD8, 0xEFD8, 0x0FD9, 0x2FD9, 0x2ED9, 0x4ED9, 0x6ED9, 0x8EE1,
	0x8DE1, 0x8CE1, 0x8CE1, 0xABE1, 0xAAE1, 0xAAE1, 0xA9E1, 0xC9E9, 0xC8E9, 0xC7E9, 0xC9E9, 0xE6E9, 0xE5E9, 0xE5E9, 0xE4E9, 0x04F2,
	0x03F2, 0x23F2, 0x23F2, 0x43F2, 0x42F2, 0x62F2, 0x62F2, 0x82F2, 0x81F2, 0xA1F2, 0xA1F2, 0xC1F2, 0xC0F2, 0xE0F2, 0xE0F2, 0x00FB,
	0x00FB, 0x20FB, 0x20FB, 0x40FB, 0x40FB, 0x60FB, 0x60FB, 0x80FB, 0x80FB, 0xA0FB, 0xA0FB, 0xC0FB, 0xC0FB, 0xE0FB, 0xE0FB, 0x00FC,
	0x00FC, 0x20FC, 0x20FC, 0x40FC, 0x40FC, 0x60FC, 0x60FC, 0x80FC, 0x80FC, 0xA0FC, 0xA0FC, 0xC0FC, 0xC0FC, 0xE0FC, 0xE0FC, 0x00FD,
	0x00FD, 0x20FD, 0x20FD, 0x40FD, 0x40FD, 0x60FD, 0x60FD, 0x80FD, 0x80FD, 0xA0FD, 0xA0FD, 0xC0FD, 0xC0FD, 0xE0FD, 0xE0FD, 0x00FE,
	0x00FE, 0x00FE, 0x00FE, 0x20FE, 0x20FE, 0x20FE, 0x20FE, 0x40FE, 0x40FE, 0x40FE, 0x40FE, 0x60FE, 0x60FE, 0x60FE, 0x60FE, 0x80FE,
	0x80FE, 0x81FE, 0xA1FE, 0xA2FE, 0xC2FE, 0xC3FE, 0xE3FE, 0xE4FE, 0x04FF, 0x05FF, 0x25FF, 0x26FF, 0x46FF, 0x47FF, 0x67FF, 0x68FF,
	0x88FF, 0x89FF, 0x8AFF, 0x8BFF, 0x8CFF, 0x8DFF, 0x8EFF, 0xAEFF, 0xAFFF, 0xB0FF, 0xB1FF, 0xB2FF, 0xB3FF, 0xB4FF, 0xB5FF, 0xD5FF,
	0xD6FF, 0xD7FF, 0xD7FF, 0xD8FF, 0xD9FF, 0xDAFF, 0xFAFF, 0xFBFF, 0xFCFF, 0xFCFF, 0xFDFF, 0xFEFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x1FF8
};*/

void LCD_WriteIndex(uint8_t index)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi4, &index, 1, 100);
}

void LCD_WriteData(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi4, &data, 1, 100);
}
void LCD_SetRegion(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
	x_start +=2;
	x_end +=2;
	y_start +=1;
	y_end +=1;
	
	LCD_WriteIndex(COLUMN_ADDRESS_SET);
	LCD_WriteData(x_start>>8);
	LCD_WriteData(x_start);
	LCD_WriteData(x_end>>8);
	LCD_WriteData(x_end);

	LCD_WriteIndex(ROW_ADDRESS_SET);
	LCD_WriteData(y_start>>8);
	LCD_WriteData(y_start);
	LCD_WriteData(y_end>>8);
	LCD_WriteData(y_end);
	
	LCD_WriteIndex(MEMORY_WRITE);
}

void LCD_Initial(void)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	
	HAL_Delay(50);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(20);

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	LCD_WriteIndex(0x11);//Sleep exit 
	HAL_Delay(100);
		
	//ST7735R Frame Rate
	LCD_WriteIndex(0xB1); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x2C); 
	LCD_WriteData(0x2D); 

	LCD_WriteIndex(0xB2); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x2C); 
	LCD_WriteData(0x2D); 

	LCD_WriteIndex(0xB3); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x2C); 
	LCD_WriteData(0x2D); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x2C); 
	LCD_WriteData(0x2D); 
	
	LCD_WriteIndex(0xB4); //Column inversion 
	LCD_WriteData(0x07); //07
	
	//ST7735R Power Sequence
	LCD_WriteIndex(0xC0); 
	LCD_WriteData(0xA2); 
	LCD_WriteData(0x02); 
	LCD_WriteData(0x84); 
	
	LCD_WriteIndex(0xC1); 
	LCD_WriteData(0xC5); 

	LCD_WriteIndex(0xC2); 
	LCD_WriteData(0x0A); 
	LCD_WriteData(0x00); 

	LCD_WriteIndex(0xC3); 
	LCD_WriteData(0x8A); 
	LCD_WriteData(0x2A); 
	LCD_WriteIndex(0xC4); 
	LCD_WriteData(0x8A); 
	LCD_WriteData(0xEE); 
	
	LCD_WriteIndex(0xC5); //VCOM 
	LCD_WriteData(0x0E); 
	
	LCD_WriteIndex(0x36); //MX, MY, RGB mode 
	LCD_WriteData(0x00); //40, C0
	
	//ST7735R Gamma Sequence
	LCD_WriteIndex(0xe0); 
	LCD_WriteData(0x0f); 
	LCD_WriteData(0x1a); 
	LCD_WriteData(0x0f); 
	LCD_WriteData(0x18); 
	LCD_WriteData(0x2f); 
	LCD_WriteData(0x28); 
	LCD_WriteData(0x20); 
	LCD_WriteData(0x22); 
	LCD_WriteData(0x1f); 
	LCD_WriteData(0x1b); 
	LCD_WriteData(0x23); 
	LCD_WriteData(0x37); 
	LCD_WriteData(0x00); 	
	LCD_WriteData(0x07); 
	LCD_WriteData(0x02); 
	LCD_WriteData(0x10); 

	LCD_WriteIndex(0xe1); 
	LCD_WriteData(0x0f); 
	LCD_WriteData(0x1b); 
	LCD_WriteData(0x0f); 
	LCD_WriteData(0x17); 
	LCD_WriteData(0x33); 
	LCD_WriteData(0x2c); 
	LCD_WriteData(0x29); 
	LCD_WriteData(0x2e); 
	LCD_WriteData(0x30); 
	LCD_WriteData(0x30); 
	LCD_WriteData(0x39); 
	LCD_WriteData(0x3f); 
	LCD_WriteData(0x00); 
	LCD_WriteData(0x07); 
	LCD_WriteData(0x03); 
	LCD_WriteData(0x10);  
	
	LCD_WriteIndex(0x2a);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x7f);

	LCD_WriteIndex(0x2b);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x9f);
	
	LCD_WriteIndex(0xF0); //Enable test command  
	LCD_WriteData(0x01); 
	
	LCD_WriteIndex(0xF6); //Disable ram power save mode 
	LCD_WriteData(0x00); 
	
	LCD_WriteIndex(0x3A); //65k mode 
	LCD_WriteData(0x05); 
	
	All_Color(0x00, 0x00, 0x00);
	LCD_WriteIndex(0x29);//Display on	 	
}

void All_Color(uint8_t color_red, uint8_t color_green, uint8_t color_blue)
{
  uint16_t index;
	LCD_SetRegion(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
	
  for(index = 0; index < PANEL_SIZE; index++)
  {
		LCD_WriteData((color_red & 0xF8) | (color_green >> 5));
		LCD_WriteData((color_green << 5 )| (color_blue>> 3));
  }
}
void LCD_ColorBar(void)
{
  uint16_t lx,ly;
  uint8_t color_red,color_green,color_blue;

	LCD_SetRegion(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);

  for(ly=0; ly<Y_MAX_PIXEL; ly++)
	{
		for(lx=0; lx<X_MAX_PIXEL; lx++)
		{
			if(lx<(X_MAX_PIXEL/4))
			{
				color_red = (ly*255)/Y_MAX_PIXEL;
				color_green = color_blue = 0;	
			}
			else if(lx<(X_MAX_PIXEL/2))
			{
				color_green = (ly*255)/Y_MAX_PIXEL;
				color_red = color_blue = 0;
			}
			else if(lx<(3*X_MAX_PIXEL/4))
			{
				color_blue = (ly*255)/Y_MAX_PIXEL;
				color_red = color_green = 0;
			}
			else
				color_green = color_red = color_blue = (ly*255)/Y_MAX_PIXEL;
			
			LCD_WriteData((color_red & 0xF8) | ((color_green >> 5)&0x07));
			LCD_WriteData(((color_green << 3 )&0xE0) | ((color_blue>> 3)&0x1F));					  
		}
	}       
}
/*void LCD_TempBarArea(uint16_t add_sx, uint16_t add_ex, uint16_t add_sy, uint16_t add_ey)
{
  uint16_t lx,ly;
  uint8_t LCD_LB, LCD_HB, temp;

	LCD_SetRegion(add_sx, add_sy, add_ex, add_ey);

	for(ly=1; ly<Y_MAX_PIXEL; ly++)
	{
		for(lx=add_sx-1; lx<X_MAX_PIXEL; lx++)
		{	
			temp = (ly*255)/((add_ey-add_sy+1));
			LCD_LB = (uint8_t)(THERMAL_COLOR[temp]);
			LCD_HB = (uint8_t)(THERMAL_COLOR[temp]>>8);
			LCD_WriteData(LCD_LB);
			LCD_WriteData(LCD_HB);			  
		}  
	}
}*/

void LCD_GrayBarArea(uint16_t add_sx, uint16_t add_ex, uint16_t add_sy, uint16_t add_ey)
{
  uint16_t lx,ly;
  uint8_t color_red,color_green,color_blue;

  LCD_SetRegion(add_sx, add_sy, add_ex, add_ey);

	for(ly=1; ly<Y_MAX_PIXEL; ly++)
	{
		for(lx=add_sx-1; lx<X_MAX_PIXEL; lx++)
		{
			color_green = color_red = color_blue = (ly*255)/(add_ey-add_sy+1);
			LCD_WriteData((color_red & 0xF8) | ((color_green >> 5)&0x07));
			LCD_WriteData(((color_green << 3 )&0xE0) | ((color_blue>> 3)&0x1F));			  
		}  
	}
}
void LCD_PutChar_Vertical(uint16_t adx, uint16_t ady, const char *p)
{
	uint8_t index1, index2;
	uint8_t tmp;
	LCD_SetRegion(adx, ady, adx+8-1, ady+8-1);
	
	for(index1=0; index1<8; index1++)
	{
		tmp = *p;
		for(index2=0; index2<8; index2++)
		{
			if(tmp & 0x80)
			{
				LCD_WriteData((uint8_t)font_color);
				LCD_WriteData((uint8_t)(font_color>>8));
			}
			else
			{
				LCD_WriteData((uint8_t)COLOR_FONT_BACK);
				LCD_WriteData((uint8_t)(COLOR_FONT_BACK>>8));
			}

			tmp<<=1;
		}
		p++;
	}
}
void LCD_DisplayString_Vertical(uint16_t addx, uint16_t addy, const char *p )
{
	unsigned char tmp;    
	const char *fp;
	while(*p!='\0')
	{       
		tmp = *p -0x20;
		fp=&ASCII_TABLE_8x8_V[tmp * 8];
		LCD_PutChar_Vertical(addx, addy, fp);
		addy+=8;
		p++;
	}
}
void LCD_DisplayTemperature(uint16_t addx, uint16_t addy, int16_t temp)
{
	char table[9];
	
	if(temp>0)
		table[0]=	'+';
	else{
		table[0]= '-';
		temp = ~temp +1;}
	
	table[1]= (temp/1000) + 0x30;
	table[2]= (temp%1000)/100 + 0x30;
	table[3]= (temp%100)/10 + 0x30;
	table[4]=	'.';
	table[5]= temp%10 + 0x30;
	table[6]= '~'+1;
	table[7]= 'C';
	table[8]= '\0';
	
	if(table[0] == '-')
		font_color = COLOR_BLUE;
	else if(temp<378)
		font_color = COLOR_GREEN;
	else
		font_color = COLOR_RED;
		
	LCD_DisplayString_Vertical(addx, addy, table);
}

void LCD_DisplayNumber(uint16_t addx, uint16_t addy, uint32_t num)
{
	char table[8];
	if(num>=100000) {
		table[0]= num/100000 + 0x30;
	}
	else{
		table[0]=' ';
	}
	if(num>=10000) {
		table[1]= (num%100000)/10000 + 0x30;
	}
	else{
		table[1]=' ';
	}
	if(num>=1000) {
		table[2]= (num%10000)/1000 + 0x30;
		table[3]=',';
	}
	else{
		table[2]=' ';
		table[3]=' ';
	}
	if(num>=100) {
		table[4]=	(num%1000)/100 + 0x30;
	}
	else{
		table[4]=' ';
	}
	if(num>=10){
		table[5]= (num%100)/10 + 0x30;
	}
	else{
		table[5]=' ';
	}
	table[6]= (num%10) + 0x30;
	table[7]= '\0';
	LCD_DisplayString_Vertical(addx, addy, table);
}

void LCD_DisplayFloat(uint16_t addx, uint16_t addy, float num){
	char table[10];
	int value;
	
	if(num>=10000) {
		table[0]= 'E';
	}
	else{
		if(num<0){
			table[0]='-';
			num=-num;
		}
		else table[0]=' ';
	}
	if(num>=1000) {
		table[1]= ((int)num%10000)/1000 + 0x30;
	}
	else{
		table[1]=' ';
	}
	if(num>=100) {
		table[2]= ((int)num%1000)/100 + 0x30;
	}
	else{
		table[2]=' ';
	}
	if(num>=10) {
		table[3]=	((int)num%100)/10 + 0x30;
	}
	else{
		table[3]=' ';
	}
	table[4]= ((int)num%10) + 0x30;
	table[5]= '.';
	num*=1000;
	value=(int)num%1000;
	table[6]=(value%1000)/100+0x30;
	table[7]=(value%100)/10+0x30;
	table[8]=(value%10)+0x30;
	table[9]='\0';
	LCD_DisplayString_Vertical(addx, addy, table);
}

void LCD_ShowLogo(uint16_t adx, uint16_t ady, const uint16_t *bitmap)
{
	uint16_t index;
	uint16_t *bitmap_ptr = (uint16_t *)bitmap;
	uint8_t point1, point2;

	LCD_SetRegion(adx, ady, 60-1+adx, 76-1+ady);
	
	for(index = 0; index < 60*76; index++)
	{
		point1= *bitmap_ptr>>8;
		point2= *bitmap_ptr++;
		
		LCD_WriteData(point1);
		LCD_WriteData(point2);
	}
}

void LCD_DrawPoint(uint16_t ady, uint16_t adx,uint16_t colour)
{
	LCD_SetRegion(adx, ady, adx, ady);
	
	LCD_WriteData((colour>>8)&0xFF);
	LCD_WriteData(colour&0xFF);
}

void Buffer_PutChar_Vertical(uint16_t pos, const char *p, uint16_t *buffer, uint16_t color)
{
	uint8_t index_x, index_y;
	uint8_t tmp;
	
	for(index_x=0; index_x<8; index_x++)
	{
		tmp = *p;
		for(index_y=0; index_y<8; index_y++)
		{
			if(tmp & 0x80)
				buffer[pos + index_x*120 + index_y] = color;
			tmp<<=1;
		}
		p++;
	}
}
void Buffer_AddString_Vertical(uint16_t pos_s, const char *p, uint16_t *buffer, uint16_t color_s)
{
	unsigned char tmp;    
	const char *fp;
	uint16_t i=0;
	while(*p!='\0')
	{
		tmp = *p -0x20;
		fp=&ASCII_TABLE_8x8_V[tmp * 8];
		Buffer_PutChar_Vertical(i + pos_s, fp, buffer, color_s);
		i+=(8*120);
		p++;
	}
}

void Buffer_AddTemperature(uint16_t pos_t, int16_t temp, uint16_t *buffer)
{
	char table[9];
	
	if(temp>0)
		table[0]=	'+';
	else{
		table[0]= '-';
		temp = ~temp +1;}
	
	table[1]=	(temp/1000) + 0x30;
	table[2]= (temp%1000)/100 + 0x30;
	table[3]= (temp%100)/10 + 0x30;
	table[4]= '.';
	table[5]= temp%10 + 0x30;
	table[6]=	'~'+1;
	table[7]= 'C';
	table[8]= '\0';

	if(table[0] == '-')
		Buffer_AddString_Vertical(pos_t, table, buffer, COLOR_BLUE);
	else if(temp<378)
		Buffer_AddString_Vertical(pos_t, table, buffer, COLOR_GREEN);
	else
		Buffer_AddString_Vertical(pos_t, table, buffer, COLOR_RED);
}

/*void Buffer_AddCenterFrame(uint8_t length, uint16_t *buffer)
{
	uint8_t i;
	
	for(i=0; i<length; i++)
	{
		buffer[120*(160/2 -length/2+1) - 120/2 - length/2 + i] = 
		buffer[120*(160/2 +length/2) - 120/2 - length/2 + i] = 
		buffer[120*(160/2 -length/2+1+i) - 120/2 - length/2] = 
		buffer[120*(160/2 -length/2+1+i) - 120/2 + length/2-1] = (center_temp<378? COLOR_GREEN:COLOR_RED);
	}
}*/

/*void USB_AddString(uint16_t pos_x, uint16_t pos_y, const char *p)
{
	unsigned char tmp;    
	const char *fp;
	while(*p!='\0')
	{       
		tmp = *p -0x20;
		fp=&ASCII_Table_8x8[tmp * 8];
		USB_PutChar(pos_x, pos_y, fp);
		pos_x+=8;
		p++;
	}  
}*/

/*void USB_PutChar(uint16_t pos_x, uint16_t pos_y, const char *p)
{
	uint16_t index1, index2;
	uint8_t tmp;
	
	for(index1=pos_y; index1<8+pos_y ;index1++)
	{
		tmp=*p;
		for(index2=pos_x; index2<8+pos_x; index2++)
		{
			display_buffer_usb[index1][index2] = (tmp&0x01)? 0xFF:0x00;
			tmp>>=1;
		}
		p++;
	}
}*/
