/*
 * @brief UART DMA example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "string.h"
#include "uart.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#ifdef __ICCARM__
#define ALIGNSTR(x) #x
#define ALIGN(x) _Pragma(ALIGNSTR(data_alignment=##x))
#else
#define ALIGN(x) __attribute__((aligned(x)))
#endif

//#define USE_INTEGER_CLOCK
#define FRAMESPACETIME 100

UART_BUFF RS232Rx, RS232Tx;
UART_BUFF RS485Rx1, RS485Tx1;

uint8_t RS485TxEndCnt;
uint8_t RS485TxEndFlag,FrameCntFlag;

uint8_t FrameCntRS232;
uint8_t FrameCntRS4851;

uint8_t PackNum,PackNum1;

/* DMA send string arrays. DMA buffers must remain in memory during the DMA
   transfer. */
#define DMASENDSTRCNT   6


/* Number of UART TX descriptors used for DMA */
#define UARTTXDESC 8

/* Next available UART TX DMA descriptor and use counter */
static volatile int nextTX0Desc,nextTX1Desc, countTX0DescUsed,countTX1DescUsed;

/* Number of UART RX descriptors used for DMA */
#define UARTRXD0ESC 32
#define UARTRXD1ESC 8
/* Maximum size of each UART RX receive buffer */
#define UARTRXBUFFSIZE  8

/* UART RX receive buffers */
static uint8_t dmaRX0Buffs[UARTRXD0ESC][UARTRXBUFFSIZE]; //RS232 BUFF 32*8=256 BYTES 
static uint8_t dmaRX1Buffs[UARTRXD1ESC][UARTRXBUFFSIZE]; //RS485 BUFF 8*8=64 BYTES
/* UART receive buffer that is available and availa flag */
static volatile int uartRX0Buff,uartRX1Buff;
static volatile bool uartRxAvail;

/* DMA descriptors must be aligned to 16 bytes */

ALIGN(16) static DMA_CHDESC_T dmaTXDesc[UARTTXDESC];
ALIGN(16) static DMA_CHDESC_T dmaRX0Desc[UARTRXD0ESC],dmaRX1Desc[UARTRXD1ESC];

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/


/* Setup DMA UART TX support, but do not queue descriptors yet */
static void DmaTX0Setup(void)
{
	/* Setup UART 0 TX channel for the following configuration:
	   - Peripheral DMA request (UART 0 TX channel)
	   - Single transfer
	   - Low channel priority */
	Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_USART0_TX);
	Chip_DMA_EnableIntChannel(LPC_DMA, DMAREQ_USART0_TX);
	Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_USART0_TX,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(3)));

	countTX0DescUsed = 0;
}
/* Setup DMA UART TX support, but do not queue descriptors yet */
static void DmaTX1Setup(void)
{
	/* Setup UART 0 TX channel for the following configuration:
	   - Peripheral DMA request (UART 0 TX channel)
	   - Single transfer
	   - Low channel priority */
	Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_USART1_TX);
	Chip_DMA_EnableIntChannel(LPC_DMA, DMAREQ_USART1_TX);
	Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_USART1_TX,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(3)));

	countTX1DescUsed = 0;
}
/* Setup DMA UART RX support, but do not queue descriptors yet */
static void dmaRX0Setup(void)
{
	/* Setup UART 0 RX channel for the following configuration:
	   - Peripheral DMA request (UART 0 RX channel)
	   - Single transfer
	   - Low channel priority */
	Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_USART0_RX);
	Chip_DMA_EnableIntChannel(LPC_DMA, DMAREQ_USART0_RX);
	Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_USART0_RX,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(3)));
}
/* Setup DMA UART RX support, but do not queue descriptors yet */
static void dmaRX1Setup(void)
{
	/* Setup UART 0 RX channel for the following configuration:
	   - Peripheral DMA request (UART 0 RX channel)
	   - Single transfer
	   - Low channel priority */
	Chip_DMA_EnableChannel(LPC_DMA, DMAREQ_USART1_RX);
	Chip_DMA_EnableIntChannel(LPC_DMA, DMAREQ_USART1_RX);
	Chip_DMA_SetupChannelConfig(LPC_DMA, DMAREQ_USART1_RX,
								(DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL | DMA_CFG_CHPRIORITY(3)));
}


