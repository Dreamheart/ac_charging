

/**********************************************************
德和新能源科技有限公司
File name:  hwtst.c	              // 文件名
Author:     韩忠华                   // 作者
Version:    v1.00                   // 版本信息
Date:       2015-1-23              // 完成日期
Description:  	              // 使用232串口与GPRS模块通信，间接实现与综合平台的通信
Others:		              // 
Function List:	              //
History:                        //
**********************************************************/

#include "board.h"
#include "Dio.h"
#include "uart.h"
#include "eeprom.h"
#include "crc.h"
#include "TouchScreen.h"


//校验码问题
extern UART_BUFF RS485Rx1;
//uint8_t MeterData[4], MeterAddr[6];
//uint8_t MeterErr, MeterCommErr, MeterAddrFlag;
//uint8_t TScreenErr;
//uint8_t Position68;
//uint32_t RdMeterTicks, RdMeterTicks1, RdMeterTicks2;       //读电量，读电压，读电流时间周期
uint8_t TScreenCommErr;
uint32_t TScreenTicks;
uint16_t TScreenOnlineCnt;

uint8_t RelaySta3;      //4852接受状态指示灯，交替开关

//uint8_t Meter_counter;

//uint32_t CurKwh;
//uint32_t Cur_0_BeginTime,Cur_0_Time;

//extern uint16_t output_vol, output_cur;         //输出电压电流，
                                                //电流电表反馈三位小数，下文做特别处理，舍掉最后一位，系统中按照两位小数传输
                                                //电压电表反馈一位小数，下文做特别处理，小数第二位补0，系统中按照两位小数传输


