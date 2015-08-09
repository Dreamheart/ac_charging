#ifndef __LCD_H__
#define __LCD_H__

#define LCD_DATA       0x00                               //data identification
#define LCD_CMD        0x01                               //code identification

extern void LCDInit ( void );
extern void LCD_Init(void);
extern void Lcd_Printf(uint8_t row,uint8_t col,uint8_t reverse,uint8_t *pstr);
extern void Lcd_ClearLineN(uint8_t row,uint8_t col,uint8_t Reserve,uint8_t num);

#endif