/* Queue up DMA descriptors and buffers for UART RX */
static void dmaRX0Queue(void)
{
	int i;

	/* Linked list of descriptors that map to the 3 receive buffers */
	for (i = 0; i < UARTRXD0ESC; i++) 
        {
		/* Setup next descriptor */
		if (i == (UARTRXD0ESC - 1)) 
                {
			/* Wrap descriptors */
			dmaRX0Desc[i].next = DMA_ADDR(&dmaRX0Desc[0]);
		}
		else 
                {
			dmaRX0Desc[i].next = DMA_ADDR(&dmaRX0Desc[i + 1]);
		}

		/* Create a descriptor for the data */
		dmaRX0Desc[i].source = DMA_ADDR(&LPC_USART0->RXDATA) + 0;	/* Byte aligned */
		dmaRX0Desc[i].dest = DMA_ADDR(&dmaRX0Buffs[i][0] + UARTRXBUFFSIZE - 1);

		/* Setup transfer configuration */
		dmaRX0Desc[i].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_0 |
							   DMA_XFERCFG_DSTINC_1 | DMA_XFERCFG_RELOAD |
							   DMA_XFERCFG_XFERCOUNT(UARTRXBUFFSIZE);
	}

	/* Setup transfer descriptor and validate it */
	Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_USART0_RX, &dmaRX0Desc[0]);

	/* Setup data transfer */
	Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_USART0_RX,
								  dmaRX0Desc[0].xfercfg);
	Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_USART0_RX);
	Chip_DMA_SWTriggerChannel(LPC_DMA, DMAREQ_USART0_RX);
}

