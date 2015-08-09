
/**********************************************************
德和新能源科技有限公司
File name:  hwtst.c	              // 文件名
Author:     韩忠华                   // 作者
Version:    v1.00                   // 版本信息
Date:       2015-1-13              // 完成日期
Description:  	              // 使用232串口与GPRS模块通信，间接实现与综合平台的通信
Others:		              // 
Function List:	              //
History:                        //
**********************************************************/

#include "drv_inc.h"
#include "Bsp_inc.h"
#include "app_inc.h"


extern PARACONFIG g_sParaConfig;
extern UART_BUFF RS232Rx, RS232Tx;
uint32_t begin_CurKwh, end_CurKwh;
extern uint32_t CurKwh;
extern uint8_t SOC;
uint8_t server_stop,server_begin;
uint16_t output_vol, output_cur;
user_info t_user_info, r_user_info;
uint8_t Dev_Status = 0, Dev_Error;    //设备状态，设备故障
uint32_t HeartbeatCnt;  //心跳计时
uint32_t InfoCnt;  //数据上传计时
uint32_t communication_delay;  //通信超时
uint8_t communication_state; //通信状态
uint8_t sys_year,sys_month,sys_day,sys_hour,sys_minute,sys_cecond; //通信状态
uint8_t back_type;               //反馈类型
uint8_t return_sign;              //反馈标志0-平台未反馈账户，1-平台已经反馈账户信息
uint8_t Order_number[4];           //订单号
extern uint32_t ChargingTime;           //充电时长，单位秒s
extern uint32_t ChargingBeginTime;
extern uint32_t Cur_0_BeginTime,Cur_0_Time;
extern uint8_t MeterCommErr;
extern uint8_t LoginPermission;       //登录许可
extern uint8_t ChargingPermission;       //充电许可
extern uint8_t StopRequest;            //后台停止请求
extern uint8_t StopPermission;       //停止许可
uint8_t RelaySta232;      //232接受状态指示灯，交替开关

