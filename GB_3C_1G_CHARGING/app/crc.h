#ifndef __CRC_H__
#define __CRC_H__

#define  Hi  0                              //用于选择CRC函数输出的高低位
#define  Lo  1

#include "board.h"

uint8_t CRC16( uint8_t * puchMsg,short usDataLen,uint8_t byte_flag);        // 得到CRC的高字节 (byte_flag == 1)

#endif