/* Queue up DMA descriptors and buffers for UART RX */
static void dmaRX1Queue(void)
{
	int i;

	/* Linked list of descriptors that map to the 3 receive buffers */
	for (i = 0; i < UARTRXD1ESC; i++) 
        {
		/* Setup next descriptor */
		if (i == (UARTRXD1ESC - 1)) 
                {
			/* Wrap descriptors */
			dmaRX1Desc[i].next = DMA_ADDR(&dmaRX1Desc[0]);
		}
		else 
                {
			dmaRX1Desc[i].next = DMA_ADDR(&dmaRX1Desc[i + 1]);
		}

		/* Create a descriptor for the data */
		dmaRX1Desc[i].source = DMA_ADDR(&LPC_USART1->RXDATA) + 0;	/* Byte aligned */
		dmaRX1Desc[i].dest = DMA_ADDR(&dmaRX1Buffs[i][0] + UARTRXBUFFSIZE - 1);

		/* Setup transfer configuration */
		dmaRX1Desc[i].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
							   DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_0 |
							   DMA_XFERCFG_DSTINC_1 | DMA_XFERCFG_RELOAD |
							   DMA_XFERCFG_XFERCOUNT(UARTRXBUFFSIZE);
	}

	/* Setup transfer descriptor and validate it */
	Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_USART1_RX, &dmaRX1Desc[0]);

	/* Setup data transfer */
	Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_USART1_RX,
								  dmaRX1Desc[0].xfercfg);
	Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_USART1_RX);
	Chip_DMA_SWTriggerChannel(LPC_DMA, DMAREQ_USART1_RX);
}
/* Clear an error on a DMA channel */
static void dmaClearChannel(DMA_CHID_T ch) {
	Chip_DMA_DisableChannel(LPC_DMA, ch);
	while ((Chip_DMA_GetBusyChannels(LPC_DMA) & (1 << ch)) != 0) {}

	Chip_DMA_AbortChannel(LPC_DMA, ch);
	Chip_DMA_ClearErrorIntChannel(LPC_DMA, ch);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	DMA Interrupt Handler
 * @return	None
 */
void DMA_IRQHandler(void)
{
	uint32_t errors, pending;

	/* Get DMA error and interrupt channels */
	errors = Chip_DMA_GetErrorIntChannels(LPC_DMA);
	pending = Chip_DMA_GetActiveIntAChannels(LPC_DMA);

	/* Check DMA interrupts of UART 0 TX channel */
	if ((errors | pending) & (1 << DMAREQ_USART0_TX)) {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_USART0_TX);

		/* Handle errors if needed */
		if (errors & (1 << DMAREQ_USART0_TX)) 
                {
			/* DMA error, channel needs to be reset */
			dmaClearChannel(DMAREQ_USART0_TX);
			DmaTX0Setup();
                        RS232Tx.Flag = 0;
		}
		else 
                {
			/* Descriptor is consumed */
			RS232Tx.Flag = 0;
		}
	}
	/* Check DMA interrupts of UART 0 TX channel */
	if ((errors | pending) & (1 << DMAREQ_USART1_TX)) {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_USART1_TX);

		/* Handle errors if needed */
		if (errors & (1 << DMAREQ_USART1_TX)) 
                {
			/* DMA error, channel needs to be reset */
			dmaClearChannel(DMAREQ_USART1_TX);
			DmaTX1Setup();
                        RS485Tx1.Flag = 0;
		}
		else 
                {
			/* Descriptor is consumed */
			RS485Tx1.Flag = 0;
		}
                RS485TxEndCnt = 0 ;
                RS485TxEndFlag = 1 ;//Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,14, 0 );
	}
	/* Check DMA interrupts of UART 0 RX channel */
	if ((errors | pending) & (1 << DMAREQ_USART0_RX)) 
        {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_USART0_RX);

		/* Handle errors if needed */
		if (errors & (1 << DMAREQ_USART0_RX)) 
                {
			/* DMA error, channel needs to be reset */
			dmaClearChannel(DMAREQ_USART0_RX);
			dmaRX0Setup();
			dmaRX0Queue();
		}
		else 
                {
                        //memcpy(&RS232Rx.Buff[RS232Rx.Idx], dmaRXBuffs[uartRXBuff], UARTRXBUFFSIZE);
                        uartRX0Buff++;
                        //RS232Rx.Len = UARTRXBUFFSIZE;
                        // += RS232Rx.Len ;
                        FrameCntRS232 = 0 ;                  
		}
	}
	/* Check DMA interrupts of UART 0 RX channel */
	if ((errors | pending) & (1 << DMAREQ_USART1_RX)) 
        {
		/* Clear DMA interrupt for the channel */
		Chip_DMA_ClearActiveIntAChannel(LPC_DMA, DMAREQ_USART1_RX);

		/* Handle errors if needed */
		if (errors & (1 << DMAREQ_USART1_RX)) 
                {
			/* DMA error, channel needs to be reset */
			dmaClearChannel(DMAREQ_USART1_RX);
			dmaRX1Setup();
			dmaRX1Queue();
		}
		else 
                {
                        //memcpy(&RS485Rx.Buff[RS232Rx.Idx], dmaRXBuffs[uartRXBuff], UARTRXBUFFSIZE);
                        uartRX1Buff++;
                        //RS232Rx.Len = UARTRXBUFFSIZE;
                        //RS232Rx.Idx += RS232Rx.Len ;
                        FrameCntRS4851 = 0 ;                  
		}
	}
}


void SetupUART(void)
{

/* Use 128x expected UART baud rate for fractional baud mode. */
	Chip_Clock_SetUSARTNBaseClockRate((115200 * 128), true);
	/* Setup UART */
	Chip_UART_Init(LPC_USART0);
	Chip_UART_ConfigData(LPC_USART0, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1);
	Chip_UART_SetBaud(LPC_USART0, 38400);
	/* Optional for low clock rates only: Chip_UART_SetBaudWithRTC32K(LPC_USART, 300); */
	Chip_UART_Enable(LPC_USART0);
	Chip_UART_TXEnable(LPC_USART0);
        LPC_USART0->INTENSET = (1 << 0);                                    /* 使能接收中断                 */
        NVIC_EnableIRQ(UART0_IRQn);                                         /* 打开UART中断                 */
	Chip_UART_Init(LPC_USART1);
	Chip_UART_ConfigData(LPC_USART1, UART_CFG_DATALEN_8 | UART_CFG_PARITY_EVEN | UART_CFG_STOPLEN_1);
	Chip_UART_SetBaud(LPC_USART1, 2400); //RS485
        LPC_USART1->INTENSET = (1 << 0);                                    /* 使能接收中断                 */
        NVIC_EnableIRQ(UART1_IRQn);                                         /* 打开UART中断                 */
	Chip_UART_Enable(LPC_USART1);
	Chip_UART_TXEnable(LPC_USART1);
        Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,14, 0 );
        
	/* DMA initialization - enable DMA clocking and reset DMA if needed */
	Chip_DMA_Init(LPC_DMA);

	/* Enable DMA controller and use driver provided DMA table for current descriptors */
	Chip_DMA_Enable(LPC_DMA);
	Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));

	/* Setup UART 0 TX DMA support */
	DmaTX0Setup();

	/* Setup UART 0 RX DMA support */
	//dmaRX0Setup();
        //dmaRX0Queue();
	/* Setup UART 1 TX DMA support */
	DmaTX1Setup();

	/* Setup UART 1 RX DMA support */
	//dmaRX1Setup();
        //dmaRX1Queue();        
	/* Enable the DMA IRQ */
	NVIC_EnableIRQ(DMA_IRQn);
}


