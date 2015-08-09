
#include "drv_inc.h"
#include "Bsp_inc.h"
#include "app_inc.h"
#include <string.h>

uint8_t version[2];
extern user_info t_user_info, r_user_info;
extern PARACONFIG g_sParaConfig;
extern uint8_t Dev_Status; 
extern uint8_t back_type;              //反馈类型
extern uint8_t return_sign;              //反馈标志0-平台未反馈账户，1-平台已经反馈账户信息
extern uint32_t ChargingTime;
unsigned long ChargingBeginTime = 0;
extern uint32_t begin_CurKwh, end_CurKwh;
extern uint32_t CurKwh;
extern uint8_t server_stop,server_begin;
uint32_t charge_CurKwh;
extern uint16_t output_vol, output_cur;
uint8_t buff[256];
uint8_t RelaySta1;      //4851接受状态指示灯，交替开关

void peer_init ( void )
{
    version[0] = 1 ;//1.00
    version[1] = 0 ;
}

/*************************************************************
Function: void Ready ( void )
Description: 充电准备函数，清除之前充电数据
Calls:
Called By:
Input:       tick
Output:      无
Return:      无
Others:      无
*************************************************************/
void Ready ( void )
{
    memset(&r_user_info.tel_number,0,12);
    r_user_info.Balance = 0;
    memset(&r_user_info.password,0,6);
    r_user_info.pay = 0;
    memset(&r_user_info.card_number,0,16);
    r_user_info.card_status = 0;
    
    memset(&t_user_info.tel_number,0,12);
    t_user_info.Balance = 0;
    memset(&t_user_info.password,0,6);
    t_user_info.pay = 0;
    memset(&t_user_info.card_number,0,16);
    t_user_info.card_status = 0;
    
    server_stop = 0;
    server_begin = 0;
    return_sign = 0;
    back_type = 0;
    begin_CurKwh = 0;
    end_CurKwh = 0;
//    output_vol = 0;
    output_cur = 0;
    ChargingTime = 0;
}