/*************************************************************
Function: uint32_t Heartbeat ( void )
Description: 心跳
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void Heartbeat ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = Heartbeat_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_Heartbeat ;        //
    RS232Tx.Buff[10] = Dev_Status ;        //
    RS232Tx.Buff[11] = Dev_Error ;        //
    tmp = 0; 
    for ( i = 0; i < Len_Heartbeat; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_Heartbeat + 10] = tmp ;        //
    RS232Tx.Len = Len_Heartbeat + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t Account_Verification ( void )
Description: 身份验证--手机号和密码
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void Account_Verification ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = Account_Verification_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_Account_Verification ;        //
    memcpy(&RS232Tx.Buff[10],t_user_info.tel_number,12);
    memcpy(&RS232Tx.Buff[22],t_user_info.password,6);
    tmp = 0; 
    for ( i = 0; i < Len_Account_Verification; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_Account_Verification+10] = tmp ;        //
    RS232Tx.Len = Len_Account_Verification + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t QR_code_Verification ( void )
Description: 身份验证---二维码扫描
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void QR_code_Verification ( void )
{
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = QR_code_Verification_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_QR_code_Verification ;        //
    RS232Tx.Buff[10] = 0 ;        //
    RS232Tx.Len = 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t Card_Verification ( void )
Description: 身份验证---卡
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void Card_Verification ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = Card_Verification_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_Card_Verification ;        //
    memcpy(&RS232Tx.Buff[10],t_user_info.card_number,16);
    memcpy(&RS232Tx.Buff[26],t_user_info.password,6);
    tmp = 0; 
    for ( i = 0; i < Len_Card_Verification; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_Card_Verification+10] = tmp ;        //
    RS232Tx.Len = Len_Card_Verification + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t Start ( void )
Description: 启动充电请求
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void Start ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = Start_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_Start ;        //
    memcpy(&RS232Tx.Buff[10],r_user_info.tel_number,12);
    RS232Tx.Buff[22] = ( begin_CurKwh >> 24 ) & 0xff;
    RS232Tx.Buff[23] = ( begin_CurKwh >> 16 ) & 0xff;
    RS232Tx.Buff[24] = ( begin_CurKwh >> 8 ) & 0xff ;
    RS232Tx.Buff[25] = ( begin_CurKwh ) & 0xff ;
    tmp = 0; 
    for ( i = 0; i < Len_Start; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_Start+10] = tmp ;        //
    RS232Tx.Len = Len_Start + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t Stop ( void )
Description: 停止充电请求
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void Stop ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = Stop_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_Stop ;        //
    RS232Tx.Buff[10] = Order_number[0];
    RS232Tx.Buff[11] = Order_number[1];
    RS232Tx.Buff[12] = Order_number[2];
    RS232Tx.Buff[13] = Order_number[3];
    RS232Tx.Buff[14] = ( end_CurKwh >> 24 ) & 0xff;
    RS232Tx.Buff[15] = ( end_CurKwh >> 16 ) & 0xff;
    RS232Tx.Buff[16] = ( end_CurKwh >> 8 ) & 0xff ;
    RS232Tx.Buff[17] = ( end_CurKwh ) & 0xff ;
    tmp = 0; 
    for ( i = 0; i < Len_Stop; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_Stop+10] = tmp ;        //
    RS232Tx.Len = Len_Stop + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t AC_Info ( void )
Description: 交流信息上传
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void AC_Info ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = AC_Info_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_AC_Info ;        //
    RS232Tx.Buff[10] = Order_number[0];
    RS232Tx.Buff[11] = Order_number[1];
    RS232Tx.Buff[12] = Order_number[2];
    RS232Tx.Buff[13] = Order_number[3];
    RS232Tx.Buff[14] = ( output_vol >> 8) & 0xff;
    RS232Tx.Buff[15] = output_vol & 0xff;
    RS232Tx.Buff[16] = ( output_cur >> 8) & 0xff;
    RS232Tx.Buff[17] = output_cur & 0xff;
    RS232Tx.Buff[18] = ( CurKwh >> 24 ) & 0xff ;
    RS232Tx.Buff[19] = ( CurKwh >> 16 ) & 0xff ;
    RS232Tx.Buff[20] = ( CurKwh >> 8 ) & 0xff ; 
    RS232Tx.Buff[21] = ( CurKwh ) & 0xff ;
    tmp = 0; 
    for ( i = 0; i < Len_AC_Info; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_AC_Info+10] = tmp ;        //
    RS232Tx.Len = Len_AC_Info + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t DC_Info ( void )
Description: 直流信息上传
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void DC_Info ( void )
{
    uint8_t i, tmp;
  
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = DC_Info_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_DC_Info ;        //
    RS232Tx.Buff[10] = Order_number[0];
    RS232Tx.Buff[11] = Order_number[1];
    RS232Tx.Buff[12] = Order_number[2];
    RS232Tx.Buff[13] = Order_number[3];
    RS232Tx.Buff[14] = SOC;
    RS232Tx.Buff[15] = ( output_vol >> 8) & 0xff;
    RS232Tx.Buff[16] = output_vol & 0xff;
    RS232Tx.Buff[17] = ( output_cur >> 8) & 0xff;
    RS232Tx.Buff[18] = output_cur & 0xff;
    RS232Tx.Buff[19] = ( CurKwh >> 24 ) & 0xff ;
    RS232Tx.Buff[20] = ( CurKwh >> 16 ) & 0xff ;
    RS232Tx.Buff[21] = ( CurKwh >> 8 ) & 0xff ; 
    RS232Tx.Buff[22] = ( CurKwh ) & 0xff ;
    tmp = 0; 
    for ( i = 0; i < Len_DC_Info; i++ )
    {
        tmp += RS232Tx.Buff[10+i]; 
    }
    RS232Tx.Buff[Len_DC_Info+10] = tmp ;        //
    RS232Tx.Len = Len_DC_Info + 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t calibration_time ( void )
Description: 校时
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void calibration_time ( void )
{
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = calibration_time_Function_code ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_calibration_time ;        //
    RS232Tx.Buff[10] = 0 ;        //
    RS232Tx.Len = 11 ;//给发送长度
    RS232SendMsg(); 
}

/*************************************************************
Function: uint32_t record_clear ( void )
Description: 清除后台二维码扫描记录
Calls:       无
Called By:   
Input:       
Output:      无
Return:      
Others:      无
*************************************************************/
void record_clear ( void )
{
    RS232Tx.Buff[0] = 0x68 ;    //起始符
    RS232Tx.Buff[1] = 0x01 ;    //版本
    RS232Tx.Buff[2] = 0x0b ; //功能码
    RS232Tx.Buff[3] = g_sParaConfig.id[0] ;        //设备编号     
    RS232Tx.Buff[4] = g_sParaConfig.id[1] ;        //设备编号
    RS232Tx.Buff[5] = g_sParaConfig.id[2] ;        //设备编号
    RS232Tx.Buff[6] = g_sParaConfig.id[3] ;        //设备编号
    RS232Tx.Buff[7] = 0 ;        //包号
    RS232Tx.Buff[8] = 0 ;        //
    RS232Tx.Buff[9] = Len_calibration_time ;        //
    RS232Tx.Buff[10] = 0 ;        //
    RS232Tx.Len = 11 ;//给发送长度
    RS232SendMsg(); 
}

