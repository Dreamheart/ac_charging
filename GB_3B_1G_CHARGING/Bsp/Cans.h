#ifndef __CAN_H__
#define __CNA_H__

#include "lpc177x_8x_can.h"

#define     CAN_TX_SIZE   128
#define     CAN_RX_SIZE   128

typedef  struct
{
    CAN_MSG_Type  txBuf[CAN_TX_SIZE];
    uint8_t   rPtr;
    uint8_t   wPtr;
} CAN_TX_BUF;

typedef  struct
{
    CAN_MSG_Type  rxBuf[CAN_RX_SIZE];
    uint8_t   rPtr;
    uint8_t   wPtr;
} CAN_RX_BUF;


extern void CanInit ( void );
extern uint8_t CAN_send ( uint8_t canId, CAN_MSG_Type *TXMsg );
extern uint8_t CAN_receive ( uint8_t canId, CAN_MSG_Type *RXMsg );
extern void CAN_Send_main ( void );
extern void can_test(void);
#endif