void peer_main ( void )
{
    uint16_t result,temp = 0;
    uint8_t crcl, crch, i, len;
    uint8_t tmp;
    //uint16_t rnum;//记录条数
    uint16_t  addr = 0;

    if ( GetRs485Ch1Sta() == 1 ) //帧接收结束标记
    {
      //接收状态指示灯
        RelaySta1 = ~ RelaySta1 ;
        tmp = RelaySta1;
        if ( tmp & 0x1 )
        {
            SetOutput ( LED1 );
        }
        else
        {
            ClrOutput ( LED1 );
        }
      //
        len = RS485Rx1.Len ;
        RS485Rx1.Flag = 0 ;
        RS485Rx1.Len = 0 ;
        RS485Rx1.Idx = 0 ;
        if (len<8)
        {
            return;
        }
   //     if ( RS485Rx1.Buff[0] == g_sParaConfig.id[0] )//先判断是否本机地址
        if ( RS485Rx1.Buff[0] == 1)//先判断是否本机地址
        {
            CRC_Init ( CRC_POLY_CRC16 );
            //  result 高8位对应MODBUS CRC L ，低8位对应MODBUS CRC H
            temp = CRC_CalcBlockChecksum ( RS485Rx1.Buff, len - 2, CRC_WR_16BIT ); //计算CRC
            crcl = temp ;
            crch = temp >> 8 ;

            if ( ( crcl == RS485Rx1.Buff[len-2] ) && ( crch == RS485Rx1.Buff[len-1] ) ) //比较CRC
            {
                switch ( RS485Rx1.Buff[1] ) //功能码
                {
                    case 0x03:// 从主站接收到的：地址 03 地址高 地址低 寄存器数量高 寄存器数量低 CRCL CRCH
                        // 发送给主站的：地址 03 字节长度 数据1 数据2 数据N CRCL CRCH
                    {
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//地址
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//03功能码

                        temp = RS485Rx1.Buff[4] ;
                        temp <<= 8 ;
                        temp |= RS485Rx1.Buff[5] ;//temp中为主站要求的长度
                        temp *= 2 ;              //1个寄存器为2字节，所以长度X2
                        RS485Tx1.Buff[2] = temp ;//长度
                        len = temp ;
                        addr = * ( ( uint16_t * ) ( &RS485Rx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ;
                        
                        switch(addr)
                        {
                            case 0x14:
                            {
                                g_sAnalogValue.linkv = GetAnalogSta();
                                if (g_sAnalogValue.linkv == 6)
                                {
                                   RS485Tx1.Buff[3] = 1; 
                                }
                                else
                                {
                                    RS485Tx1.Buff[3] = 0;
                                }
                             //   RS485Tx1.Buff[3] = 1;   //调试固定给1
                                if (return_sign == 0)
                                {
                                    RS485Tx1.Buff[4] = 0;
                                }  
                                else
                                {
                                    RS485Tx1.Buff[4] = back_type + 1;
                                }
                                for ( i = 0; i < 12; i++ )
                                {
                                    RS485Tx1.Buff[5 + i] = r_user_info.tel_number[0 + i] ;
                                }
                                RS485Tx1.Buff[17] = r_user_info.Balance >> 24 ;
                                RS485Tx1.Buff[18] = r_user_info.Balance >> 16 ;
                                RS485Tx1.Buff[19] = r_user_info.Balance >> 8 ;
                                RS485Tx1.Buff[20] = r_user_info.Balance ;
                                RS485Tx1.Buff[21] = 0;
                                RS485Tx1.Buff[22] = server_begin;
                                break;
                            }
                            case 0x28:
                            {
                                
                                RS485Tx1.Buff[3] = ChargingTime/3600;
                                RS485Tx1.Buff[4] = (ChargingTime%3600)/60;
                                RS485Tx1.Buff[5] = ChargingTime % 60;
                                RS485Tx1.Buff[6] = 0;
                                charge_CurKwh = CurKwh - begin_CurKwh;
                                RS485Tx1.Buff[7] = charge_CurKwh >> 8;
                                RS485Tx1.Buff[8] = charge_CurKwh;
                                RS485Tx1.Buff[9] = output_vol >> 8;
                                RS485Tx1.Buff[10] = output_vol;
                                RS485Tx1.Buff[11] = output_cur >> 8;
                                RS485Tx1.Buff[12] = output_cur;
                                RS485Tx1.Buff[13] = 0;
                                RS485Tx1.Buff[14] = server_stop;
                                break;
                            }
                            case 0x32:
                            {
                                RS485Tx1.Buff[3] = 0;
                                RS485Tx1.Buff[4] = return_sign;
                                RS485Tx1.Buff[5] = r_user_info.pay >> 24;
                                RS485Tx1.Buff[6] = r_user_info.pay >> 16;
                                RS485Tx1.Buff[7] = r_user_info.pay >> 8;
                                RS485Tx1.Buff[8] = r_user_info.pay;
                                RS485Tx1.Buff[9] = r_user_info.Balance >> 24;
                                RS485Tx1.Buff[10] = r_user_info.Balance >> 16;
                                RS485Tx1.Buff[11] = r_user_info.Balance >> 8;
                                RS485Tx1.Buff[12] = r_user_info.Balance;
                                break;
                            }
                            case 0x64:
                            {
                                LoadAllParameters();
                                RS485Tx1.Buff[3] = g_sParaConfig.id[0];
                                RS485Tx1.Buff[4] = g_sParaConfig.id[1];
                                RS485Tx1.Buff[5] = g_sParaConfig.id[2];
                                RS485Tx1.Buff[6] = g_sParaConfig.id[3];
                                break;
                            }
                        }
                        

                        temp += 3 ;//数据区长度 + 帧头
                        CRC_Init ( CRC_POLY_CRC16 );
                        result = CRC_CalcBlockChecksum ( RS485Tx1.Buff, temp, CRC_WR_16BIT );
                        crcl = result ;
                        crch = result >> 8 ;
                        RS485Tx1.Buff[temp] = crcl;
                        RS485Tx1.Buff[temp+1] = crch;
                        RS485Tx1.Len = temp + 2 ; //给发送长度
                        RS485Ch1SendMsg();
                        break;
                    }
                    case 0x10:// 从主站接收到的：地址 0x10 地址高 地址低 寄存器数量高 寄存器数量低 字节数 数据1  数据2 数据N CRCL CRCH
                    {
                    
                        // 发送给主站的：地址 0x10 地址高 地址低 寄存器数量高 寄存器数量低 CRCL CRCH
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//地址
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//01功能码
                        RS485Tx1.Buff[2] = RS485Rx1.Buff[2] ;//
                        RS485Tx1.Buff[3] = RS485Rx1.Buff[3] ;//
                        RS485Tx1.Buff[4] = RS485Rx1.Buff[4] ;//
                        RS485Tx1.Buff[5] = RS485Rx1.Buff[5] ;//

                        CRC_Init ( CRC_POLY_CRC16 );
                        result = CRC_CalcBlockChecksum ( RS485Tx1.Buff, 6, CRC_WR_16BIT );
                        crcl = result ;
                        crch = result >> 8 ;
                        RS485Tx1.Buff[6] = crcl;
                        RS485Tx1.Buff[7] = crch;

                        RS485Tx1.Len = 8 ;//给发送长度
                        RS485Ch1SendMsg(); //先发送回应给主站，避免数据处理时间过长导致通信错误

                        temp = RS485Tx1.Buff[4] ;
                        temp <<= 8 ;
                        temp |= RS485Tx1.Buff[5] ;//temp中为主站要求的长度
                        temp *= 2 ;             //1个寄存器为2字节，所以长度X2
                        RS485Rx1.Buff[2] = temp;//长度
                        len = temp ;//取长度
                        addr = * ( ( uint16_t * ) ( &RS485Tx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ; //取地址
                        i = 7;
                        
                        switch(addr)
                        {
                            case 0x00:
                            {
                                Ready();
                                record_clear();
                                break;
                            }
                            case 0x01:
                            {   
                                for ( i = 0; i < 12; i++ )
                                {
                                    t_user_info.tel_number[0 + i] = RS485Rx1.Buff[7 + i] ;
                                }
                                for ( i = 0; i < 6; i++ )
                                {
                                    t_user_info.password[0 + i] = RS485Rx1.Buff[19 + i] ;
                                }
                                return_sign = 0;
                                Account_Verification();
                                
                                break;
                            }
                            case 0x0f:
                            {
                             //   return_sign = 0;
                                QR_code_Verification();
                                break;
                            }
                            case 0x23:
                            {
                                if (RS485Rx1.Buff[8] == 1)
                                {
                                    begin_CurKwh = CurKwh;      
                                    Start();
                                }
                                else if (RS485Rx1.Buff[8] == 2)
                                {
                                    end_CurKwh = CurKwh;
                                    OUTPUTCLOSE(); 
                                    RUNLEDCLOSE();
                                    Dev_Status = 0;
                                    return_sign = 0;
                                    Stop();
                                } 
                                break;
                            }
                            case 0x64:
                            {
                                g_sParaConfig.id[0] = RS485Rx1.Buff[7];
                                g_sParaConfig.id[1] = RS485Rx1.Buff[8];
                                g_sParaConfig.id[2] = RS485Rx1.Buff[9];
                                g_sParaConfig.id[3] = RS485Rx1.Buff[10];
                                SaveParameter();
                                break;
                            }
                        }

                    }

                }
            }
            else
            {
                return;
            }
        }
    }
}





