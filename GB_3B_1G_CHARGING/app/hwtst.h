#ifndef __HWTST_H__
#define __HWTST_H__


#define TST_CLOSE    0
#define TST_485_1    1
#define TST_485_2    2
#define TST_CAN_1    3
#define TST_CAN_2    4

#define CAN1_TST_ID  0x155
#define CAN2_TST_ID  0x2aa

typedef struct
{
    uint8_t flg;         // 启动硬件测试标示
    uint8_t sts;         // 测试过程状态机

    uint8_t r485;        // 485测试结果
    uint8_t rcan;        // 485测试结果
    uint8_t rin;         // 输入测试结果
    uint16_t rtmp[2];    // 温度测试结果
    uint16_t rcurr;      // 电流测试结果
    uint8_t  fuse;       // 熔丝测试结果
} HWTST;

extern void hw_tst_init ( void );
extern void hw_tst_main ( void );

#endif

