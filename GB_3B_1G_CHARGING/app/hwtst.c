
#include "drv_inc.h"
#include "Bsp_inc.h"
#include "app_inc.h"


extern UART_BUFF RS232Rx, RS232Tx;
extern UART_BUFF RS485Rx1, RS485Tx1;
extern UART_BUFF RS485Rx2, RS485Tx2;
uint8_t HwTestFlag, RelayFlag;
uint16_t HwTestCnt, TestCanCnt, Test485Cnt;
uint8_t RelaySta;
uint8_t TestCan, CanStates;
uint8_t Test485, RS485States;
uint8_t TestOutCnt;
uint8_t GetHwTestSta ( void )
{
    return HwTestFlag;
}
void HwTestInit ( void )
{
    RelayFlag = 0x55 ;
}
void HwTestIO ( void )
{
    uint8_t i, tmp;
    static uint32_t HwTestTicks;
     
    if (SysTickCnt >HwTestTicks + 1000 )
    {
        HwTestTicks = SysTickCnt ;
        RelaySta = ~ RelaySta ;
        tmp = RelaySta;

        if ( tmp & 0x1 )
        {
            SetOutput ( LED2 );
        }
        else
        {
            ClrOutput ( LED2 );
        }

        for ( i = 0; i < OUTPUT_NUM_MAX; i++ )
        {
            ClrOutput ( i + RELAY1 );
        }
        if (TestOutCnt>=OUTPUT_NUM_MAX)
        {
            TestOutCnt = 0;
        }
        SetOutput ( TestOutCnt + RELAY1 );
        TestOutCnt++;
    }
}

void HwTest485 ( void )
{
    uint8_t tmp, i;
    tmp = Test485 % 4 ;

    switch ( tmp )
    {
        case 0:
        {
            RS485Tx1.Buff[0] = 0;
            RS485Tx1.Buff[1] = 1;
            RS485Tx1.Buff[2] = 2;
            RS485Tx1.Buff[3] = 3;
            RS485Tx1.Buff[4] = 4;
            RS485Tx1.Buff[5] = 5;
            RS485Tx1.Buff[6] = 6;
            RS485Tx1.Buff[7] = 7;
            RS485Tx1.Len = 8;
            RS485Ch1SendMsg();
            Test485 = 1;
            Test485Cnt = 0;
            break;
        }
        case 1:
        {
            Test485Cnt++;

            if ( GetRs485Ch2Sta() == 1 )
            {
                for ( i = 0; i < 8; i++ )
                {
                    if ( RS485Rx2.Buff[i] == RS485Tx1.Buff[i] )
                    {
                        ;
                    }
                    else
                    {
                        RS485States = 0x3 ;//错误
                        Test485 = 2;
                        break;
                    }
                }

                Test485 = 2;
            }

            if ( Test485Cnt > 1000 )
            {
                RS485States = 0x3 ;//错误
                Test485 = 2;
            }

            break;
        }
        case 2:
        {
            RS485Tx2.Buff[0] = 0;
            RS485Tx2.Buff[1] = 1;
            RS485Tx2.Buff[2] = 2;
            RS485Tx2.Buff[3] = 3;
            RS485Tx2.Buff[4] = 4;
            RS485Tx2.Buff[5] = 5;
            RS485Tx2.Buff[6] = 6;
            RS485Tx2.Buff[7] = 7;
            RS485Tx2.Len = 8;
            RS485Ch2SendMsg();
            Test485 = 3;
            Test485Cnt = 0;
            break;
        }
        case 3:
        {
            Test485Cnt++;

            if ( GetRs485Ch1Sta() == 1 )
            {
                for ( i = 0; i < 8; i++ )
                {
                    if ( RS485Rx1.Buff[i] == RS485Tx2.Buff[i] )
                    {
                        ;
                    }
                    else
                    {
                        RS485States = 0x3 ;//错误
                        Test485 = 0;
                        break;
                    }
                }

                Test485 = 0;
            }

            if ( Test485Cnt > 1000 )
            {
                RS485States = 0x3 ;//错误
                Test485 = 0;
            }

            break;
        }
    }
}