/*************************************************************
Function: void MeterGetRs232()
Description: 从站应答帧
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void GetTScreenMsg ( void ) //从站应答帧
{

    uint8_t  len, i, j, CS;
    uint8_t tmp_[4];
    uint8_t temp;
    uint32_t tmp,tmp1,tmp2;
    
    uint16_t temp_;
    uint16_t *data;
    uint8_t crcl, crch;
    uint16_t addr = 0;
      
    if ( GetRs485Ch1Sta() == 1 ) //帧接收结束标记
    {
        //接收状态指示灯
        RelaySta3 = ~ RelaySta3 ;
        temp = RelaySta3;
        if ( temp & 0x1 )
        {
            SetOutput ( LED2 );
        }
        else
        {
            ClrOutput ( LED2 );
        }
        //
        TScreenOnlineCnt = 1;
        len = RS485Rx1.Len;
        RS485Rx1.Flag = 0 ;
        RS485Rx1.Len = 0 ;
        RS485Rx1.Idx = 0 ;        
  
///////////////////////////////////////////////////////////////////////////////////////////////////////    
///////////////////////////////////////////////////////////////////////////////////////////////////////  
        
                /*
                      
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//地址
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//01功能码
                        RS485Tx1.Buff[2] = RS485Rx1.Buff[2] ;//
                        RS485Tx1.Buff[3] = RS485Rx1.Buff[3] ;//
                        RS485Tx1.Buff[4] = RS485Rx1.Buff[4] ;//
                        RS485Tx1.Buff[5] = RS485Rx1.Buff[5] ;//
                        RS485Tx1.Len = 6;
                        RS485Ch1SendMsg();
                        */
   
        if ( RS485Rx1.Buff[0] == 1)//先判断是否本机地址
        {
    //        Chip_CRC_Init();
            //  result 高8位对应MODBUS CRC L ，低8位对应MODBUS CRC H
    //        data = ( uint16_t * )RS232Rx.Buff;
    //        temp_ = Chip_CRC_CRC16 ( data, len - 2 ); //计算CRC
    //        crcl = temp_ ;
    //        crch = temp_ >> 8 ;
          
          if ( RS485Rx1.Buff[1] == 0x10)
          {
            len = 13;
          }
          else
          {
            len = 8;
          }
          

          crch = CRC16(RS485Rx1.Buff,(len - 2),Hi);
          crcl = CRC16(RS485Rx1.Buff,(len - 2),Lo);
          

            if ( ( crcl == RS485Rx1.Buff[len-2] ) && ( crch == RS485Rx1.Buff[len-1] ) ) //比较CRC
            {
            
                switch ( RS485Rx1.Buff[1] ) //功能码
                {
                
                    case 0x00:
                      
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//地址
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//03功能码 
                        RS485Tx1.Buff[2] = RS485Rx1.Buff[2] ;//地址
                        RS485Tx1.Buff[3] = RS485Rx1.Buff[3] ;//03功能码      
                        RS485Tx1.Buff[4] = RS485Rx1.Buff[4] ;//地址
                        RS485Tx1.Buff[5] = RS485Rx1.Buff[5] ;//03功能码 
                        RS485Tx1.Buff[6] = RS485Rx1.Buff[6] ;//地址
                        RS485Tx1.Buff[7] = RS485Rx1.Buff[7] ;//03功能码     
                        
                        RS485Tx1.Len = 8 ;//给发送长度
                        
                        RS485Ch1SendMsg();
                      
                        break;
                        
                    case 0x03:// 从主站接收到的：地址 03 地址高 地址低 寄存器数量高 寄存器数量低 CRCL CRCH
                        // 发送给主站的：地址 03 字节长度 数据1 数据2 数据N CRCL CRCH
                   {
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//地址
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//03功能码

                  //      temp = RS232Rx.Buff[4] ;
                  //      temp <<= 8 ;
                  //      temp |= RS232Rx.Buff[5] ;//temp中为主站要求的长度
                  //      temp *= 2 ;              //1个寄存器为2字节，所以长度X2
                  //      RS232Tx.Buff[2] = temp ;//长度
                  //      len = temp ;
                        
                       RS485Tx1.Buff[2] = 4;
           //               len = 8 ;
                        addr = * ( ( uint16_t * ) ( &RS485Rx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ;
                        
                        
                    
                        switch(addr)
                        {
                            case 0x64:
                            {
                                LoadParameter();
                                RS485Tx1.Buff[3] = g_sParaConfig.id[0];
                                RS485Tx1.Buff[4] = g_sParaConfig.id[1];
                                RS485Tx1.Buff[5] = g_sParaConfig.id[2];
                                RS485Tx1.Buff[6] = g_sParaConfig.id[3];
                               
                                break;
                            }
                        }

                //        temp += 3 ;//数据区长度 + 帧头
                //        Chip_CRC_Init ();
                //        data = ( uint16_t * )RS232Rx.Buff;
                 //       temp_ = Chip_CRC_CRC16 ( data, temp );
                //        crcl = temp_ ;
                //        crch = temp_ >> 8 ;
                        
                        crch = CRC16(RS485Tx1.Buff,(7),Hi);
                        crcl = CRC16(RS485Tx1.Buff,(7),Lo);
          
                        RS485Tx1.Buff[7] = crcl;
                        RS485Tx1.Buff[8] = crch;
                        RS485Tx1.Len = 9; //给发送长度
    
                        
                      RS485Ch1SendMsg();
               //         RS232SendMsg(); 
  
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

              //          Chip_CRC_Init ();
              //          data = ( uint16_t * )RS232Rx.Buff;
              //          temp_ = Chip_CRC_CRC16 ( data, 6 );
              //          crcl = temp_ ;
               //         crch = temp_ >> 8 ;
                        
                         crch = CRC16(RS485Tx1.Buff,6,Hi);
                         crcl = CRC16(RS485Tx1.Buff,6,Lo);
                        
                        
                        RS485Tx1.Buff[6] = crcl;
                        RS485Tx1.Buff[7] = crch;

                        RS485Tx1.Len = 8 ;//给发送长度
    
                        
                        RS485Ch1SendMsg(); //先发送回应给主站，避免数据处理时间过长导致通信错误
                //        RS232SendMsg(); 
    

                //        temp = RS232Tx.Buff[4] ;
                //        temp <<= 8 ;
                //        temp |= RS232Tx.Buff[5] ;//temp中为主站要求的长度
                //        temp *= 2 ;             //1个寄存器为2字节，所以长度X2
                //        RS232Rx.Buff[2] = temp;//长度
                //        len = temp ;//取长度
                        
                        RS485Tx1.Buff[2] = 0;
                        
                        addr = * ( ( uint16_t * ) ( &RS485Tx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ; //取地址
                        i = 7;
                        
                        switch(addr)
                        {
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
                return;
               
            }
            else
            {
                return;
            }
                       
        }       
                                                         
                        
 /*       
       if ( RS485Rx1.Buff[0] == 1)//先判断是否本机地址
        {
            Chip_CRC_Init();
            //  result 高8位对应MODBUS CRC L ，低8位对应MODBUS CRC H
            data = ( uint16_t * )RS485Rx1.Buff;
            temp_ = Chip_CRC_CRC16 ( data, len - 2 ); //计算CRC
            crcl = temp_ ;
            crch = temp_ >> 8 ;
     //     crch = CRC16(RS485Rx1.Buff,(len-2),Hi);
     //     crcl = CRC16(RS485Rx1.Buff,(len-2),Lo);

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
                            case 0x64:
                            {
                                LoadParameter();
                                RS485Tx1.Buff[3] = g_sParaConfig.id[0];
                                RS485Tx1.Buff[4] = g_sParaConfig.id[1];
                                RS485Tx1.Buff[5] = g_sParaConfig.id[2];
                                RS485Tx1.Buff[6] = g_sParaConfig.id[3];
                                break;
                            }
                        }

                        temp += 3 ;//数据区长度 + 帧头
                        Chip_CRC_Init ();
                        data = ( uint16_t * )RS485Rx1.Buff;
                        temp_ = Chip_CRC_CRC16 ( data, temp );
                        crcl = temp_ ;
                        crch = temp_ >> 8 ;
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

                        Chip_CRC_Init ();
                        data = ( uint16_t * )RS485Rx1.Buff;
                        temp_ = Chip_CRC_CRC16 ( data, 6 );
                        crcl = temp_ ;
                        crch = temp_ >> 8 ;
                        
               //          crch = CRC16(RS485Rx1.Buff,6,Hi);
               //          crcl = CRC16(RS485Rx1.Buff,6,Lo);
                        
                        
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
        */
                        
                        
        }
        
  
///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////      
          

}


/*************************************************************
Function:     void MeterDeal ( void )
Description:  读电表，判断是否掉线
Calls:       无
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void TScreenDeal ( void )
{
    GetTScreenMsg();

    if ( TScreenOnlineCnt >= 30 )         //60次通信超时则认为触屏通信故障
    {
        TScreenCommErr = 1 ;
        SetOutput(3);                   //故障指示灯亮
    }
    else
    {
        TScreenCommErr = 0 ;
        ClrOutput(3);                   //故障指示灯灭
    }
    
    if (SysTickCnt > TScreenTicks + 1000 )//每1s读一次
    {
        TScreenOnlineCnt += 1;
        TScreenTicks = SysTickCnt;     
    }
    
    
    /*
    if (SysTickCnt > RdMeterTicks + 1000 )//每1s读一次
    {
        MeterOnlineCnt += 1;
        if (Meter_counter == 1)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCalc();//读电表电量
            Meter_counter += 1;
        }
        else if (Meter_counter == 2)
        {       
            RdMeterTicks = SysTickCnt ;
            MeterVol();//读电表电压
            Meter_counter += 1;
        }
        else if (Meter_counter == 3)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCur();//读电表电流
            Meter_counter += 1;
        }
        else
        {
            Meter_counter = 1;
        }
    }
    
    */
}











