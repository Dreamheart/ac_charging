
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"

#include "drv_inc.h"
#include "Bsp_inc.h"


uint8_t g_bLcdBuf[18];
uint8_t g_bLcdTmp[64];

void SSEL_Low ( void )
{
    GPIO_OutputValue ( 0, 1 << 6, 0 ) ;                               //片选为低
}

void SSEL_High ( void )
{
    GPIO_OutputValue ( 0, 1 << 6, 1 ) ;                               //片选为高
}

void SRST_Low ( void )
{
    GPIO_OutputValue ( 4, 1 << 14, 0 ) ;                              //片选为低
}

void SRST_High ( void )
{
    GPIO_OutputValue ( 4, 1 << 14, 1 ) ;                              //片选为高
}

void SSDI_Low ( void )
{
    GPIO_OutputValue ( 0, 1 << 8, 0 ) ;                               //片选为低
}

void SSDI_High ( void )
{
    GPIO_OutputValue ( 0, 1 << 8, 1 ) ;                               //片选为高
}

void SSCK_Low ( void )
{
    GPIO_OutputValue ( 0, 1 << 7, 0 ) ;                               //片选为低
}

void SSCK_High ( void )
{
    GPIO_OutputValue ( 0, 1 << 7, 1 ) ;                               //片选为高
}
void SSDO_Low ( void )
{
    GPIO_OutputValue ( 0, 1 << 9, 0 ) ;                               //片选为低
}

void SSDO_High ( void )
{
    GPIO_OutputValue ( 0, 1 << 9, 1 ) ;                               //片选为高
}

void LCDInit ( void )
{
    SSP_CFG_Type SSP_ConfigStruct;
    
    PINSEL_ConfigPin ( 0, 6, 0 );
    PINSEL_ConfigPin ( 0, 7, 2 );
    PINSEL_ConfigPin ( 0, 8, 0 );
    PINSEL_ConfigPin ( 0, 9, 2 );
    PINSEL_ConfigPin ( 4, 14, 0 );
    GPIO_SetDir ( 0, 1 << 6, GPIO_DIRECTION_OUTPUT );
    GPIO_SetDir ( 0, 1 << 8, GPIO_DIRECTION_OUTPUT );
    GPIO_SetDir ( 4, 1 << 14, GPIO_DIRECTION_OUTPUT );
    /* Initializing SSP device section ------------------------------------------------------ */
    // initialize SSP configuration structure to default
    SSP_ConfigStructInit ( &SSP_ConfigStruct );

    // Initialize SSP peripheral with parameter given in structure above
    SSP_Init ( LPC_SSP1, &SSP_ConfigStruct );

    // Enable SSP peripheral
    SSP_Cmd ( LPC_SSP1, ENABLE );
    
    SSEL_High();
    SRST_High();
    
    LCD_Init();
}
//==============================================================================
//============================================================================== 
uint8_t Spi_Operation(uint8_t src,uint8_t len)
{
    uint8_t rt;
    rt = SPIx_ReadWriteByte(LPC_SSP1,src);
    return rt;
}
//==============================================================================
void Lcd_Send(uint8_t *data,uint8_t type,uint8_t cnt)
{
    uint8_t i;
    SSEL_Low();
    
    if(type == LCD_CMD){
        SSDI_Low();
    }
    else {
        SSDI_High();
    }
    for(i = 0;i < cnt;i++) {
        Spi_Operation(*data++,8);
    }
    
    SSEL_High();
}
//==============================================================================
void Lcd_Reset(void)
{
    SRST_Low();
    SRST_High();
}

void Lcd_Clr(void)
{
    uint8_t i;
    g_bLcdTmp[0] = 0x15;
    g_bLcdTmp[1] = 0x00;
    g_bLcdTmp[2] = 0x3f;
    g_bLcdTmp[3] = 0x75;
    g_bLcdTmp[4] = 0x00;
    g_bLcdTmp[5] = 0x3f;
    Lcd_Send(g_bLcdTmp,LCD_CMD,6);
    
    memset(g_bLcdTmp,0,64);
    for(i = 0;i < 64;i++){
      Lcd_Send(g_bLcdTmp,LCD_DATA,64);
    }
}