void HwTestCan ( void )
{
    uint8_t tmp, i;
    CAN_MSG_Type  TxMsg, RxMsg;
    tmp = TestCan % 4 ;

    switch ( tmp )
    {
        case 0:
        {
            TxMsg.format = EXT_ID_FORMAT;
            TxMsg.id = 0;
            TxMsg.len = 8;
            TxMsg.type = DATA_FRAME;
            TxMsg.dataA[0] = 1 ; //
            TxMsg.dataA[1] = 2 ; //
            TxMsg.dataA[2] = 3 ; //
            TxMsg.dataA[3] = 4 ; //
            TxMsg.dataB[0] = 5 ; // 运行
            TxMsg.dataB[1] = 6 ; //
            TxMsg.dataB[2] = 7 ; //
            TxMsg.dataB[3] = 8 ; //
            CAN_send ( CAN_1, &TxMsg );
            TestCan = 1;
            TestCanCnt = 0;
            break;
        }
        case 1:
        {
            TestCanCnt++;

            if ( CAN_receive ( CAN_2, &RxMsg ) == 0 )
            {
                for ( i = 0; i < 4; i++ )
                {
                    if ( ( RxMsg.dataA[i] == i + 1 ) && ( RxMsg.dataB[i] == i + 5 ) )
                    {
                        CanStates = 0;
                    }
                    else
                    {
                        CanStates = 0x3 ;//错误
                        TestCan = 2;
                        break;
                    }
                }

                TestCan = 2;
            }

            if ( TestCanCnt > 1000 )
            {
                CanStates = 0x3 ;//错误
                TestCan = 2;
            }

            break;
        }
        case 2:
        {
            TxMsg.format = EXT_ID_FORMAT;
            TxMsg.id = 0;
            TxMsg.len = 8;
            TxMsg.type = DATA_FRAME;
            TxMsg.dataA[0] = 1 ; //
            TxMsg.dataA[1] = 2 ; //
            TxMsg.dataA[2] = 3 ; //
            TxMsg.dataA[3] = 4 ; //
            TxMsg.dataB[0] = 5 ; // 运行
            TxMsg.dataB[1] = 6 ; //
            TxMsg.dataB[2] = 7 ; //
            TxMsg.dataB[3] = 8 ; //
            CAN_send ( CAN_2, &TxMsg );
            TestCan = 3;
            TestCanCnt = 0;
            break;
        }
        case 3:
        {
            TestCanCnt++;

            if ( CAN_receive ( CAN_1, &RxMsg ) == 0 )
            {
                for ( i = 0; i < 4; i++ )
                {
                    if ( ( RxMsg.dataA[i] == i + 1 ) && ( RxMsg.dataB[i] == i + 5 ) )
                    {
                        CanStates = 0;
                    }
                    else
                    {
                        CanStates = 0x3 ;//错误
                        TestCan = 0;
                        break;
                    }
                }

                TestCan = 0;
                break;
            }

            if ( TestCanCnt > 1000 )
            {
                CanStates = 0x3 ;//错误
                TestCan = 0;
            }

            break;
        }
    }

}

