#ifndef __RTC_H__
#define __RTC_H__

#include "lpc177x_8x_rtc.h"


extern RTC_TIME_Type RtcCurTime;

extern RTC_TIME_Type RtcGetTime ( void );                                // 读取RTC时间
extern void RtcSetTime ( RTC_TIME_Type time );                           // 设置RTC时间
extern void RtcInit ( void );                                             // 初始化时钟

#endif

