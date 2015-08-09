#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <string.h>

#define S3V3    0xFFF
#define RES1    30000

typedef struct
{
    uint32_t current;
    uint32_t temp[2];
    uint8_t  fuse;
    uint8_t  linkv;
} ANALOGVALUE;

extern ANALOGVALUE g_sAnalogValue;
extern uint8_t GetBatSta(void);
extern uint32_t dealcurrent ( void );
extern uint8_t dealfuse ( void );
extern uint16_t dealtemp ( uint8_t num );
extern uint8_t GetAnalogSta(void);

#endif