/*************************************************************
  Function: void RS232SendMsg(void)
  Description: RS232发送数据函数，用于启动发送
  Calls:       GPDMA_Setup
               GPDMA_ChannelCmd
  Called By:
  Input:       RS232Tx.Buff
               RS232Tx.Len
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void RS232SendMsg ( void )
{
	/* Create a descriptor for the data */
	dmaTXDesc[0].source = DMA_ADDR(RS232Tx.Buff + RS232Tx.Len - 1);	/* Last address here */
	dmaTXDesc[0].dest = DMA_ADDR(&LPC_USART0->TXDATA);	/* Byte aligned */

	/* If there are multiple buffers with non-contiguous addresses, they can be chained
	   together here (it is recommended to only use the DMA_XFERCFG_SETINTA on the
	   last chained descriptor). If another TX buffer needs to be sent, the DMA
	   IRQ handler will re-queue and send the buffer there without using chaining. */
	dmaTXDesc[0].next = DMA_ADDR(0);

	/* Setup transfer configuration */
	dmaTXDesc[0].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
										 DMA_XFERCFG_SWTRIG | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_1 |
										 DMA_XFERCFG_DSTINC_0 | DMA_XFERCFG_XFERCOUNT(RS232Tx.Len);

	/* If a transfer is currently in progress, then stop here and let the DMA
	   handler re-queue the next transfer. Otherwise, start the transfer here. */

		/* Setup transfer descriptor and validate it */
		Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_USART0_TX, &dmaTXDesc[0]);

		/* Setup data transfer */
		Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_USART0_TX,dmaTXDesc[0].xfercfg);

		Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_USART0_TX);

}
/*************************************************************
  Function: void RS485Ch1SendMsg(void)
  Description: RS485发送数据函数，用于启动发送
  Calls:       GPDMA_Setup
               GPDMA_ChannelCmd
  Called By:
  Input:       RS485Tx1.Buff
               RS485Tx1.Len
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void RS485Ch1SendMsg ( void )
{
    //    RS485Tx1.Len += 2 ;//保证数据可以发完
        Chip_UART_TXEnable(LPC_USART1);
        Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,14, 1 );
	/* Create a descriptor for the data */
	dmaTXDesc[1].source = DMA_ADDR(RS485Tx1.Buff + RS485Tx1.Len - 1);	/* Last address here */
	dmaTXDesc[1].dest = DMA_ADDR(&LPC_USART1->TXDATA);	/* Byte aligned */

	/* If there are multiple buffers with non-contiguous addresses, they can be chained
	   together here (it is recommended to only use the DMA_XFERCFG_SETINTA on the
	   last chained descriptor). If another TX buffer needs to be sent, the DMA
	   IRQ handler will re-queue and send the buffer there without using chaining. */
	dmaTXDesc[1].next = DMA_ADDR(0);

	/* Setup transfer configuration */
	dmaTXDesc[1].xfercfg = DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA |
										 DMA_XFERCFG_SWTRIG | DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_1 |
										 DMA_XFERCFG_DSTINC_0 | DMA_XFERCFG_XFERCOUNT(RS485Tx1.Len);

	/* If a transfer is currently in progress, then stop here and let the DMA
        handler re-queue the next transfer. Otherwise, start the transfer here. */
        
        /* Setup transfer descriptor and validate it */
        Chip_DMA_SetupTranChannel(LPC_DMA, DMAREQ_USART1_TX, &dmaTXDesc[1]);
        
        /* Setup data transfer */
        Chip_DMA_SetupChannelTransfer(LPC_DMA, DMAREQ_USART1_TX,dmaTXDesc[1].xfercfg);
        
        Chip_DMA_SetValidChannel(LPC_DMA, DMAREQ_USART1_TX);
}
/*************************************************************
Function: uint8_t GetRs232Sta(void)
  Description: 获取RS232状态，0--空闲 2--忙  1--接收到数据帧 RS232Rx.Buff RS232Rx.Len
  Calls:
  Called By:
  Input:
  Output:      无
  Return:      RS232状态
  Others:      无
*************************************************************/
uint8_t GetRs232Sta ( void )
{
    uint8_t sta = 0 ;

    if ( RS232Tx.Flag == 1 )
    {
        sta = 2 ;
    }
    else
    {
        sta = RS232Rx.Flag ;
    }

    return sta ;
}