void GPRS_main ( void )
{
    uint8_t i, len;
    uint8_t sum_check;
    uint16_t data_len;
    uint8_t tmp;
    
    if(( SysTickCnt - HeartbeatCnt ) > 27000)
    {
        Heartbeat();
        HeartbeatCnt = SysTickCnt;
    } 
    
    if (Dev_Status == 1)
    {
        ChargingTime = (SysTickCnt - ChargingBeginTime)/1000 ;
        if(( SysTickCnt - InfoCnt ) > 17000)
        {
            AC_Info();
            InfoCnt = SysTickCnt;
        } 
        Cur_0_Time = (SysTickCnt - Cur_0_BeginTime)/1000;     
        if ((Cur_0_Time > 180)||                //180s检测不到有效电流则认为充电已经停止
            (g_sAnalogValue.linkv != 6)||   //充电过程中如果检测枪断开则停止充电
            (MeterCommErr == 1)||               //电表通信故障
            (output_cur > 3500)||               //电流软保护，当输出电流大于35A时停止充电
            (output_vol > 25300)||(output_vol < 18700))           //电压软保护，当输入电压超出253V或者低于187V时充电机停止输出
        {
            end_CurKwh = CurKwh;
            OUTPUTCLOSE(); 
            RUNLEDCLOSE();
            Dev_Status = 0;
            Stop();
        }
        
    }
    
    if ( GetRs232Sta() == 1 ) //帧接收结束标记
    {
        //接收状态指示灯
        RelaySta232 = ~ RelaySta232 ;
        tmp = RelaySta232;
        if ( tmp & 0x1 )
        {
            SetOutput ( LED1 );
        }
        else
        {
            ClrOutput ( LED1 );
        }
        //
        len = RS232Rx.Len ;
        RS232Rx.Flag = 0 ;
        RS232Rx.Len = 0;
        RS232Rx.Idx = 0 ;
        if( RS232Rx.Buff[0] == 0x68 )
        {
            data_len = ( RS232Rx.Buff[9] << 8 ) + RS232Rx.Buff[10];
            if( len == ( data_len + 12 ) )
            {
                sum_check = 0;
                for ( i = 0; i < data_len; i++ )
                {
                    sum_check += RS232Rx.Buff[11+i]; 
                }
                if( sum_check == RS232Rx.Buff[11+data_len])
                {
                    switch ( RS232Rx.Buff[2] )
                    {
                    case 0x01:
                        {
                            communication_state = 1;
                            communication_delay = 0;
                            break;
                        }
                     case 0x02:
                        {
                            communication_state = 1;
                            return_sign = 1;
                            back_type = RS232Rx.Buff[7];
                            memcpy(r_user_info.tel_number,&RS232Rx.Buff[11],12);
                            r_user_info.Balance = 0;
                            r_user_info.Balance = RS232Rx.Buff[23];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[24];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[25];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[26];
                            break;
                        }
                     case 0x03:
                        {
                            LoginPermission = 1;
                            memcpy(r_user_info.tel_number,&RS232Rx.Buff[11],12);
                            r_user_info.Balance = 0;
                            r_user_info.Balance = RS232Rx.Buff[23];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[24];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[25];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[26];
                            break;
                        }
                     case 0x04:
                        {
                            if (RS232Rx.Buff[7] == 0)
                            {
                                Order_number[0] = RS232Rx.Buff[11];
                                Order_number[1] = RS232Rx.Buff[12];
                                Order_number[2] = RS232Rx.Buff[13];
                                Order_number[3] = RS232Rx.Buff[14];
                                
                                ChargingPermission = 1;
                                
                                r_user_info.Balance = 0;
                                r_user_info.Balance = RS232Rx.Buff[15];
                                r_user_info.Balance <<= 8;
                                r_user_info.Balance |= RS232Rx.Buff[16];
                                r_user_info.Balance <<= 8;
                                r_user_info.Balance |= RS232Rx.Buff[17];
                                r_user_info.Balance <<= 8;
                                r_user_info.Balance |= RS232Rx.Buff[18];
                            }
                            break;
                        }
                     case 0x05:
                        {
                            StopPermission = 1;
                            r_user_info.Balance = 0;
                            r_user_info.Balance = RS232Rx.Buff[11];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[12];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[13];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[14];
                            r_user_info.pay = 0;
                            r_user_info.pay = RS232Rx.Buff[15];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[16];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[17];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[18];
                            break;
                        }
                     case 0x06:
                        {
                            r_user_info.pay = 0;
                            r_user_info.pay = RS232Rx.Buff[11];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[12];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[13];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[14];
                            communication_state = 1;
                            if(RS232Rx.Buff[7] == 0x03 )
                            {
                                OUTPUTCLOSE();//停止充电
                                RUNLEDCLOSE();
                                Stop();
                            }
                            break;
                        }
                     case 0x07:
                        {
                            r_user_info.pay = 0;
                            r_user_info.pay = RS232Rx.Buff[11];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[12];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[13];
                            r_user_info.pay <<= 8;
                            r_user_info.pay |= RS232Rx.Buff[14];
                            communication_state = 1;
                            if(RS232Rx.Buff[7] == 0x03 )
                            {
                                OUTPUTCLOSE();//停止充电
                                RUNLEDCLOSE();
                                Stop();
                            }
                            break;
                        }
                     case 0x08:
                        {
                            StopRequest = 1;
                            end_CurKwh = CurKwh;
                            Dev_Status = 0;
                            return_sign = 0;
                            communication_state = 1;
                            OUTPUTCLOSE();//停止充电
                            RUNLEDCLOSE();
                            server_stop = 1;
                            Stop();
                            break;
                        }
                     case 0x09:
                        {
                            communication_state = 1;
                            memcpy(t_user_info.card_number,&RS232Rx.Buff[11],16);
                            r_user_info.Balance = 0;
                            r_user_info.Balance = RS232Rx.Buff[27];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[28];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[29];
                            r_user_info.Balance <<= 8;
                            r_user_info.Balance |= RS232Rx.Buff[30];
                            r_user_info.card_status = RS232Rx.Buff[31];
                            break;
                        }
                     case 0x0a:
                        {
                            communication_state = 1;
                            sys_year = RS232Rx.Buff[11];
                            sys_month = RS232Rx.Buff[12];
                            sys_day = RS232Rx.Buff[13];
                            sys_hour = RS232Rx.Buff[14];
                            sys_minute = RS232Rx.Buff[15];
                            sys_cecond = RS232Rx.Buff[16];
                            break;
                        } 
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }
}