void HwTestAdc ( void )
{

}
uint8_t crcl, crch;
uint16_t result = 0;
void HwModbus ( void )
{
    uint16_t addr, temp ;
    uint8_t i, len;

    if ( GetRs232Sta() == 1 ) //帧接收结束标记
    {
        if ((RS232Rx.Buff[0]==0x02)&&(RS232Rx.Buff[1]==0x00)) //读卡器返回信息
        {
            return;
        }
        len = RS232Rx.Len ;
        RS232Rx.Flag = 0 ;
        RS232Rx.Len = 0;
        RS232Rx.Idx = 0 ;
        //if (RS232Rx.Buff[0] == SLAVE_ID)//先判断是否本机地址
        {
            CRC_Init ( CRC_POLY_CRC16 );
            //result 高8位对应MODBUS CRC L ，低8位对应MODBUS CRC H
            result = CRC_CalcBlockChecksum ( RS232Rx.Buff, len - 2, CRC_WR_16BIT ); //计算CRC
            crcl = result ;
            crch = result >> 8 ;

            if ( ( crcl == RS232Rx.Buff[len-2] ) && ( crch == RS232Rx.Buff[len-1] ) ) //比较CRC
            {
                ;
            }
            else
            {
                return;
            }
        }

        switch ( RS232Rx.Buff[1] ) //功能码
        {
            case 0x03:// 从主站接收到的：地址 03 地址高 地址低 寄存器数量高 寄存器数量低 CRCL CRCH
            {
                // 发送给主站的：地址 03 字节长度 数据1 数据2 数据N CRCL CRCH
                
                if (HwTestFlag==0)
                {
                    HwTestFlag = 1 ;
                    CAN_Init ( CAN_2, 250000 );
                    CAN_Init ( CAN_1, 250000 );
                    UartInit ( 0, 115200, UART_PARITY_NONE );
                    UartInit ( 1, 38400, UART_PARITY_NONE );
                    UartInit ( 2, 38400, UART_PARITY_EVEN );                    
                }
                RS232Tx.Buff[0] = RS232Rx.Buff[0] ;//地址
                RS232Tx.Buff[1] = RS232Rx.Buff[1] ;//03功能码

                temp = RS232Rx.Buff[4] ;
                temp <<= 8 ;
                temp |= RS232Rx.Buff[5] ;//temp中为主站要求的长度
                temp *= 2 ;              //1个寄存器为2字节，所以长度X2
                RS232Tx.Buff[2] = temp ;//长度
                len = temp ;
                i = 3 ;//返回数据起始地址
                temp = * ( ( uint16_t * ) ( &RS232Rx.Buff[2] ) );
                addr =  SWAP ( temp ) ;
                temp = len;

                while ( len )
                {
                    switch ( addr )
                    {
                        case 250:
                        {
                            //高字节：BIT1-485 2 BIT0-485 1
                            RS232Tx.Buff[i] = RS485States;
                            i++;
                            //低字节：BIT1-CAN 2 BIT0-CAN 1
                            RS232Tx.Buff[i] = CanStates;
                            i++;
                            addr++;
                            break;
                        }
                        case 251:
                        {
                            //高字节：BIT1-信号2 BIT0-信号1
                            RS232Tx.Buff[i] = 0 ;

                            if ( GetInputStatus ( INPUT1 ) )
                            {
                                RS232Tx.Buff[i] |= 0x1 ;
                            }

                            if ( GetInputStatus ( INPUT2 ) )
                            {
                                RS232Tx.Buff[i] |= 0x2 ;
                            }

                            i++;
                            //低字节：BIT5-BIT0 输出6-输出1 RelaySta
                            RS232Tx.Buff[i] = RelaySta ;
                            i++;
                            addr++;
                            break;
                        }
                        case 252:
                        {
                            //温度1
                            // dealtemp ( 1 );
                            g_sAnalogValue.linkv = GetAnalogSta();
                            RS232Tx.Buff[i++] = g_sAnalogValue.linkv;
                            addr++;
                            break;
                        }
                        case 253:
                        {
                            //温度2
                            dealtemp ( 2 );
                            addr++;
                            break;
                        }
                        case 254:
                        {
                            //电流
                            dealcurrent();
                            addr++;
                            break;
                        }
                        case 255:
                        {
                            //熔丝
                            dealfuse();
                            addr++;
                            break;
                        }
                    }

                    len -= 2 ;
                }

                temp += 3 ;//数据区长度 + 帧头
                CRC_Init ( CRC_POLY_CRC16 );
                result = CRC_CalcBlockChecksum ( RS232Tx.Buff, temp, CRC_WR_16BIT );
                crcl = result ;
                crch = result >> 8 ;
                RS232Tx.Buff[temp] = crcl;
                RS232Tx.Buff[temp+1] = crch;

                RS232Tx.Len = temp + 2 ; //给发送长度
                RS232SendMsg();
                break;
            }
            case 0x10:// 从主站接收到的：地址 0x10 地址高 地址低 寄存器数量高 寄存器数量低 字节数 数据1  数据2 数据N CRCL CRCH
            {
                // 发送给主站的：地址 0x10 地址高 地址低 寄存器数量高 寄存器数量低 CRCL CRCH
                RS232Tx.Buff[0] = RS232Rx.Buff[0] ;//地址
                RS232Tx.Buff[1] = RS232Rx.Buff[1] ;//03功能码
                RS232Tx.Buff[2] = RS232Rx.Buff[2] ;//
                RS232Tx.Buff[3] = RS232Rx.Buff[3] ;//
                RS232Tx.Buff[4] = RS232Rx.Buff[4] ;//
                RS232Tx.Buff[5] = RS232Rx.Buff[5] ;//
                CRC_Init ( CRC_POLY_CRC16 );
                result = CRC_CalcBlockChecksum ( RS232Tx.Buff, 6, CRC_WR_16BIT );
                crcl = result ;
                crch = result >> 8 ;
                RS232Tx.Buff[6] = crcl;
                RS232Tx.Buff[7] = crch;

                RS232Tx.Len = 8 ;//给发送长度
                RS232SendMsg(); //先发送回应给主站，避免数据处理时间过长导致通信错误
                /*以下增加具体功能实现*/
                //先取地址，地址高 地址低
                //再取长度，寄存器数量高 寄存器数量低
                //循环寄存器长度
                //依次将数据放到对应变量里
                break;
            }
            default:
                break;


        }

    }
}

void HwTestCntInc ( void )
{
    HwTestCnt++;

    if ( HwTestCnt > 1000 )
    {
        HwTestCnt = 0 ;
    }
}

void HwMain ( void )
{
    HwModbus();

    if ( GetHwTestSta() )
    {
        HwTestIO();
        HwTest485();
        HwTestCan();
        HwTestAdc();
    }
}