/*************************************************************
Function: uint8_t GetRs485Ch1Sta(void)
  Description: 获取RS485状态，0--空闲 2--忙  1--接收到数据帧 RS232Rx.Buff RS232Rx.Len
  Calls:
  Called By:
  Input:
  Output:      无
  Return:      RS485状态
  Others:      无
*************************************************************/
uint8_t GetRs485Ch1Sta ( void )
{
    uint8_t sta = 0 ;

    if ( RS485Tx1.Flag == 1 )
    {
        sta = 2 ;
    }
    else
    {
        sta = RS485Rx1.Flag ;
    }

    return sta ;
}

/*********************************************************************************************************
** Function name:       UART0_IRQHandler
** Descriptions:        UART0中断服务函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void UART0_IRQHandler (void)
{
    if (LPC_USART0->STAT & 0x01) 
    {                                      /* 接收中断                     */
        
        RS232Rx.Buff[RS232Rx.Idx] = LPC_USART0->RXDATA;                          /* 接收数据                     */
        RS232Rx.Idx++;
        FrameCntRS232 = 0 ;  
        uartRX0Buff = 1 ;
    }

}
/*********************************************************************************************************
** Function name:       UART0_IRQHandler
** Descriptions:        UART0中断服务函数
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void UART1_IRQHandler (void)
{
    if (LPC_USART1->STAT & 0x01) 
    {                                      /* 接收中断                     */
        
        RS485Rx1.Buff[RS485Rx1.Idx] = LPC_USART1->RXDATA;                          /* 接收数据                     */
        RS485Rx1.Idx++;
        FrameCntRS4851 = 0 ;  
        uartRX1Buff = 1 ;
    }

}
/*************************************************************
Function: void FrameIsEnd(void)
  Description: 判断串口接收是否结束，目前在进入RDA中断后如20ms不在产生接收中断，认为帧结束
  Calls:
  Called By:   SysTick_Handler
  Input:       无
  Output:      帧结束标记
  Return:      无
  Others:      无
*************************************************************/
void FrameIsEnd ( void )
{
    if ( uartRX0Buff )
    {
        FrameCntRS232++;

        if ( FrameCntRS232 > 10 )
        {
         // memcpy(&RS232Rx.Buff[0], dmaRX0Buffs[0], (uartRX0Buff+1)*UARTRXBUFFSIZE);
          RS232Rx.Flag = 1 ;
          RS232Rx.Len = RS232Rx.Idx ;
          uartRX0Buff = 0 ;
          RS232Rx.Idx = 0 ;
        }
    }

    if ( uartRX1Buff )
    {
        FrameCntRS4851++;

        if ( FrameCntRS4851 > 20 )
        {
            RS485Rx1.Len = RS485Rx1.Idx ;
            RS485Rx1.Flag = 1 ;
            uartRX1Buff = 0 ;
        }
    }
    
    if (RS485TxEndFlag)
    {   
       RS485TxEndCnt++;
       if (RS485TxEndCnt>=9)
       {
          RS485TxEndFlag = 0 ;
          Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,14, 0 );
          Chip_UART_TXDisable(LPC_USART1);
       }
    }

}