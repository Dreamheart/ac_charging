#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "lpc177x_8x_rtc.h"

#define SPOTOUTPUT  0x01
#define SPOTGREEN   0x02
#define SPOTYELLOW  0x03
#define SPOTRESVED  0x04

/*
配置参数数据结构
*/
typedef struct
{
    uint8_t   id[4];                                               // 充电桩地址
    uint16_t   price;                                                    // 电价
} PARACONFIG;

#pragma pack()
//------------------------------------------------------------------------------
extern volatile unsigned long SysTickCnt;
extern volatile unsigned long SysTime;
extern RTC_TIME_Type CurTime ;

#define   SWAP(a)           ((a)>>8 | (a)<<8)

#endif

