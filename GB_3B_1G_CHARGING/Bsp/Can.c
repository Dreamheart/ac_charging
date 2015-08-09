/**********************************************************
  江苏嘉钰新能源技术有限公司
  File name:  Can.c	              // 文件名
  Author:     王辉                    // 作者
  Version:    v1.00                   // 版本信息
  Date:       2012-8-15               // 完成日期
  Description:  	              // CAN的驱动程序
  Others:		              // 无
  Function List:	              // void CanInit(void)
                                         void SetCanBuadRate(uint8_t num,uint32_t buadrate)
                                         CAN_SendMsg(CAN, &TXMsg);

  History:
**********************************************************/
#include "lpc177x_8x_can.h"
#include "lpc177x_8x_pinsel.h"
#include "debug_frmwrk.h"

#include "bsp_inc.h"

/** CAN variable definition **/
CAN_MSG_Type TXMsg[2], RXMsg[2]; // messages for test Bypass mode
uint32_t CANRxCount, CANTxCount = 0;

/************************** PRIVATE FUNCTIONS *************************/
void CAN_IRQHandler ( void );
void CAN_InitMessage ( void );
void PrintMessage ( CAN_MSG_Type *msg );


/*************************************************************
  Function: void CAN_IRQHandler()
  Description: CAN中断函数，处理两个CAN的接收数据
  Calls:       CAN_IntGetStatus
               CAN_ReceiveMsg
  Called By:   无
  Input:       无
  Output:      RXMsg
  Return:      无
  Others:      无
*************************************************************/
void CAN_IRQHandler()
{
    uint8_t IntStatus;
    uint32_t data1;

    /* get interrupt status
     * Note that: Interrupt register CANICR will be reset after read.
     * So function "CAN_IntGetStatus" should be call only one time
     */
    IntStatus = CAN_IntGetStatus ( CAN_1 );

    //check receive interrupt
    if ( ( IntStatus >> 0 ) & 0x01 )
    {
        CAN_ReceiveMsg ( CAN_1, &RXMsg[0] );
        //PrintMessage(&RXMsg);
        /*
        		CANRxCount++; //count success received message

        		//increase data for next TX Message
        		TXMsg.id ++;

        		data1 = (TXMsg.dataA[0])|(((TXMsg.dataA[1]))<<8)|((TXMsg.dataA[2])<<16)|((TXMsg.dataA[3])<<24);

        		if(data1 == 0xFFFFFFFF)
        			data1 = 0;
        		else
        			data1++;

        		*((uint8_t *) &TXMsg.dataA[0])= *((uint8_t *) &TXMsg.dataB[0])= data1 & 0x000000FF;
        		*((uint8_t *) &TXMsg.dataA[1])= *((uint8_t *) &TXMsg.dataB[1])=(data1 & 0x0000FF00)>>8;;
        		*((uint8_t *) &TXMsg.dataA[2])= *((uint8_t *) &TXMsg.dataB[2])=(data1 & 0x00FF0000)>>16;
        		*((uint8_t *) &TXMsg.dataA[3])= *((uint8_t *) &TXMsg.dataB[3])=(data1 & 0xFF000000)>>24;

        		CAN_SendMsg(CAN_1, &TXMsg);*/
    }

    IntStatus = CAN_IntGetStatus ( CAN_2 );

    //check receive interrupt
    if ( ( IntStatus >> 0 ) & 0x01 )
    {
        CAN_ReceiveMsg ( CAN_2, &RXMsg[1] );
        /*		PrintMessage(&RXMsg);

        		CANRxCount++; //count success received message

        		//increase data for next TX Message
        		TXMsg.id ++;

        		data1 = (TXMsg.dataA[0])|(((TXMsg.dataA[1]))<<8)|((TXMsg.dataA[2])<<16)|((TXMsg.dataA[3])<<24);

        		if(data1 == 0xFFFFFFFF)
        			data1 = 0;
        		else
        			data1++;

        		*((uint8_t *) &TXMsg.dataA[0])= *((uint8_t *) &TXMsg.dataB[0])= data1 & 0x000000FF;
        		*((uint8_t *) &TXMsg.dataA[1])= *((uint8_t *) &TXMsg.dataB[1])=(data1 & 0x0000FF00)>>8;;
        		*((uint8_t *) &TXMsg.dataA[2])= *((uint8_t *) &TXMsg.dataB[2])=(data1 & 0x00FF0000)>>16;
        		*((uint8_t *) &TXMsg.dataA[3])= *((uint8_t *) &TXMsg.dataB[3])=(data1 & 0xFF000000)>>24;

        		CAN_SendMsg(CAN_2, &TXMsg);*/
    }
}

/*-------------------------PRIVATE FUNCTIONS----------------------------*/
/*********************************************************************//**
 * @brief		Print Message via COM1
 * param[in]	msg: point to CAN_MSG_Type object that will be printed
 * @return 		none
 **********************************************************************/
