
#include "lpc177x_8x_can.h"
#include "lpc177x_8x_pinsel.h"
#include "debug_frmwrk.h"

#include "bsp_inc.h"

CAN_RX_BUF        can1_rx_buf;
CAN_RX_BUF        can2_rx_buf;

CAN_TX_BUF        can1_tx_buf;
CAN_TX_BUF        can2_tx_buf;


void CAN_Callback_snd()
{

}

/*************************************************************
  Function: void CAN_Callback_err()
  Description: CAN错误码中断函数回调函数
  Calls:       无
  Called By:   无
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void CAN_Callback_err()
{

}

/*************************************************************
  Function: void CAN_IRQHandler()
  Description: CAN中断函数，处理两个CAN的接收数据
  Calls:       CAN_IntGetStatus
               CAN_IntHandler
  Called By:   无
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void CAN_IRQHandler ( void )
{
    uint8_t IntStatus;

    IntStatus = CAN_IntGetStatus ( CAN_1 );

    if ( IntStatus & 0x01 != 0 )          //最大CAN通道数为2，分别检测两个通道的中断
    {
        CAN_ReceiveMsg ( CAN_1, &can1_rx_buf.rxBuf[can1_rx_buf.wPtr++] );
        can1_rx_buf.wPtr &= CAN_RX_SIZE - 1;
    }

    IntStatus = CAN_IntGetStatus ( CAN_2 );

    if ( IntStatus & 0x01 != 0 )          //最大CAN通道数为2，分别检测两个通道的中断
    {
        CAN_ReceiveMsg ( CAN_2, &can2_rx_buf.rxBuf[can2_rx_buf.wPtr++] );
        can2_rx_buf.wPtr &= CAN_RX_SIZE - 1;
    }
}
/*************************************************************
  Function: uint8_t CAN_send(uint8_t canId,CAN_MSG_Type *TXMsg)
  Description: CAN通讯数据发送缓冲填充，处理两个CAN的数据
  Calls:       CAN_IntGetStatus
               CAN_ReceiveMsg
  Called By:   无
  Input:       canId 对应CAN通路
               TXMsg 待发送数据
  Output:      RXMsg
  Return:      0：发送成功 1:发送区满
  Others:      无
*************************************************************/
uint8_t CAN_send ( uint8_t canId, CAN_MSG_Type *TXMsg )
{
    CAN_TX_BUF *pCanTx;
    if ( canId == CAN_1 )
    {
        pCanTx = &can1_tx_buf;
    }
    else if ( canId == CAN_2 )
    {
        pCanTx = &can2_tx_buf;
    }

    if ( ! ( pCanTx->rPtr == ( pCanTx->wPtr + 1 ) % CAN_TX_SIZE ) )
    {
        memcpy ( pCanTx->txBuf + pCanTx->wPtr, TXMsg, sizeof ( CAN_MSG_Type ) );
        pCanTx->wPtr = ( pCanTx->wPtr + 1 ) % CAN_TX_SIZE;
        return 0;
    }

    return 1;
}

void CanErr ( void )
{
    LPC_CAN_TypeDef *CANx;
    uint32_t GSR;
    uint32_t canbase[2] = {LPC_CAN1_BASE, LPC_CAN2_BASE};

    for ( int i = 0; i < 2; i++ )
    {
        CANx  = ( LPC_CAN_TypeDef * ) canbase[i];
        GSR = CANx->GSR;

        if ( ( ( GSR >> 16 ) & 0xff ) >= 0x7f || ( ( GSR >> 24 ) & 0xff ) >= 0x7f )
        {
            CANx->MOD = 1; // Enter Reset Mode
            CANx->IER = 0; // Disable All CAN Interrupts
            CANx->GSR = 0;

            /* Request command to release Rx, Tx buffer and clear data overrun */
            for ( i = 0; i < 20; i++ )
            {
                CANx->CMR = ( 1 << 1 ) | ( 1 << 2 ) | ( 1 << 3 );    /* 释放接收BOX */
            }

            /* Read to clear interrupt pending in interrupt capture register */
            volatile uint32_t temp = CANx->ICR;

            CANx->IER = ( 1 << CANINT_RIE );            // Enable All CAN Interrupts
            CANx->MOD = 0;                                // Return Normal operating
        }
    }
}

