
/**********************************************************
德和新能源科技有限公司
File name:  hwtst.c	              // 文件名
Author:     韩忠华                   // 作者
Version:    v1.00                   // 版本信息
Date:       2015-2-27              // 完成日期
Description:  	              // 
Others:		              // 
Function List:	              //
History:                        //
**********************************************************/


#include "board.h"
#include "Dio.h"
#include "uart.h"
#include "globals.h"
#include "GPRS.h"
#include "string.h"

extern uint8_t GetAnalogSta(void);
uint32_t ConnectorCheckCnt;             //连接器状态检测计数器
uint8_t ConnectorStatus;             //连接器状态
uint8_t ConnectorCnt;             //连接器状态5次循环计数
uint8_t linkv;                  //连接器检测电压
extern PARACONFIG g_sParaConfig;
extern uint8_t Dev_Status;
uint32_t QRCodeCnt;         //二维码扫描请求计时
uint8_t LoginPermission = 0;       //登录许可
uint32_t ChargingRequestCnt;         //充电请求计时
uint8_t ChargingPermission;       //充电许可
uint8_t StopRequest;            //后台停止请求
uint32_t StopRequestCnt;           //停止请求计时
uint8_t StopPermission;       //停止许可
extern uint32_t Cur_0_BeginTime,Cur_0_Time;
extern uint32_t begin_CurKwh, end_CurKwh;
extern uint32_t CurKwh;
extern uint8_t EStopFlag,ParaError,MeterCommErr;
uint8_t test;
void Charging_main( void )
{
  //方式2，200ms检测连接器状态，连续5次结果相同则作为有效值
  
  if(( SysTickCnt - ConnectorCheckCnt ) > 200 )
  {
    ConnectorCheckCnt = SysTickCnt;
    if (ConnectorStatus != GetAnalogSta())
    {
      ConnectorStatus = GetAnalogSta();
      ConnectorCnt = 1;
    }
    else 
    {
      ConnectorCnt += 1;
      if (ConnectorCnt > 5)
      {
        linkv = ConnectorStatus;
        ConnectorCnt = 6;
      }
    }
  }     
 
  if ((LoginPermission == 1))   //接受到平台反馈用户信息发起充电请求,当前平台3秒发一次身份确认
  {
    LoginPermission = 0;
    if (Dev_Status==0)
    {
      if( linkv == 6 )            
      {
        ChargingPermission = 0;
        begin_CurKwh = CurKwh; 
        Start();
      } 
    }
  }
  
  if(ChargingPermission == 1)                 //获得后台充电许可后启动充电
  {
    record_clear();
    ChargingPermission = 0;
    //    SetOutput(5);           //启动
    OUTPUTOPEN ;
    SetOutput(2);           //运行指示灯
    Dev_Status = 1;                 //设备状态为充电
    Cur_0_BeginTime = SysTickCnt;
  }
  
  if (StopRequest == 1)        //接受到平台反馈用户信息发起充电请求,当前平台5秒发一次身份确认
  {
    StopRequest = 0;
    end_CurKwh = CurKwh;
    Dev_Status = 0;
    //       ClrOutput(1);           //停止输出
    OUTPUTCLOSE;
    ClrOutput(2);           //关闭运行指示灯
    Stop();
    //      StopRequestCnt = SysTickCnt;
  }
  
  if(StopPermission == 1)
  {
    StopPermission = 0;
  }
  
  
  if( GetInputStatus(1)&&Dev_Status )
  {
    end_CurKwh = CurKwh;
    Stop();
    Dev_Status = 0;
  }
  if (GetInputStatus(1))
  {
    ClrOutput(2);
    EStopFlag = 1;
    OUTPUTCLOSE;
  }
  else
  {
    EStopFlag = 0;
  }   
  
  if (MeterCommErr||ParaError||EStopFlag)
  {
     SetOutput(3);
  }
  else
  {
     ClrOutput(3);
  }
}





