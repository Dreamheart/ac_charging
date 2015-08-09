
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

#include "drv_inc.h"
#include "Bsp_inc.h"
#include "app_inc.h"

uint32_t ConnectorCheckCnt;             //连接器状态检测计数器
uint8_t ConnectorStatus;             //连接器状态
uint8_t ConnectorCnt;             //连接器状态5次循环计数
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

void Charging_main( void )
{
 //   if (Dev_Status == 1)
 //   {
        //针对连续检测时发现的连接器状态偶尔变化情况按以下两种方案进行测试
        //方式1，1s检测连接器状态
        /*
        if(( SysTickCnt - ConnectorCheckCnt ) > 1000 )
        {
            g_sAnalogValue.linkv = GetAnalogSta();
            ConnectorCheckCnt = SysTickCnt;
        } 
        */  
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
                    g_sAnalogValue.linkv = ConnectorStatus;
                    ConnectorCnt = 6;
                }
            }
        }     
   // }
/*    
    if (Dev_Status != 1)
    {
        if(( SysTickCnt - QRCodeCnt ) > 17000 )            //15秒发送一次二维码扫描查询
        {
            QR_code_Verification();
            QRCodeCnt = SysTickCnt;
        } 
    }
*/    
    if (LoginPermission == 1)   //接受到平台反馈用户信息发起充电请求,当前平台3秒发一次身份确认
    {
        LoginPermission = 0;
        if( g_sAnalogValue.linkv == 6 )            
        {
            ChargingPermission = 0;
            begin_CurKwh = CurKwh; 
            Start();
        } 
    }
        
    if(ChargingPermission == 1)                 //获得后台充电许可后启动充电
    {
        record_clear();
        ChargingPermission = 0;
        OUTPUTOPEN();               //接受到平台启动反馈的订单号正式启动充电
        Dev_Status = 1;                 //设备状态为充电
        RUNLEDOPEN();
        Cur_0_BeginTime = SysTickCnt;
    }
    
    if (StopRequest == 1)        //接受到平台反馈用户信息发起充电请求,当前平台5秒发一次身份确认
    {
        StopRequest = 0;
        end_CurKwh = CurKwh;
        Dev_Status = 0;
        OUTPUTCLOSE();//停止充电
        RUNLEDCLOSE();
        Stop();
        StopRequestCnt = SysTickCnt;
    }
    
    if(StopPermission == 1)
    {
        StopPermission = 0;
    }
}





