#ifndef __UART_H__
#define __UART_H__


/* Receive buffer size */
#define UART_RING_BUFSIZE	256

typedef struct
{
    uint8_t  Buff[UART_RING_BUFSIZE];   //接收缓冲区
    uint8_t  Idx;                       //缓冲区指针
    uint8_t  Len;                       //接收到的数据长度 或 准备发送的数据长度
    uint8_t  Flag;                      //接收完一帧数据标记，或 发送完一帧数据标记
} UART_BUFF;

extern UART_BUFF RS232Rx, RS232Tx;
extern UART_BUFF RS485Rx1, RS485Tx1;
extern UART_BUFF RS485Rx2, RS485Tx2;
extern void FrameIsEnd ( void );
void RS232SendMsg ( void );    //发送数据，发送RS232Tx.Buff中长度RS232Tx.Len数据
void RS485Ch1SendMsg ( void ); //发送数据，发送RS485Tx1.Buff中长度RS485Tx1.Len数据
void RS485Ch2SendMsg ( void ); // 发送数据，发送RS485Tx2.Buff中长度RS485Tx2.Len数据
uint8_t GetRs232Sta ( void ); //获取RS232状态，0--空闲 2--忙  1--接收到数据帧 RS232Rx.Buff RS232Rx.Len
uint8_t GetRs485Ch1Sta ( void ); //获取RS485状态，0--空闲 2--忙  1--接收到数据帧 RS485Rx1.Buff RS485Rx1.Len
uint8_t GetRs485Ch2Sta ( void ); //获取RS485状态，0--空闲 2--忙  1--接收到数据帧 RS485Rx2.Buff RS485Rx2.Len
void UartInit ( uint8_t num, uint32_t baudrate, uint8_t parity ); //    初始化UART接口
void UartsInit ( void );


#endif