void LCD_Init(void)
{
    uint8_t *pdata = g_bLcdTmp;
//    Lcd_Reset();
    *pdata++ = 0xAE; 
    *pdata++ = 0xB3; 
    *pdata++ = 0x91; 
    *pdata++ = 0xA8; 
    *pdata++ = 0x3F; 
    *pdata++ = 0xA2; 
    *pdata++ = 0x44; 
    *pdata++ = 0xA1; 
    *pdata++ = 0x00; 
    *pdata++ = 0xA0; 
    *pdata++ = 0x63; 
    *pdata++ = 0xAD; 
    *pdata++ = 0x02; 
    *pdata++ = 0x81; 
    *pdata++ = 0x40; 
    *pdata++ = 0x86; 
    *pdata++ = 0xBE; 
    *pdata++ = 0x02; 
    *pdata++ = 0xBF; 
    *pdata++ = 0x0E; 
    *pdata++ = 0xA4; 
    *pdata++ = 0x23; 
    *pdata++ = 0x01; 
    *pdata++ = 0x24; 
    *pdata++ = 0x00; 
    *pdata++ = 0x00; 
    *pdata++ = 0x3F; 
    *pdata++ = 0x3F; 
    *pdata++ = 0x00; 
    *pdata++ = 0xAF; 
    Lcd_Send(g_bLcdTmp,LCD_CMD,30);
    Lcd_Clr();
}

void write_4_byte(uint8_t data)
{
    uint8_t k,kk,kkk;
    uint8_t data_4byte[4];
    kk = data;
    for(k = 0;k < 4;k++)
    {
        kkk=kk&0x03;
        switch(kkk)
        {
            case 0x00:
                data_4byte[k] = 0x00;
                break;
            case 0x01:
                data_4byte[k] = 0x0f;
                break;
            case 0x02:
                data_4byte[k] = 0xf0;
                break;
            case 0x03:
                data_4byte[k] = 0xff;
                break;
            default:break;
        }
        kk = kk>>2;
    }
    Lcd_Send(data_4byte,LCD_DATA,4); /* 8 column  a nibble of command is a dot*/
}

void Lcd_Printf(uint8_t row,uint8_t col,uint8_t reverse,uint8_t *pstr)
{
    uint8_t i;
    uint8_t tmp,tmpr,tmpc;
    const uint8_t *plcd;

    if(pstr == NULL)return;
    if(row > 0x3f)return;
    if(col > 0x3f)return;

    tmpc = col;
    tmpr = row;
    while(*pstr != '\0')
    {
        if(*pstr < 0xa0)
        {
            g_bLcdTmp[0] = 0x15;
            g_bLcdTmp[1] = tmpc;
            g_bLcdTmp[2] = tmpc+0x03;
            g_bLcdTmp[3] = 0x75;
            g_bLcdTmp[4] = tmpr;
            g_bLcdTmp[5] = tmpr+0x0b;
            Lcd_Send(g_bLcdTmp,LCD_CMD,6);

            plcd = AsciiLib[0] + (*pstr - ' ')*12;
            
            for(i = 0;i < 12;i++)
            {
                if(reverse == TRUE)
                {
                    tmp = ~*(plcd + i);
                }
                else tmp = *(plcd + i);
                write_4_byte(tmp);
            }
            tmpc += 3;
            pstr++;
        }
        else
        {
            g_bLcdTmp[0] = 0x15;
            g_bLcdTmp[1] = tmpc;
            g_bLcdTmp[2] = tmpc+0x07;
            g_bLcdTmp[3] = 0x75;
            g_bLcdTmp[4] = tmpr;
            g_bLcdTmp[5] = tmpr+0x0b;
            Lcd_Send(g_bLcdTmp,LCD_CMD,6);
            
            uint8_t codeh,codel;
            for(i = 0;i < (sizeof(FontCnIndex)>>1);i++)
            {
                codeh = (uint8_t)(FontCnIndex[i]>>8);
                codel = (uint8_t)(FontCnIndex[i]);
                if(*pstr == codeh && *(pstr+1) == codel)
                {
                    break;
                }
            }
            plcd = (uint8_t*)(FontCnLib+i);
            
            for(i = 0;i < 24;i++)
            {
                if(reverse == TRUE)
                {
                    tmp = ~*(plcd + i);
                }
                else tmp = *(plcd + i);
                write_4_byte(tmp);
            }
            tmpc += 6;
            pstr += 2;
        }
    }
}

void Lcd_ClearLineN(uint8_t row,uint8_t col,uint8_t Reserve,uint8_t num)
{
    uint8_t i;
    uint8_t tmp,tmpr,tmpc;
    const uint8_t *plcd;
    
    if(num > 0x10)return;
    if(row > 0x3f)return;
    if(col > 0x3f)return;

    tmpc = col;
    tmpr = row;
    while(num-- != 0){
        g_bLcdTmp[0] = 0x15;
        g_bLcdTmp[1] = tmpc;
        g_bLcdTmp[2] = tmpc+0x03;
        g_bLcdTmp[3] = 0x75;
        g_bLcdTmp[4] = tmpr;
        g_bLcdTmp[5] = tmpr+0x0f;
        Lcd_Send(g_bLcdTmp,LCD_CMD,6);

        plcd = AsciiLib[0];
        
        for(i = 0;i < 12;i++)
        {
            if(Reserve == TRUE)
            {
                tmp = ~*(plcd + i);
            }
            else tmp = *(plcd + i);
            write_4_byte(tmp);
        }
        tmpc += 3;
    }
}