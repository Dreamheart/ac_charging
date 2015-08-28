/*
 * @brief LPC82x ADC example
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
#include <stdio.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t RdAdCnt;
uint32_t ADCResult[16];
#define BOARD_ADC_CH 5

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void StartADC(void)
{
    LPC_ADC->SEQ_CTRL[ADC_SEQA_IDX] = (0x1 << 5) |              /* 将通道0使能在SQEA中          */
                                      (0x1 << 18);     
    LPC_ADC->SEQ_CTRL[ADC_SEQA_IDX] |= (0x1 << 31);                     /* 使能SQEA                     */     
    LPC_ADC->SEQ_CTRL[ADC_SEQA_IDX] |= (0x1 << 26) ;                    /* 软件方式启动ADC转换          */

}


void ReadADC(void)
{
      RdAdCnt &= 0x0f;
      if((LPC_ADC->DR[5] & (0x1 << 31)))/* 等待DATA寄存器数据可读        */
      {                          
        ADCResult[RdAdCnt++] = (LPC_ADC->DR[5] >> 4 ) & 0xFFF;	
      }
}

uint8_t GetAnalogSta(void)
{
      uint32_t temp,i;
      
      temp = 0 ;
      for (i=0;i<16;i++)
      {
         temp += ADCResult[i];
      }
      temp /= 16 ;//滤波
      
      temp *=12000;//12V
      temp /= 4096;
 
       if ((temp>=780)&&(temp<=1120))
      {
         return 6;
      }
      if ((temp>=1260)&&(temp<=1650))
      {
         return 9;
      }
      if ((temp>=1750))
      {
         return 12;
      }  
      
       
  /*    
       if ((temp>=380)&&(temp<=1140))
      {
         return 6;
      }
      if ((temp>=1250)&&(temp<=1700))
      {
         return 9;
      }
      if ((temp>=1710))
      {
         return 12;
      }  
      */
      
 //  return 6;
   
   
}
/*********************************************************************************************************
** Function name:		ADCInit
**
** Descriptions:		ADC初始化函数
**
** parameters:			None
** Returned value:	None
** 
*********************************************************************************************************/
void ADCInit (void)
{
   // ADCValue = 0x0; 
    
    LPC_SYSCON->PDRUNCFG &= ~(0x1 << 4 );                               /* 给ADC模块上电                */                    
    LPC_SYSCON->SYSAHBCLKCTRL |= ( 0x1 << 24 );                         /* 使能ADC时钟                  */

   // LPC_SWM->PINENABLE0 &= ~(0x1 << 13);                                /* ADC_0通道使能在PIO0_7        */
   // LPC_IOCON->PIO0[15] = 0;                                            /* 模拟引脚PIO0_7 配置为高阻态  */
    
    LPC_ADC->CTRL = (0x0 << 0)|                                         /* 0分频                        */
                    (0x0 << 10);                                        /* 关闭低功耗模式               */

    LPC_ADC->SEQ_CTRL[ADC_SEQA_IDX] = (0x1 << 5) |                      /* 将通道0使能在SQEA中          */
                                      (0x1 << 18);                      /* 上升沿触发                   */                                    
    
    LPC_ADC->SEQ_CTRL[ADC_SEQA_IDX] |= (0x1 << 31);                     /* 使能SEQA                     */       
       
  return;
}
/**
 * @brief	main routine for ADC example
 * @return	Function should not exit
 */
void SetupADC(void)
{
	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC, 0);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC))) {}

	/* Setup for maximum ADC clock rate using sycnchronous clocking */
	Chip_ADC_SetClockRate(LPC_ADC, ADC_MAX_SAMPLE_RATE);
        ADCInit(); 
        return;
}
