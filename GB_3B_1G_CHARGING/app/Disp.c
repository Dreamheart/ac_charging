
#include "Bsp_inc.h"
#include "app_inc.h"

DISPTCB Disp_TCB_Tbl[SRCNNUM];
DISPTCB *Disp_TCB_List;
DISPTCB *Disp_TCB_Cur;
DISPTCB *Disp_TCB_Last;
DISPTCB *Disp_TCB_Card;
uint8_t F_display_do = 0;

uint32_t ChargingTime = 0;
uint8_t RtcSecondBck;

uint32_t UserNumber[6]= {0x95,0x09,0x55,0x62,0x30,0x01};
uint32_t UserPassword = 0x00123456;

uint8_t g_bDispBuf[18];
uint8_t g_bDispTmp[64];

uint32_t CheckOverTimeTicks;

DISPCONFig Disp_Init_Array[SRCNNUM] = {
    {0x00,0x00,0x00,0x00,DisplayWelcome},
    {0x00,0x00,0x00,0x08,DisplayParameter},
    {0x80,0x00,0x02,0x00,DisplayLoginMode},
    {0x00,0x80,0x00,0x0b,DisplayUserLogin1},
    {0x00,0x80,0x00,0x06,DisplayUserLogin2},
    {0x00,0x00,0x00,0x00,DisplayLoginCheck},
    {0x00,0x00,0x00,0x00,DisplayLoginCheckError},
    {0x00,0x00,0x00,0x00,DisplayLoginLock},
    {0x00,0x00,0x00,0x00,DisplayQRLogin},
    {0x00,0x00,0x00,0x00,DisplayUserInfo},
    {0x00,0x00,0x00,0x00,DisplayLinkInfo},
    {0x00,0x00,0x00,0x00,DisplayUsingInfo1},
    {0x00,0x00,0x00,0x00,DisplayUsingInfo2},
    {0x00,0x00,0x00,0x00,DisplaySettlementInfo},
};
//==============================================================================
//==============================================================================
void Disp_TCB_Init(void)
{
    uint8_t i;
    memset(Disp_TCB_Tbl,0,sizeof(Disp_TCB_Tbl));

    for(i = 0;i < SRCNNUM;i++) {
        memcpy(&Disp_TCB_Tbl[i].select,&Disp_Init_Array[i].select,sizeof(Disp_Init_Array[0]));
    }
    Disp_TCB_List = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Cur = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Last = NULL;
    Disp_TCB_Card = NULL;
                                                                       //welcome
    Disp_TCB_Tbl[SCREENWELCOME].TcbPrev    = NULL;
    Disp_TCB_Tbl[SCREENWELCOME].TcbNext[0] = &Disp_TCB_Tbl[SCREENLOGINMODE];  
    Disp_TCB_Tbl[SCREENWELCOME].TcbNext[1] = &Disp_TCB_Tbl[SCREENPARAMETER];
    Disp_TCB_Tbl[SCREENWELCOME].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENWELCOME].TcbNext[3] = NULL;
                                                                   //log in mode
    Disp_TCB_Tbl[SCREENLOGINMODE].TcbPrev = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Tbl[SCREENLOGINMODE].TcbNext[0] = &Disp_TCB_Tbl[SCREENUSERLOGIN1];  
    Disp_TCB_Tbl[SCREENLOGINMODE].TcbNext[1] = &Disp_TCB_Tbl[SCREENQRLOGIN];
    Disp_TCB_Tbl[SCREENLOGINMODE].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENLOGINMODE].TcbNext[3] = NULL;
                                                                    // parameter
    Disp_TCB_Tbl[SCREENPARAMETER].TcbPrev    = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Tbl[SCREENPARAMETER].TcbNext[0] = NULL;
    Disp_TCB_Tbl[SCREENPARAMETER].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENPARAMETER].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENPARAMETER].TcbNext[3] = NULL;
                                                                    // user mode
    Disp_TCB_Tbl[SCREENUSERLOGIN1].TcbPrev    = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENUSERLOGIN1].TcbNext[0] = &Disp_TCB_Tbl[SCREENUSERLOGIN2];
    Disp_TCB_Tbl[SCREENUSERLOGIN1].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENUSERLOGIN1].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENUSERLOGIN1].TcbNext[3] = NULL;                
                                                                    // user mode
    Disp_TCB_Tbl[SCREENUSERLOGIN2].TcbPrev    = &Disp_TCB_Tbl[SCREENUSERLOGIN1];
    Disp_TCB_Tbl[SCREENUSERLOGIN2].TcbNext[0] = &Disp_TCB_Tbl[SCREENLOGINCHECK];
    Disp_TCB_Tbl[SCREENUSERLOGIN2].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENUSERLOGIN2].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENUSERLOGIN2].TcbNext[3] = NULL;     
                                                                   // user check
    Disp_TCB_Tbl[SCREENLOGINCHECK].TcbPrev    = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Tbl[SCREENLOGINCHECK].TcbNext[0] = &Disp_TCB_Tbl[SCREENUSERINFO];
    Disp_TCB_Tbl[SCREENLOGINCHECK].TcbNext[1] = &Disp_TCB_Tbl[SCREENLOGINCHECKERROR];
    Disp_TCB_Tbl[SCREENLOGINCHECK].TcbNext[2] = &Disp_TCB_Tbl[SCREENLOGINLOCK];
    Disp_TCB_Tbl[SCREENLOGINCHECK].TcbNext[3] = NULL;    
                                                             // user check error
    Disp_TCB_Tbl[SCREENLOGINCHECKERROR].TcbPrev    = NULL;
    Disp_TCB_Tbl[SCREENLOGINCHECKERROR].TcbNext[0] = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENLOGINCHECKERROR].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENLOGINCHECKERROR].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENLOGINCHECKERROR].TcbNext[3] = NULL; 
                                                                  // user locked
    Disp_TCB_Tbl[SCREENLOGINLOCK].TcbPrev    = NULL;
    Disp_TCB_Tbl[SCREENLOGINLOCK].TcbNext[0] = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENLOGINLOCK].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENLOGINLOCK].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENLOGINLOCK].TcbNext[3] = NULL;
                                                                      // QR mode
    Disp_TCB_Tbl[SCREENQRLOGIN].TcbPrev    = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENQRLOGIN].TcbNext[0] = &Disp_TCB_Tbl[SCREENLOGINCHECK];
    Disp_TCB_Tbl[SCREENQRLOGIN].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENQRLOGIN].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENQRLOGIN].TcbNext[3] = NULL;
                                                                    // user info
    Disp_TCB_Tbl[SCREENUSERINFO].TcbPrev    = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENUSERINFO].TcbNext[0] = &Disp_TCB_Tbl[SCREENLINKINFO];
    Disp_TCB_Tbl[SCREENUSERINFO].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENUSERINFO].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENUSERINFO].TcbNext[3] = NULL;
                                                                    // user info
    Disp_TCB_Tbl[SCREENLINKINFO].TcbPrev    = &Disp_TCB_Tbl[SCREENLOGINMODE];
    Disp_TCB_Tbl[SCREENLINKINFO].TcbNext[0] = &Disp_TCB_Tbl[SCREENUSINGINFO1];
    Disp_TCB_Tbl[SCREENLINKINFO].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENLINKINFO].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENLINKINFO].TcbNext[3] = NULL;
                                                                     // use info
    Disp_TCB_Tbl[SCREENUSINGINFO1].TcbPrev    = NULL;
    Disp_TCB_Tbl[SCREENUSINGINFO1].TcbNext[0] = &Disp_TCB_Tbl[SCREENSETTLEMENTINFO];
    Disp_TCB_Tbl[SCREENUSINGINFO1].TcbNext[1] = &Disp_TCB_Tbl[SCREENUSINGINFO2];
    Disp_TCB_Tbl[SCREENUSINGINFO1].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENUSINGINFO1].TcbNext[3] = NULL;
                                                                     // use info
    Disp_TCB_Tbl[SCREENUSINGINFO2].TcbPrev    = NULL;
    Disp_TCB_Tbl[SCREENUSINGINFO2].TcbNext[0] = &Disp_TCB_Tbl[SCREENSETTLEMENTINFO];
    Disp_TCB_Tbl[SCREENUSINGINFO2].TcbNext[1] = &Disp_TCB_Tbl[SCREENUSINGINFO1];
    Disp_TCB_Tbl[SCREENUSINGINFO2].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENUSINGINFO2].TcbNext[3] = NULL;
                                                              // settlement info
    Disp_TCB_Tbl[SCREENSETTLEMENTINFO].TcbPrev    = &Disp_TCB_Tbl[SCREENWELCOME];
    Disp_TCB_Tbl[SCREENSETTLEMENTINFO].TcbNext[0] = NULL;
    Disp_TCB_Tbl[SCREENSETTLEMENTINFO].TcbNext[1] = NULL;
    Disp_TCB_Tbl[SCREENSETTLEMENTINFO].TcbNext[2] = NULL;
    Disp_TCB_Tbl[SCREENSETTLEMENTINFO].TcbNext[3] = NULL;

}
//==============================================================================
//==============================================================================
uint8_t KeyBoard_Get(void)
{
    uint8_t value;
    if(KeyBoardType.consume == KeyBoardType.product)return 0xff;
    value = KeyBoardType.value[KeyBoardType.consume];
    KeyBoardType.consume = (KeyBoardType.consume+1)%KEYMAXNUM;
    KeyBoardType.flagfull = 0; 
    return value;
}
//==============================================================================
//==============================================================================
uint8_t GetDisplayScreenNum(void)
{
    uint8_t i;
    for(i = 0;i < SRCNNUM;i++){
        if(&Disp_TCB_Tbl[i] == Disp_TCB_Cur)break;
    }
    return i;
}
//==============================================================================
//==============================================================================
void KeyBoard_Deal(void)
{   
    uint8_t i,j,value;
    uint32_t tmp;
    
    value = KeyBoard_Get();

    if(value == 0xff)return;
    
    i = GetDisplayScreenNum();
    
    switch(value) {
    case KEY_9:
    case KEY_8:
    case KEY_7:
    case KEY_6:
      /*
        if(i == SRCNTIME){
            if((Disp_TCB_Cur->cursor&0x0f) == 3)break;
        }
      */
    case KEY_5:
    case KEY_4:
    case KEY_3:
    case KEY_2:
    case KEY_1:
    case KEY_0:
      if (i == SCREENLOGINMODE)
      {
        if(value  == KEY_1 ||value  == KEY_2)
        {
            Disp_TCB_Cur->select2 = Disp_TCB_Cur->select;
            Disp_TCB_Cur->select = (value-1) |0x80;
        }
        break;
      }
        if(i != SCREENPARAMETER 
           && i != SCREENLOGINMODE 
           && i != SCREENUSERLOGIN1
           && i != SCREENUSERLOGIN2)
        {
           break;
        }
        //----------------------------------------------------------------------
        if((Disp_TCB_Cur->cursor&0x0f) >= Disp_TCB_Cur->cursor_max)
        {
            break;
        }
        //----------------------------------------------------------------------
        g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = value + '0';
        Disp_TCB_Cur->cursor++;
        /*
        if(i == SRCNTIME) {
            if((Disp_TCB_Cur->cursor&0x0f) == 2){
                g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = ':';
                Disp_TCB_Cur->cursor++;
            }
        }
        */
        g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = '\0';
        //----------------------------------------------------------------------
        break;
    case KEY_DOT:
        //----------------------------------------------------------------------
 //       if(i != SRCNMONEY && i != SRCNENERGY )break;
        if((Disp_TCB_Cur->cursor&0x0f) == 0 || (Disp_TCB_Cur->cursor&0x20) != 0)break;
        //----------------------------------------------------------------------
        Disp_TCB_Cur->cursor |= 0x20;
        g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = '.';
        Disp_TCB_Cur->dotpos = Disp_TCB_Cur->cursor&0x0f;
        Disp_TCB_Cur->cursor++;
        g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = '\0';
        break;
    case KEY_ESC:
        if(i == SCREENLOGINCHECK )break;
        if(i == SCREENPARAMETER)
        {
            Disp_TCB_Cur->cursor &= 0x70;
        }
        if(Disp_TCB_Cur->TcbPrev == NULL)break;
        Disp_TCB_Cur = Disp_TCB_Cur->TcbPrev;
        F_display_do = TRUE;
        Disp_TCB_Cur->cursor &= 0xf0;
        memset(g_bDispBuf,0,sizeof(g_bDispBuf));
        break;
    case KEY_UP:
        if(i == SCREENUSINGINFO2)
        {
            F_display_do = TRUE;
            Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[1];
            break;
        }
        else if(i == SCREENLOGINMODE)
        {
            Disp_TCB_Cur->select2 = Disp_TCB_Cur->select;
            Disp_TCB_Cur->select = (Disp_TCB_Cur->select&0x3f)+Disp_TCB_Cur->count_max;
            Disp_TCB_Cur->select = (--Disp_TCB_Cur->select)%Disp_TCB_Cur->count_max;
            Disp_TCB_Cur->select |= Disp_TCB_Cur->select2&0x80;
        }
        break;
    case KEY_DOWN:
        if(i == SCREENUSINGINFO1)
        {
            F_display_do = TRUE;
            Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[1];
        }
        else if(i == SCREENLOGINMODE)
        {
            Disp_TCB_Cur->select2 = Disp_TCB_Cur->select;
            Disp_TCB_Cur->select = (++Disp_TCB_Cur->select&0x3f)%Disp_TCB_Cur->count_max;
            Disp_TCB_Cur->select |= Disp_TCB_Cur->select2&0x80;
        }
        break;
    case KEY_CANCEL:
        //----------------------------------------------------------------------
        if( i == SCREENPARAMETER 
            || i == SCREENUSERLOGIN1
            || i == SCREENUSERLOGIN2 )
        {
            if((Disp_TCB_Cur->cursor&0x0f)){
                Disp_TCB_Cur->cursor2 = Disp_TCB_Cur->cursor;
                Disp_TCB_Cur->cursor--;
                if(g_bDispBuf[Disp_TCB_Cur->cursor&0xf] == '.') {
                    Disp_TCB_Cur->cursor &= 0xdf;
                }
                g_bDispBuf[Disp_TCB_Cur->cursor&0x0f] = '\0';
                break;
            }
        }
        break;
    case KEY_ENTER:
        //----------------------------------------------------------------------
        if(i == SCREENLOGINCHECK || i == SCREENUSINGINFO2)
        {
            break;
        }
        if(i == SCREENPARAMETER) 
        {
            if((Disp_TCB_Cur->cursor&0x80) != 0x80)
            {
                Disp_TCB_Cur->cursor |= 0x80;
                Lcd_ClearLineN(34,CURSORPOS,0,10);
                break;
            }
            if((Disp_TCB_Cur->cursor&0x0f) < 8)break;
            g_sParaConfig.id[0] = ((g_bDispBuf[6]-'0')<<4)+(g_bDispBuf[7]-'0');
            g_sParaConfig.id[1] = ((g_bDispBuf[4]-'0')<<4)+(g_bDispBuf[5]-'0');
            g_sParaConfig.id[2] = ((g_bDispBuf[2]-'0')<<4)+(g_bDispBuf[3]-'0');
            g_sParaConfig.id[3] = ((g_bDispBuf[0]-'0')<<4)+(g_bDispBuf[1]-'0');
            SaveParameter();
            Disp_TCB_Cur->cursor &= 0x70;
            
        }
        else if(i == SCREENUSERLOGIN1)
        {
            if((Disp_TCB_Cur->cursor&0x0f) < Disp_TCB_Cur->cursor_max)break;
            UserNumber[5] = g_bDispBuf[0]-'0';
            UserNumber[4] = g_bDispBuf[1]-'0';
            UserNumber[4] = (UserNumber[4]<<4)+g_bDispBuf[2]-'0';
            UserNumber[3] = g_bDispBuf[3]-'0';
            UserNumber[3] = (UserNumber[3]<<4)+g_bDispBuf[4]-'0';
            UserNumber[2] = g_bDispBuf[5]-'0';
            UserNumber[2] = (UserNumber[2]<<4)+g_bDispBuf[6]-'0';
            UserNumber[1] = g_bDispBuf[7]-'0';
            UserNumber[1] = (UserNumber[1]<<4)+g_bDispBuf[8]-'0';
            UserNumber[0] = g_bDispBuf[9]-'0';
            UserNumber[0] = (UserNumber[0]<<4)+g_bDispBuf[10]-'0';
        }
        else if(i == SCREENUSERLOGIN2)
        {
            if((Disp_TCB_Cur->cursor&0x0f) < Disp_TCB_Cur->cursor_max)break;
            tmp = ((g_bDispBuf[0]-'0')<<20)+((g_bDispBuf[1]-'0')<<16)+((g_bDispBuf[2]-'0')<<12)
                  +((g_bDispBuf[3]-'0')<<8) +((g_bDispBuf[4]-'0')<<4)+((g_bDispBuf[5]-'0'));
            UserPassword = tmp;
            CheckOverTimeTicks = SysTickCnt;
        }
        else if(i == SCREENQRLOGIN)
        {
            CheckOverTimeTicks = SysTickCnt;
        }
        else if(i == SCREENLINKINFO)
        {   
            if(g_sAnalogValue.linkv == 6)
            {
                OUTPUTOPEN(); 
                ChargingTime = 0;
            }
            else 
            {
                break;
            }
        }
        else if(i == SCREENUSINGINFO1)
        {
            OUTPUTCLOSE();
        }
        //----------------------------------------------------------------------
        j = Disp_TCB_Cur->select&0x3f;
        if(Disp_TCB_Cur->TcbNext[j] != NULL) 
        {
            Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[j];
            Disp_TCB_Cur->select &= 0x80;
            Disp_TCB_Cur->cursor &= 0xc0;
            Disp_TCB_Cur->dotpos = 0;
            F_display_do = TRUE;
            memset(g_bDispBuf,0,sizeof(g_bDispBuf));
            F_display_do = TRUE;
        }
        memset(g_bDispBuf,0,sizeof(g_bDispBuf));
        F_display_do = TRUE;
        //----------------------------------------------------------------------
        break;
    case KEY_CFG:
        if(i == SCREENWELCOME)
        {
            Disp_TCB_Cur = &Disp_TCB_Tbl[SCREENPARAMETER];
            F_display_do = TRUE;
        }
    default:break;
    }
}
//==============================================================================
//==============================================================================
void DisplayWelcome(void)//0
{
    uint8_t *pData;
    pData = "欢迎使用";
    Lcd_Printf(10,20,0,pData);
    pData = "星星智能充电";
    Lcd_Printf(32,14,0,pData);
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayParameter(void)//1
{
    uint8_t *pData;
    uint8_t dispbuf[10];
    pData = "参数设置";
    Lcd_Printf(2,1,0,pData);
    pData = "1.充电桩编号";
    Lcd_Printf(18,6,0,pData);
    if((Disp_TCB_Cur->cursor&0x80) == 0x00)
    {
        dispbuf[0] = (g_sParaConfig.id[3]>>4) + '0';
        dispbuf[1] = (g_sParaConfig.id[3]&0x0f) + '0';
        dispbuf[2] = (g_sParaConfig.id[2]>>4) + '0';
        dispbuf[3] = (g_sParaConfig.id[2]&0x0f) + '0';
        dispbuf[4] = (g_sParaConfig.id[1]>>4) + '0';
        dispbuf[5] = (g_sParaConfig.id[1]&0x0f) + '0';
        dispbuf[6] = (g_sParaConfig.id[0]>>4) + '0';
        dispbuf[7] = (g_sParaConfig.id[0]&0x0f) + '0';
        dispbuf[8] = '\0';
        Lcd_Printf(34,CURSORPOS,0,dispbuf);
    }
    else 
    {
        Lcd_Printf(34,CURSORPOS,0,g_bDispBuf);
    }
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayLoginMode(void)//2
{
    uint8_t *pData;
    pData = "选择登陆方式";
    Lcd_Printf(2,1,0,pData);
    pData = "1.用户登陆";
    Lcd_Printf(18,8,0,pData);
    pData = "2.二维码登陆";
    Lcd_Printf(34,8,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayUserLogin1(void)//3
{
    uint8_t *pData;
    pData = "请输入用户名";
    Lcd_Printf(10,12,0,pData);
    Lcd_Printf(34,CURSORPOS,0,g_bDispBuf);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}
void DisplayUserLogin2(void)//4
{
    uint8_t *pData,i;
    uint8_t dispbuf[10];
    pData = "请输入密码";
    Lcd_Printf(10,12,0,pData);
    for(i=0;i < sizeof(g_bDispBuf);i++)
    {
        if(g_bDispBuf[i]!= '\0')
        {
            dispbuf[i] = '*';
        }
        else
        { 
          dispbuf[i] = '\0';
          break;
        }
    }

    Lcd_Printf(34,CURSORPOS,0,dispbuf);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}
void DisplayLoginCheck(void)//5
{
    uint8_t *pData;
    pData = "与服务器通信中";
    Lcd_Printf(10,10,0,pData);
    pData = "请稍等";
    Lcd_Printf(32,22,0,pData);
    
    if(SysTickCnt >= CheckOverTimeTicks+60000)
    {
        LoginCheckOverTime();
    }
}

void DisplayLoginCheckError(void)//6
{
    uint8_t *pData;
    pData = "用户与密码不符";
    Lcd_Printf(18,10,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayLoginLock(void)//7
{
    uint8_t *pData;
    pData = "密码输入错误超过3次";
    Lcd_Printf(2,1,0,pData);
    pData = "用户xxxxxx";
    Lcd_Printf(18,1,0,pData);
    pData = "已经冻结";
    Lcd_Printf(34,1,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayQRLogin(void)//8
{
    uint8_t *pData;
    pData = "请扫二维码";
    Lcd_Printf(16,16,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayUserInfo(void)//9
{
    uint8_t *pData;
    pData = "用户:00000001";
    Lcd_Printf(2,1,0,pData);
    pData = "余额:1000.00元";
    Lcd_Printf(18,1,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

void DisplayLinkInfo(void)//10
{
    uint8_t *pData;
    
    g_sAnalogValue.linkv = GetAnalogSta();
    
    if(g_sAnalogValue.linkv == 12)
    {  
        pData = "连接器: 断开";
        Lcd_Printf(2,1,0,pData);
        ClrOutput(LED2);
    }
    else if(g_sAnalogValue.linkv == 9)
    {  
        pData = "连接器: 连接";
        Lcd_Printf(2,1,0,pData);
        SetOutput(LED2);
    }
    else if(g_sAnalogValue.linkv == 6)
    {  
        pData = "连接器: 充电";
        Lcd_Printf(2,1,0,pData);
        ClrOutput(LED2);
        pData = "启动充电";
        Lcd_Printf(50,1,0,pData);
        
        pData = "确定";
        Lcd_Printf(50,50,0,pData);
    }
}

void DisplayUsingInfo1(void)//11
{
    uint8_t *pData;
    uint16_t minute; 
    uint8_t dispbuf[10];
    pData = "充电时长:";
    Lcd_Printf(2,1,0,pData);
    pData = "分";
    dispbuf[0] = ((uint8_t)(ChargingTime/60000)&0x0f) + '0';
    dispbuf[1] = ((ChargingTime/6000)&0x0f) + '0';
    dispbuf[2] = ((ChargingTime/600)&0x0f) + '0';
    dispbuf[3] = ((ChargingTime/60)&0x0f) + '0';
    dispbuf[4] = *pData;
    dispbuf[5] = *(pData+1);
    dispbuf[6] = '\0';
    Lcd_Printf(2,28,0,dispbuf);
    pData = "充电电量:10.00Kwh";
    Lcd_Printf(18,1,0,pData);

    pData = "down";
    Lcd_Printf(18,50,0,pData);
    
    pData = "停止充电";
    Lcd_Printf(50,1,0,pData);
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
    
    if(CurTime.SEC != RtcSecondBck)
    {
       ChargingTime++;
       RtcSecondBck = CurTime.SEC;
    }
    
    g_sAnalogValue.linkv = GetAnalogSta();
    if(g_sAnalogValue.linkv != 6)
    {
        OUTPUTCLOSE();
        ChargingOver();
    }
}

void DisplayUsingInfo2(void)//12
{
    uint8_t *pData;
    uint16_t minute; 
    uint8_t dispbuf[10];
    pData = "充电电压:220.0V";
    Lcd_Printf(2,1,0,pData);
    pData = "充电电流:10.00A";
    Lcd_Printf(18,1,0,pData);

    pData = "up";
    Lcd_Printf(2,50,0,pData);
    
    if(CurTime.SEC != RtcSecondBck)
    {
       ChargingTime++;
       RtcSecondBck = CurTime.SEC;
    }
    
    g_sAnalogValue.linkv = GetAnalogSta();
    if(g_sAnalogValue.linkv != 6 )
    {
        OUTPUTCLOSE();
        ChargingOver();
    }
}

void DisplaySettlementInfo(void)//13
{
    uint8_t *pData;
    
    pData = "用户:00000001";
    Lcd_Printf(2,1,0,pData);
    pData = "充电电量:10.00Kwh";
    Lcd_Printf(18,1,0,pData);
    pData = "充电金额:6.00元";
    Lcd_Printf(34,1,0,pData);
    
    pData = "确定";
    Lcd_Printf(50,50,0,pData);
}

//==============================================================================
//==============================================================================
void DisplayScreen(void)
{
    uint8_t *pData,tmp;
    /*
    uint32_t ScreenRefreshTick;
    if(SysTickCnt - ScreenRefreshTick < DISPFRESHDLY)return;
    ScreenRefreshTick = SysTickCnt;
   */
    if(F_display_do)
    {
        F_display_do = 0;
        Lcd_Clr();
    }
    
    if((Disp_TCB_Cur->select&0x80)!= 0) {
        tmp = (Disp_TCB_Cur->select2&0x3f)*16+18;
        if(tmp <= 48) {
            pData = " ";
            Lcd_Printf(tmp,3,0,pData);
        }

        tmp = (Disp_TCB_Cur->select&0x3f)*16+18;
        if(tmp <= 48){
            pData = " ";
            Lcd_Printf(48,3,0,pData);
            pData = ">";
            Lcd_Printf(tmp,3,0,pData);
        }
        else {
            pData = ">";
            Lcd_Printf(48,3,0,pData);
        }
    }
    
    if((Disp_TCB_Cur->cursor&0x80)!= 0) {
        if((Disp_TCB_Cur->cursor2&0x0f) > (Disp_TCB_Cur->cursor&0x0f)){
            tmp = (Disp_TCB_Cur->cursor2&0x0f)*3 + CURSORPOS; 
            pData = " ";
            Lcd_Printf(34,tmp,0,pData);
        }

        tmp = (Disp_TCB_Cur->cursor&0x0f)*3 + CURSORPOS;
        Disp_TCB_Cur->flashdcnt++;
        if(Disp_TCB_Cur->flashdcnt > 500/DISPFRESHDLY
           || Disp_TCB_Cur->cursor+1 == Disp_TCB_Cur->cursor2) 
        {
            Disp_TCB_Cur->flashdcnt = 0;
            if((Disp_TCB_Cur->cursor&0x40)!= 0){
                Disp_TCB_Cur->cursor &=0xbf;    
                pData = "_";
            }
            else {
                Disp_TCB_Cur->cursor |=0x40;    
                pData = " ";
            }
            Lcd_Printf(34,tmp,0,pData);
            Disp_TCB_Cur->cursor2 = Disp_TCB_Cur->cursor+2;
        }
    }
    
    Disp_TCB_Cur->pDispFunc();
}
//==============================================================================
//==============================================================================

void DisplayMain(void)
{
    static uint32_t dispticks;
    static uint8_t dispno = 0;
    
    if(SysTickCnt < dispticks + 200)
    {
        return;
    }
    dispticks = SysTickCnt;
    
    DisplayScreen();
    KeyBoard_Deal();
}

// 当用户验证正确时，调用此函数
void LoginCheckSuccess(void)
{
    F_display_do = TRUE;
    Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[0];
}
// 当用户验证错误时，调用此函数
void LoginCheckError(void)
{
    F_display_do = TRUE;
    Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[1];
}
// 当用户验证错误次数超3次时，调用此函数，并锁定用户
void LoginUserLock(void)
{   
    F_display_do = TRUE;
    Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[2];
}
// 当检测到连接器CP从6v变为9V或12V时，调用此函数（已调用处理）
void ChargingOver(void)
{   
    F_display_do = TRUE;
    Disp_TCB_Cur = Disp_TCB_Cur->TcbNext[0];
}
// 当用户验证超时，调用此函数（已调用处理）
void LoginCheckOverTime(void)
{
    F_display_do = TRUE;
    Disp_TCB_Cur = Disp_TCB_Cur->TcbPrev;
}
