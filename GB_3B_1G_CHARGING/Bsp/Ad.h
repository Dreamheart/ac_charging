#ifndef __AD_H__
#define __AD_H__

#include <string.h>

uint32_t GetCurrent ( uint8_t num );                          // 获取当前电流值
uint32_t GetFuseSta ( uint8_t num );                        // 获取当前熔丝状态
uint32_t GetTemp ( uint8_t num );                               // 获取当前温度

extern void AdcInit ( void );
extern void adc_main ( void );

#endif