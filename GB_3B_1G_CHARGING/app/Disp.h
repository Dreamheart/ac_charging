#ifndef __DISP_H__
#define __DISP_H__
//==============================================================================
#define KEY_0           0x00
#define KEY_1           0x01
#define KEY_2           0x02
#define KEY_3           0x03
#define KEY_4           0x04
#define KEY_5           0x05
#define KEY_6           0x06
#define KEY_7           0x07
#define KEY_8           0x08
#define KEY_9           0x09
#define KEY_DOT         0x0a
#define KEY_CANCEL      0x0b
#define KEY_ENTER       0x0c
#define KEY_DOWN        0x0d
#define KEY_UP          0x0e
#define KEY_ESC         0x0f
#define KEY_CFG         0x10
#define LOGINOK         0xa0
#define LOGINFAIL       0xa1
#define LOGINOVER       0xa2

#define SCREENWELCOME          (0x00)
#define SCREENPARAMETER        (SCREENWELCOME+1)
#define SCREENLOGINMODE        (SCREENPARAMETER+1)
#define SCREENUSERLOGIN1       (SCREENLOGINMODE+1)
#define SCREENUSERLOGIN2       (SCREENUSERLOGIN1+1)
#define SCREENLOGINCHECK       (SCREENUSERLOGIN2+1)
#define SCREENLOGINCHECKERROR  (SCREENLOGINCHECK+1)
#define SCREENLOGINLOCK        (SCREENLOGINCHECKERROR+1)
#define SCREENQRLOGIN          (SCREENLOGINLOCK+1)
#define SCREENUSERINFO         (SCREENQRLOGIN+1)
#define SCREENLINKINFO         (SCREENUSERINFO+1)
#define SCREENUSINGINFO1       (SCREENLINKINFO+1)
#define SCREENUSINGINFO2       (SCREENUSINGINFO1+1)
#define SCREENSETTLEMENTINFO   (SCREENUSINGINFO2+1)

#define SRCNNUM         14                                    //total srcn number
#define CURSORPOS       12                                     //cursor position

#define DISPFRESHDLY    0xc8

#define OUTPUTOPEN()      SetOutput(RELAY1)
#define OUTPUTCLOSE()     ClrOutput(RELAY1)   
#define RUNLEDOPEN()      SetOutput(RELAY2)   
#define RUNLEDCLOSE()     ClrOutput(RELAY2)
#define ERRORLEDOPEN()    SetOutput(RELAY3)
#define ERRORLEDCLOSE()   ClrOutput(RELAY3)



typedef struct Oled_TCB{
    
    uint8_t select;                                              // 菜单选择位置 
    uint8_t cursor;                                                  // 光标位置
    uint8_t count_max;                                         // 选择位置最大值
    uint8_t cursor_max;                                        // 光标位置最大值
    void (*pDispFunc)(void);                                   // 指向显示的指针
    struct Oled_TCB *TcbNext[4];                                //  显示后级目录
    struct Oled_TCB *TcbPrev;                                   //  显示前级目录
    uint8_t dotpos;                                                // 小数点位置
    uint8_t select2;                                         // 菜单选择位置备份
    uint8_t cursor2;                                             // 光标位置备份
    uint8_t flashdcnt;                                                 // 未用到
}DISPTCB;

typedef struct DispConfig{
    uint8_t select;
    uint8_t cursor;
    uint8_t count_max;
    uint8_t cursor_max;
    void (*pDispFunc)(void);
}DISPCONFig;

extern void Disp_TCB_Init(void);
extern uint8_t KeyBoard_Get(void);
extern void DisplayMain(void);

extern void DisplayWelcome(void);//0
extern void DisplayParameter(void);//1
extern void DisplayLoginMode(void);//2
extern void DisplayUserLogin1(void);//3
extern void DisplayUserLogin2(void);//4
extern void DisplayLoginCheck(void);//5
extern void DisplayLoginCheckError(void);//6
extern void DisplayLoginLock(void);//7
extern void DisplayQRLogin(void);//8
extern void DisplayUserInfo(void);//9
extern void DisplayLinkInfo(void);//10
extern void DisplayUsingInfo1(void);//11
extern void DisplayUsingInfo2(void);//12
extern void DisplaySettlementInfo(void);//13

extern void LoginCheckSuccess(void);
extern void LoginCheckError(void);
extern void LoginUserLock(void);
extern void ChargingOver(void);
extern void LoginCheckOverTime(void);
#endif