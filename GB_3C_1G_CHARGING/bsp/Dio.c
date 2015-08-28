/**********************************************************
  江苏德和新能源科技有限公司
  File name:  Dio.c	              // 文件名
  Author:     王辉                    // 作者
  Version:    v1.00                   // 版本信息
  Date:       2012-8-15               // 完成日期
  Description:  	              // 数字输入、数字输出的驱动程序
  Others:		              // 无
  Function List:	              // void DioInit(void) 初始化端口配置
                                         uint8_t GetInputStatus(uint8_t num)  获取输入端口状态
                                         uint8_t AddrCheck(void) 读地址开关，返回地址
                                         void SetOutput(uint8_t num) 设置输出状态 num : 输出端口号 1-6  LED 10-12
                                         void ClrOutput(uint8_t num) 清除输出状态 num : 输出端口号 1-6  LED 10-12
  History:2013-05-31 采用二维数组替代原宏定义方式配置IO端口,更改端口更方便
**********************************************************/


#include "board.h"
#include "Dio.h"

#if CPU822
const uint8_t InputConfig[INPUT_NUM_MAX][2]=
{
    0,26,
    0,27,
    0,16,
    0,10
};


const uint8_t OutputConfig[OUTPUT_NUM_MAX][2]=
{
    0,1,
    0,15,
    0,24,
    0,25,
    0,6,
    0,7  
};

#else
const uint8_t InputConfig[INPUT_NUM_MAX][2]=
{
    0,27,
    0,16,
    0,10,
    0,11
};


const uint8_t OutputConfig[OUTPUT_NUM_MAX][2]=
{
    0,1,
    0,15,
    0,24,
    0,25,
    0,26,
    0,6,
    0,7  
};
#endif

#define RUNLED 6 //运行灯
#define WARLED 7 //报警灯
#define INSULATIONEN 9 //绝缘检测控制
#define FAN_CTRL 1 //风扇
#define OUTRELAY 2 //主接触器
#define DISCHARGE 3 //泄放接触器
#define BMSPOWERPORT 4 //辅助源+
#define BMSPOWERPORT2 10 //辅助源-
#define LOCKRELAY 5 //锁
#define UNLOCKRELAY 8 //开锁
#define WD_CONNECTED_PORT		(5)
#define WD_CONNECTED_PIN		(1)
#define WD_CONNECTED_MASK		(1 << WD_CONNECTED_PIN)

uint8_t InputSta[INPUT_NUM_MAX];
static uint8_t InputStaCur[INPUT_NUM_MAX];
static uint8_t InputStaPre[INPUT_NUM_MAX];
static uint8_t InputStaCnt[INPUT_NUM_MAX];

/*************************************************************
  Function: GetInputStatus(uint8_t num)
  Description: 获取输入端口状态
  Calls:       无
  Called By:   无
  Input:       num : 输入通道号，范围1-2
  Output:      无
  Return:      输入端口状态
  Others:      无
*************************************************************/
uint8_t GetInputStatus ( uint8_t num )
{
    if ( ( num > 0 ) && ( num <= INPUT_NUM_MAX ) )
    {
        return InputSta[num-1];
    }
    else
    {
        return OFF ;
    }
}
    uint32_t PortValue;
/*************************************************************
  Function: void InputCheck(void)
  Description: 读输入管脚状态，消抖处理后，将状态刷新到数组
  Calls:       无
  Called By:   SysTick_Handler
  Input:       无
  Output:      InputSta[]
  Return:      无
  Others:      无
*************************************************************/
void InputCheck ( void )
{

    uint8_t i;
    for (i=0;i<INPUT_NUM_MAX;i++)
    {
        PortValue = Chip_GPIO_GetPortValue(LPC_GPIO_PORT, 0);
        if (PortValue & (1<<InputConfig[i][1]))
        {
            InputStaCur[i] = OFF ;
        }
        else
        {
            InputStaCur[i] = ON ;
        }        
    }

    for ( i = 0; i < INPUT_NUM_MAX; i++ )
    {
        if ( InputStaCur[i] == InputStaPre[i] )
        {
            InputStaCnt[i]++;

            if ( InputStaCnt[i] > 10 )
            {
                InputSta[i] = InputStaCur[i] ;
            }
        }
        else
        {
            InputStaCnt[i] = 0 ;
        }

        InputStaPre[i] = InputStaCur[i] ;
    }
}



/*************************************************************
  Function: void SetOutput(uint8_t num)
  Description: 设置输出端口状态
  Calls:       GPIO_OutputValue
  Called By:   SysTick_Handler、
  Input:       num : 输出端口号 1-9  LED 10-12
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void SetOutput ( uint8_t num )
{
     if ( ( num > 0 ) && ( num <= OUTPUT_NUM_MAX ) )
     {
        Chip_GPIO_SetPinState ( LPC_GPIO_PORT, OutputConfig[num-1][0] ,OutputConfig[num-1][1], ON );
     }
}

/*************************************************************
  Function: void ClrOutput(uint8_t num)
  Description: 清除输出端口状态
  Calls:       GPIO_OutputValue
  Called By:   SysTick_Handler、
  Input:       num : 输出端口号 1-6
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void ClrOutput ( uint8_t num )
{
     if ( ( num > 0 ) && ( num <= OUTPUT_NUM_MAX ) )
     {
        Chip_GPIO_SetPinState ( LPC_GPIO_PORT, OutputConfig[num-1][0] ,OutputConfig[num-1][1], OFF );
     }
}

/*************************************************************
  Function: void OutEnable(void)
  Description: 吸合输出接触器
  Calls:       SetOutput()
  Called By:
  Input:       sts--指令
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void OutEnable(void)
{
    SetOutput(OUTRELAY);
    ClrOutput(INSULATIONEN);//关闭绝缘模块
}
/*************************************************************
  Function: void OutDisable(void)
  Description: 断开输出接触器
  Calls:       ClrOutput()
  Called By:
  Input:       
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void OutDisable(void)
{
    ClrOutput(OUTRELAY);
    SetOutput(INSULATIONEN);//绝缘允许
}

