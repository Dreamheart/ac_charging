#ifndef __PEER_H__
#define __PEER_H__

#include "lpc177x_8x_uart.h"


#define   CMD_SYNCTM        0

#define   CMD_VERS          5

#define   CMD_SETID         10

#define   CMD_SETV          15
#define   CMD_SETI          16
#define   CMD_OUTV          17
#define   CMD_OUTI          18
#define   CMD_TMP           19
#define   CMD_STS           20
#define   CMD_MEASV         21
#define   CMD_MEASI         22
#define   CMD_MEASTMP       23

#define   CMD_CLEAR         50

#define   CMD_CTRLV         100
#define   CMD_CTRLI         101
#define   CMD_CTRLMODE      102

#define   CMD_GRPS          110
#define   CMD_ID            111
#define   CMD_CNT           112
#define   CMD_MAXV          113
#define   CMD_MAXI          114
#define   CMD_MODE          115
#define   CMD_MAXVS         117
#define   CMD_MINVS         118
#define   CMD_LIMITV        119
#define   CMD_FLOATI        120
#define   CMD_EVENI         121
#define   CMD_IFETOF        122
#define   CMD_IFFTOE        123
#define   CMD_DISCHRGI      124
#define   CMD_LEFTT         125

#define   CMD_ALERMN        200
#define   CMD_PARAN         201
#define   CMD_RECORDN       202

#define   CMD_HWTST         250

#define   CMD_SNID          300

#define   CMD_ALARM         500
#define   CMD_PARA          3000
#define   CMD_RECORD        8000

#define   BIAS_CNTID         0
#define   BIAS_MODE          (BIAS_CNTID+1)
#define   BIAS_MAXV          (BIAS_MODE+1)
#define   BIAS_MAXI          (BIAS_MAXV+1)
#define   BIAS_MAXVS         (BIAS_MAXI+1)
#define   BIAS_MINVS         (BIAS_MAXVS+1)
#define   BIAS_LIMITV        (BIAS_MINVS+1)
#define   BIAS_FLOATI        (BIAS_LIMITV+1)
#define   BIAS_EVENI         (BIAS_FLOATI+1)
#define   BIAS_IFETOF        (BIAS_EVENI+1)
#define   BIAS_IFFTOE        (BIAS_IFETOF+1)
#define   BIAS_DISCHRGI      (BIAS_IFFTOE+1)
#define   BIAS_LEFTT         (BIAS_DISCHRGI+1)
//ALARM
#define   BIAS_LOWOUTV        1
#define   BIAS_OVEROUTV       (BIAS_LOWOUTV+1)
#define   BIAS_OVERTMP        (BIAS_OVEROUTV+1)
#define   BIAS_COMERR         (BIAS_OVERTMP+1)
#define   BIAS_HWERR          (BIAS_COMERR+1)
#define   BIAS_LOWINV         (BIAS_HWERR+1)
#define   BIAS_OVERINV        (BIAS_LOWINV+1)
#define   BIAS_OVEROUTI       (BIAS_OVERINV+1)
#define   BIAS_BALANCEERR     (BIAS_OVEROUTI+1)

#define   TRIGGER_RECOVER     0
#define   TRIGGER_OCCUR       1

//------------------------------------------------------------------------------
extern PARACONFIG   g_sParaConfig;
extern void peer_init ( void );
extern void peer_main ( void );
extern void Start ( void );
extern void Stop ( void );
extern void QR_code_Verification ( void );
extern void Account_Verification ( void );

#endif