/*************************************************************
  Function: uint8_t CAN_receive(uint8_t canId,CAN_MSG_Type *RXMsg)
  Description: CAN通讯数据接收缓冲，处理两个CAN的数据
  Calls:       CAN_IntGetStatus
               CAN_ReceiveMsg
  Called By:   无
  Input:       canId 对应CAN通路
               RXMsg 提取数据目标地址
  Output:      RXMsg
  Return:      0：读取成功 1:接送区空
  Others:      无
*************************************************************/
uint8_t CAN_receive ( uint8_t canId, CAN_MSG_Type *RXMsg )
{

    if ( canId == CAN_1 )
    {
        if ( can1_rx_buf.rPtr != can1_rx_buf.wPtr )
        {
            *RXMsg = can1_rx_buf.rxBuf[can1_rx_buf.rPtr++];
            can1_rx_buf.rPtr &= CAN_RX_SIZE - 1;
            return 0;
        }
    }
    else if ( canId == CAN_2 )
    {
        if ( can2_rx_buf.rPtr != can2_rx_buf.wPtr )
        {
            *RXMsg = can2_rx_buf.rxBuf[can2_rx_buf.rPtr++];
            can2_rx_buf.rPtr &= CAN_RX_SIZE - 1;
            return 0;
        }
    }

    return 1;
}
/*************************************************************
  Function: void CAN_Send_main()
  Description: CAN总发送函数，处理两个CAN的数据发送
  Calls:       CAN_SendMsg

  Called By:   void main(void)
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void CAN_Send_main ( void )
{
    if ( can1_tx_buf.wPtr != can1_tx_buf.rPtr )
    {
        while ( CAN_SendMsg ( CAN_1, can1_tx_buf.txBuf + can1_tx_buf.rPtr ) == SUCCESS )
        {
            can1_tx_buf.rPtr = ( can1_tx_buf.rPtr + 1 ) % CAN_TX_SIZE;

            //if ( can1_tx_buf.rPtr == can1_tx_buf.wPtr )
            {
                break;
            }
        }
    }

    if ( can2_tx_buf.wPtr != can2_tx_buf.rPtr )
    {
        while ( CAN_SendMsg ( CAN_2, can2_tx_buf.txBuf + can2_tx_buf.rPtr ) == SUCCESS )
        {
            can2_tx_buf.rPtr = ( can2_tx_buf.rPtr + 1 ) % CAN_TX_SIZE;

            //if ( can2_tx_buf.rPtr == can2_tx_buf.wPtr )
            {
                break;
            }
        }
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
    CAN_Init ( CAN_2, 250000 );


    CAN_Init ( CAN_1, 250000 );

    //Enable Interrupt
    ////CAN_IRQCmd(TRANSMIT_CAN_NO, CANINT_TIE1, ENABLE);

    CAN_IRQCmd ( CAN_1, CANINT_RIE, ENABLE );

    CAN_IRQCmd ( CAN_2, CANINT_RIE, ENABLE );

    //Enable CAN Interrupt
    NVIC_EnableIRQ ( CAN_IRQn );

    //CAN_InitMessage();

    //PrintMessage(&TXMsg);

    CAN_SetAFMode ( CAN_ACC_BP );

    //CAN_SendMsg(CAN_1, &TXMsg);

}

void can_test(void)
{
    static uint32_t CantstTick;
    CAN_MSG_Type  TxMsg;
    CAN_MSG_Type  RxMsg;
    
    if ( CAN_receive ( CAN_1, &RxMsg ) == 0)
    {
        if(RxMsg.id == 0x01)
        {
            TxMsg.format = EXT_ID_FORMAT;
            TxMsg.id = 0x02;
            TxMsg.len = 8;
            TxMsg.type = DATA_FRAME;
            TxMsg.dataA[0] = 0x11 ; 
            TxMsg.dataA[1] = 0x22 ; 
            TxMsg.dataA[2] = 0x33 ; 
            TxMsg.dataA[3] = 0x44 ; 
            TxMsg.dataB[0] = 0x55 ; 
            TxMsg.dataB[1] = 0x66 ; 
            TxMsg.dataB[2] = 0x77 ; 
            TxMsg.dataB[3] = 0x88 ; 
            CAN_send ( CAN_1, &TxMsg );
        }
    }
    extern volatile unsigned long  SysTickCnt;
    if(SysTickCnt < CantstTick + 500)return;
    CantstTick = SysTickCnt;
    // sent infomation
      
    TxMsg.format = EXT_ID_FORMAT;
    TxMsg.id = 0xa555aaa;
    TxMsg.len = 8;
    TxMsg.type = DATA_FRAME;
    TxMsg.dataA[0] = 1 ; 
    TxMsg.dataA[1] = 2 ; 
    TxMsg.dataA[2] = 3 ; 
    TxMsg.dataA[3] = 4 ; 
    TxMsg.dataB[0] = 5 ; 
    TxMsg.dataB[1] = 6 ; 
    TxMsg.dataB[2] = 7 ; 
    TxMsg.dataB[3] = 8 ; 
    CAN_send ( CAN_1, &TxMsg );
}