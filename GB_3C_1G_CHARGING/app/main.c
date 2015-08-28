/*
 * @brief General Purpose Input/Output example
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
#include "Dio.h"
#include "uart.h"
#include "globals.h"
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Number of tickrate per second */
#define TICKRATE_HZ (1000)

#if CPU822
#define OUTPUT_MASK       0x300c0c2
#define INPUT_MASK        0xc010400
#else
#define OUTPUT_MASK       0x700c0c2
#define INPUT_MASK        0x8010c00
#endif


//#define OUTPUT_MASK       0x30880c2
//#define INPUT_MASK       0xc010400

#define SCT_PWM            LPC_SCT
#define SCT_PWM_PIN_LED    0        /* COUT0 [index 2] Controls LED */
#define SCT_PWM_LED        1        /* Index of LED PWM */
#define SCT_PWM_RATE   1000        /* PWM frequency 1 KHz */

extern void SetupUART(void);
extern void FlashInit ( void );
extern void SetupADC(void);
extern void StartADC(void);
extern void ReadADC(void);
extern void MeterDeal ( void );
extern void df161_test ( void );
extern uint8_t LoadParameter ( void );
uint8_t EStopFlag,ParaError;
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
volatile unsigned long SysTickCnt;
uint8_t WdFlag;
PARACONFIG g_sParaConfig;
uint32_t ResetCnt;
extern uint32_t HeartBeatTime;
/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
        SysTickCnt++;

        if ( ! ( SysTickCnt % 500 ) )
        {
          
            if ( WdFlag )               //喂狗
            {
                SetOutput ( LED1 );
                WdFlag = 0 ;
            }
            else
            {
                ClrOutput ( LED1 );
                WdFlag = 1 ;
            }
        }
        StartADC();
       // RdMeterDly++;
       // MeterOnlineCnt++;
        //读输入端口
        InputCheck();
        //判断UART接收帧是否结束
        FrameIsEnd();
                
        if (ResetCnt>=1) ResetCnt--;
        else HeartBeatTime = 27000;
}

void IOCON_Init() {
       /* Enable IOCON clock */
       Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

       /* Pin I/O Configuration */
       /*LPC_IOCON->PIO0[IOCON_PIO0]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO1]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO2]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO3]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO4]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO5]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO6]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO7]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO8]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO9]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO10]= 0x80; */
       /*LPC_IOCON->PIO0[IOCON_PIO11]= 0x80; */
       /*LPC_IOCON->PIO0[IOCON_PIO12]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO13]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO14]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO15]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO16]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO17]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO18]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO19]= 0x90; */
       LPC_IOCON->PIO0[IOCON_PIO20]= 0x00; 
       LPC_IOCON->PIO0[IOCON_PIO21]= 0x00; 
       /*LPC_IOCON->PIO0[IOCON_PIO22]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO23]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO24]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO25]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO26]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO27]= 0x90; */
       /*LPC_IOCON->PIO0[IOCON_PIO28]= 0x90; */

}

void InputMux_Init()
{
    LPC_INMUX->SCT0_INMUX[0] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[1] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[2] = 15;    /*  */
    LPC_INMUX->SCT0_INMUX[3] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[0] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[1] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[2] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[3] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[4] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[5] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[6] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[7] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[8] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[9] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[10] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[11] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[12] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[13] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[14] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[15] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[16] = 15;    /*  */
    LPC_DMATRIGMUX->DMA_ITRIG_INMUX[17] = 15;    /*  */
    LPC_INMUX->DMA_INMUX_INMUX[0] = 31;    /*  */
    LPC_INMUX->DMA_INMUX_INMUX[1] = 31;    /*  */

}

