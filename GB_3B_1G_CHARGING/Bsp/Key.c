
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"

#include "Bsp_inc.h"
#include "app_inc.h"

struct KEYBOARD KeyBoardType;

const uint8_t g_KeyCode[] = {
      0x28,0x11,0x21,0x41,0x12,0x22,0x42,0x14,
      0x24,0x44,0x18,0x48,0x88,0x84,0x82,0x81,
      0x58,
};

const uint8_t KeyBoardConfig[8][2]=
{
    1,20,
    1,21,
    1,22,
    1,23,
    1,24,
    4,1,
    1,26,
    1,27,
};

void KeyBoardInit(void)
{
    uint8_t i;
    for(i = 0 ;i < 8;i++)
    {
        PINSEL_ConfigPin ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), 0 );//将所用IO引脚设置为GPIO功能
    }
}

void SetRowInput(void)
{
    uint8_t i;
    for(i = 0 ;i < 8;i++)
    {
        GPIO_SetDir ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), GPIO_DIRECTION_INPUT );
    }
    for(i = 4 ;i < 8;i++)
    {
        GPIO_SetDir ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), GPIO_DIRECTION_OUTPUT );
        GPIO_OutputValue ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), 0 );
    }
}

void SetLineInput(void)
{
    uint8_t i;
    for(i = 0 ;i < 8;i++)
    {
        GPIO_SetDir ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), GPIO_DIRECTION_INPUT );
    }
    for(i = 0 ;i < 4;i++)
    {
        GPIO_SetDir ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), GPIO_DIRECTION_OUTPUT );
        GPIO_OutputValue ( KeyBoardConfig[i][0], (1<<KeyBoardConfig[i][1]), 0 );
    }
}

uint8_t GetRowValue(void)
{
   uint32_t tmp;
   uint8_t i,rtn = 0;
   for(i = 0;i < 4;i++)
   {
        tmp = GPIO_ReadValue (KeyBoardConfig[i][0]);
        if(!(tmp&(1<<KeyBoardConfig[i][1])))
        {
             rtn |= 1<<i;
        }
   }
   return rtn;
}

uint8_t GetLineValue(void)
{
   uint32_t tmp;
   uint8_t i,rtn = 0;
   for(i = 4;i < 8;i++)
   {
        tmp = GPIO_ReadValue (KeyBoardConfig[i][0]);
        if(!(tmp&(1<<KeyBoardConfig[i][1])))
        {
             rtn |= 1<<i;
        }
   }
   return rtn;
}

//==============================================================================
void KeyBoard_Scan(void)
{
    uint8_t i;
    uint32_t tmp;
//    if(Sys_Start_Flag == 0)return;
    if(KeyBoardType.flagfull == 1)return;
    if(SysTickCnt - KeyBoardType.scanticks < KEYSCANDLY)return;
    KeyBoardType.scanticks = SysTickCnt;

    switch(KeyBoardType.status) {
    case 0:
        SetRowInput();
        KeyBoardType.status++;
        break;
    case 1:
        tmp = GetRowValue();
        if(tmp != 0)
        {
            KeyBoardType.code = tmp;
            KeyBoardType.status++;
        }
        else
        {
            KeyBoardType.code = 0;
        }
        KeyBoardType.keycnt = 0;
        break;
    case 2:
        tmp = GetRowValue();
        if(tmp == KeyBoardType.code)
        {
            if(++KeyBoardType.keycnt > KEYSCANCNT)
            {
                KeyBoardType.status++;
            }
        }
        else
        {
            KeyBoardType.keycnt = 0;
            KeyBoardType.code = 0;
            KeyBoardType.status = 1;
        }
        break;
    case 3:
        SetLineInput();
        KeyBoardType.status++;
        break;
    case 4:
        tmp = GetLineValue();
        if(tmp != 0)
        {
            KeyBoardType.code += tmp;
            KeyBoardType.status++;
        }
        else
        {
            KeyBoardType.code = 0;
            KeyBoardType.status = 0;
        }
        KeyBoardType.keycnt = 0;
        break;  
    case 5:
        tmp = GetLineValue();
        if(tmp == (KeyBoardType.code&0xf0))
        {
            if(++KeyBoardType.keycnt > KEYSCANCNT)
            {
                KeyBoardType.status++;
            }
        }
        else
        {
            KeyBoardType.keycnt = 0;
            KeyBoardType.code = 0;
            KeyBoardType.status = 0;
        }
        break;
      
    case 6:
        for(i = 0;i < sizeof(g_KeyCode);i++)
        {
            if(KeyBoardType.code == g_KeyCode[i])
            {
                break;
            }
        }
        if(i >= sizeof(g_KeyCode))
        {
            KeyBoardType.keycnt = 0;
            KeyBoardType.code = 0;
            KeyBoardType.status = 0;
            break;
        }
        KeyBoardType.value[KeyBoardType.product] = i;
        i = (KeyBoardType.product+1)%KEYMAXNUM;
        
        if(i == KeyBoardType.consume)
        {
            KeyBoardType.flagfull = 1;
        }
        else
        {
            KeyBoardType.product = i;
        }
        KeyBoardType.status++;
        FlipOutput(LED2);
//        BUZZER_ON;
//        SysBlockTick.buzzercnt = BUZZERDLY;
        break;
    case 7:
        if(KeyBoardType.dly++ > KEYRPTDLY/KEYSCANDLY) {
             KeyBoardType.dly = 0;
             KeyBoardType.status = 0;
             KeyBoardType.keycnt = 0;
             KeyBoardType.code = 0;
        }
        break;
    default:break;
    }
}
//==============================================================================
//==============================================================================