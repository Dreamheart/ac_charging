
#include "lpc177x_8x_rtc.h"


/************************** PRIVATE FUNCTION *************************/
void RTC_IRQHandler ( void );
RTC_TIME_Type RtcCurTime;

/*************************************************************
  Function: void RTC_IRQHandler(void)
  Description: RTC中断函数
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void RTC_IRQHandler ( void )
{
    uint32_t secval;

    /* This is increment counter interrupt*/
    if ( RTC_GetIntPending ( LPC_RTC, RTC_INT_COUNTER_INCREASE ) )
    {
        secval = RTC_GetTime ( LPC_RTC, RTC_TIMETYPE_SECOND );

        // Clear pending interrupt
        RTC_ClearIntPending ( LPC_RTC, RTC_INT_COUNTER_INCREASE );
    }

    /* Continue to check the Alarm match*/
    if ( RTC_GetIntPending ( LPC_RTC, RTC_INT_ALARM ) )
    {
        // Clear pending interrupt
        RTC_ClearIntPending ( LPC_RTC, RTC_INT_ALARM );
    }
}

/*************************************************************
  Function: RTC_TIME_Type RtcGetTime(void)
  Description: 读取RTC时间
  Calls:       RTC_GetFullTime
  Called By:
  Input:       无
  Output:      无
  Return:      RtcCurTime
  Others:      无
*************************************************************/
RTC_TIME_Type RtcGetTime ( void )
{
    RTC_GetFullTime ( LPC_RTC, &RtcCurTime );
    return RtcCurTime;
}

/*************************************************************
  Function: void RtcSetTime(RTC_TIME_Type time)
  Description: 设置RTC时间
  Calls:       RTC_SetTime
  Called By:
  Input:       time--要设置的时间
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void RtcSetTime ( RTC_TIME_Type time )
{
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_SECOND, time.SEC );
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_MINUTE, time.MIN );
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_HOUR, time.HOUR );
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_MONTH, time.MONTH );
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_YEAR, time.YEAR );
    RTC_SetTime ( LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, time.DOM );
}

/*************************************************************
  Function: void RtcInit(void)
  Description: 初始化RTC时钟
  Calls:
  Called By:   main()
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void RtcInit ( void )
{
    /* RTC Block section ------------------------------------------------------ */
    // Init RTC module
    RTC_Init ( LPC_RTC );

    /* Disable RTC interrupt */
    NVIC_DisableIRQ ( RTC_IRQn );
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority ( RTC_IRQn, ( ( 0x01 << 3 ) | 0x01 ) );

    /* Enable rtc (starts increase the tick counter and second counter register) */
    RTC_ResetClockTickCounter ( LPC_RTC );
    RTC_Cmd ( LPC_RTC, ENABLE );
    RTC_CalibCounterCmd ( LPC_RTC, DISABLE );

    /* Set ALARM time for second */
    //RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_SECOND, 10);

    // Get and print current time
    //RTC_GetFullTime (LPC_RTC, &RTCFullTime);



    /* Set the CIIR for second counter interrupt*/
    //RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);
    /* Set the AMR for 10s match alarm interrupt */
    //RTC_AlarmIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);

    /* Enable RTC interrupt */
    //NVIC_EnableIRQ(RTC_IRQn);
}