void PrintMessage ( CAN_MSG_Type *CAN_Msg )
{
    uint32_t data;

    _DBG ( "Message ID:     " );

    _DBH32 ( CAN_Msg->id );
    _DBG_ ( "" );

    _DBG ( "Message length: " );

    _DBH32 ( CAN_Msg->len );
    _DBG_ ( " BYTES" );

    _DBG ( "Message type:   " );

    if ( CAN_Msg->type == DATA_FRAME )
    {
        _DBG_ ( "DATA FRAME " );
    }
    else
    {
        _DBG_ ( "REMOTE FRAME " );
    }

    _DBG ( "Message format: " );

    if ( CAN_Msg->format == STD_ID_FORMAT )
    {
        _DBG_ ( "STANDARD ID FRAME FORMAT" );
    }
    else
    {
        _DBG_ ( "EXTENDED ID FRAME FORMAT" );
    }

    _DBG ( "Message dataA:  " );

    data = ( CAN_Msg->dataA[0] ) | ( CAN_Msg->dataA[1] << 8 ) | ( CAN_Msg->dataA[2] << 16 ) | ( CAN_Msg->dataA[3] << 24 );

    _DBH32 ( data );
    _DBG_ ( "" );

    data = ( CAN_Msg->dataB[0] ) | ( CAN_Msg->dataB[1] << 8 ) | ( CAN_Msg->dataB[2] << 16 ) | ( CAN_Msg->dataB[3] << 24 );

    _DBG ( "Message dataB:  " );

    _DBH32 ( data );
    _DBG_ ( "" );

    _DBG_ ( "" );
}

/*********************************************************************//**
 * @brief		Initialize transmit and receive message for Bypass operation
 * @param[in]	none
 * @return 		none
 **********************************************************************/
void CAN_InitMessage ( void )
{
    TXMsg[0].format = EXT_ID_FORMAT;
    TXMsg[0].id = 0x00001234;
    TXMsg[0].len = 8;
    TXMsg[0].type = DATA_FRAME;
    TXMsg[0].dataA[0] = TXMsg[0].dataA[1] = TXMsg[0].dataA[2] = TXMsg[0].dataA[3] = 0x00000000;
    TXMsg[0].dataB[0] = TXMsg[0].dataB[1] = TXMsg[0].dataB[2] = TXMsg[0].dataB[3] = 0x00000000;

    RXMsg[0].format = 0x00;
    RXMsg[0].id = 0x00;
    RXMsg[0].len = 0x00;
    RXMsg[0].type = 0x00;
    RXMsg[0].dataA[0] = RXMsg[0].dataA[1] = RXMsg[0].dataA[2] = RXMsg[0].dataA[3] = 0x00000000;
    RXMsg[0].dataB[0] = RXMsg[0].dataB[1] = RXMsg[0].dataB[2] = RXMsg[0].dataB[3] = 0x00000000;

    TXMsg[1].format = EXT_ID_FORMAT;
    TXMsg[1].id = 0x00001234;
    TXMsg[1].len = 8;
    TXMsg[1].type = DATA_FRAME;
    TXMsg[1].dataA[0] = TXMsg[1].dataA[1] = TXMsg[1].dataA[2] = TXMsg[1].dataA[3] = 0x00000000;
    TXMsg[1].dataB[0] = TXMsg[1].dataB[1] = TXMsg[1].dataB[2] = TXMsg[1].dataB[3] = 0x00000000;

    RXMsg[1].format = 0x00;
    RXMsg[1].id = 0x00;
    RXMsg[1].len = 0x00;
    RXMsg[1].type = 0x00;
    RXMsg[1].dataA[0] = RXMsg[1].dataA[1] = RXMsg[1].dataA[2] = RXMsg[1].dataA[3] = 0x00000000;
    RXMsg[1].dataB[0] = RXMsg[1].dataB[1] = RXMsg[1].dataB[2] = RXMsg[1].dataB[3] = 0x00000000;
}

/*************************************************************
  Function: void SetCanBuadRate(uint8_t num,uint32_t buadrate)
  Description: 设置CAN的波特率
  Calls:       CAN_Init
  Called By:
  Input:       num--CAN端口号，1或者2
               buadrate--波特率
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void SetCanBuadRate ( uint8_t num, uint32_t buadrate )
{
    if ( num == 1 )
    {
        CAN_Init ( CAN_1, buadrate );
    }

    if ( num == 2 )
    {
        CAN_Init ( CAN_2, buadrate );
    }
}
/*************************************************************
  Function: void CanInit(void)
  Description: 初始化CAN端口，配置管脚，开接收中断
  Calls:
  Called By:   main()
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void CanInit ( void )
{
    PINSEL_ConfigPin ( 0, 21, 4 );

    PINSEL_ConfigPin ( 0, 22, 4 );

    PINSEL_ConfigPin ( 0, 4, 2 );

    PINSEL_ConfigPin ( 0, 5, 2 );


    //Initialize CAN1 & CAN2
    CAN_Init ( CAN_1, 125000 );


    CAN_Init ( CAN_2, 125000 );

    //Enable Interrupt
    ////CAN_IRQCmd(TRANSMIT_CAN_NO, CANINT_TIE1, ENABLE);

    CAN_IRQCmd ( CAN_1, CANINT_RIE, ENABLE );

    CAN_IRQCmd ( CAN_2, CANINT_RIE, ENABLE );

    //Enable CAN Interrupt
    NVIC_EnableIRQ ( CAN_IRQn );

    CAN_InitMessage();

    //PrintMessage(&TXMsg);

    CAN_SetAFMode ( CAN_ACC_BP );

    //CAN_SendMsg(CAN_1, &TXMsg);

}