void SwitchMatrix_Init()
{
       /* Enable the clock to the Switch Matrix */
       Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

       /* Pin Assign 8 bit Configuration */
       /* U0_TXD */
       /* U0_RXD */
       LPC_SWM->PINASSIGN[0] = 0xffff0004UL;
       /* U1_TXD */
       /* U1_RXD */
       LPC_SWM->PINASSIGN[1] = 0xff1617ffUL;
       /* SPI0_SCK */
       LPC_SWM->PINASSIGN[3] = 0x12ffffffUL;
       /* SPI0_MOSI */
       /* SPI0_MISO */
       /* SPI0_SSEL0 */
       LPC_SWM->PINASSIGN[4] = 0xff130d11UL;
       /* SCT_OUT0 */
       LPC_SWM->PINASSIGN[7] = 0x14ffffffUL;

       /* Pin Assign 1 bit Configuration */
       /* SWCLK */
       /* SWDIO */
       /* RESET */
       /* ADC_5 */
       LPC_SWM->PINENABLE0 = 0xfffbfecfUL;

}

#define DUTYCYCLE_LOW_POWER		25
#define DUTYCYCLE_HIGH_POWER	50
unsigned int g_pwm_percyclecounter;
unsigned int g_pwm_oncounter;
void SetupPWM(unsigned char dutycycle)
{
	Chip_SCTPWM_Stop(SCT_PWM); 
	/* Initialize the SCT as PWM and set frequency */
	Chip_SCTPWM_Init(SCT_PWM);
	Chip_SCTPWM_SetRate(SCT_PWM, SCT_PWM_RATE);

	/* Use SCT0_OUT0 pin */
	Chip_SCTPWM_SetOutPin(SCT_PWM, SCT_PWM_LED, SCT_PWM_PIN_LED);

	g_pwm_percyclecounter = Chip_SCTPWM_GetTicksPerCycle(SCT_PWM);
	
	g_pwm_oncounter = g_pwm_percyclecounter*dutycycle/100;
	
	/* Start with 0% duty cycle */
	Chip_SCTPWM_SetDutyCycle(SCT_PWM, SCT_PWM_LED, g_pwm_oncounter);
	Chip_SCTPWM_Start(SCT_PWM);  

	Chip_SCTPWM_Stop(SCT_PWM); 
}

/**
 * @brief	Main program body
 * @return	Does not return
 */
 unsigned int g_inputsta;
 unsigned int g_lastinputsta;
void main(void) {
	uint32_t temp;
//__disable_irq();

//          SCB->RESERVED0 = 0x2000;    //终端向量 重映射

//		  __enable_irq();
	/* Generic Initialization */
	SystemCoreClockUpdate();
        SwitchMatrix_Init();
        IOCON_Init();
        InputMux_Init();
       // FlashInit();     
        
        SetupUART();
        HeartBeatTime = 10000;
        ResetCnt = 120000;
        SetupPWM(DUTYCYCLE_LOW_POWER);
        SetupADC();
        	/* Set port 0 pins 0-3 (0-1 on 824) to the output direction*/
	Chip_GPIO_SetPortDIROutput(LPC_GPIO_PORT, 0, OUTPUT_MASK);
        Chip_GPIO_SetPortDIRInput(LPC_GPIO_PORT, 0, INPUT_MASK);
	/* Set GPIO port mask value to make sure only port 0
	   pins 0-3 (0-1 on 824) are active during state change */
	Chip_GPIO_SetPortMask(LPC_GPIO_PORT, 0, ~(OUTPUT_MASK));
        
  //      FlashInit();
	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);
        
        Chip_SCTPWM_Start(SCT_PWM);//启动PWM输出
        //Chip_SCTPWM_Stop(SCT_PWM);//停止PWM输出

		g_inputsta = GetInputStatus(1);
		g_lastinputsta = g_inputsta;

	while(1)
		{
			
			if(g_inputsta)
				{
					SetupPWM(DUTYCYCLE_HIGH_POWER);
					Chip_SCTPWM_Start(SCT_PWM);
				}
			else
				{
					SetupPWM(DUTYCYCLE_LOW_POWER);
					Chip_SCTPWM_Start(SCT_PWM);
				}
			g_lastinputsta = g_inputsta;
			while(1)
				{
					g_inputsta = GetInputStatus(1);
					if(g_inputsta != g_lastinputsta)
						break;
				}
		
		}
        //__enable_irq();
	//while (1) 
        {
    //        ReadADC();
    //        MeterDeal();
     //       GPRS_main();
    //        Charging_main();
	}
}
