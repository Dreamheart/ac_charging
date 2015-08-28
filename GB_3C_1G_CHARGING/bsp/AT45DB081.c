/**********************************************************
  江苏德和新能源科技有限公司
  File name:  AT45DB081.c	              // 文件名
  Author:     王辉                    // 作者
  Version:    v1.00                   // 版本信息
  Date:       2015-4-15               // 完成日期
  Description:  	              //AT45DB081的驱动程序,实现记录及事件的存储
  Others:		              // 无
  Function List:	              // void SaveRecord(uint8_t *buff)
                                         void LoadRecord(uint32_t num,uint8_t *buff)
                                         void SaveEvent(uint8_t *buff)
                                         void LoadEvent(uint32_t num,uint8_t *buff)
                                         void SaveAlarm(uint8_t *buff)
                                         void LoadAlarm(uint32_t num,uint8_t *buff)
                                         void FlashInit(void)
                                         void ErasingDataFlash(void)
                                         uint16_t GetRecordNum(void);
                                         uint16_t GetEventNum(void);
                                         uint16_t GetAlarmNum(void);
  History: 2012-09-13 更新驱动程序，自动识别FLASH型号，兼容AT45DB2562之前所有型号
**********************************************************/


#include "board.h"
/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize    0x100

/* Private define ------------------------------------------------------------*/
#define SE         0x7C  /* Sector Erase instruction */
#define PE         0x81  /* Page Erase instruction */

/* Private define ------------------------------------------------------------*/
#define WRITE      0x82  /* Write to Memory instruction */
#define READ       0xD3  /* Read from Memory instruction */
#define RDSR       0xD7  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define PE         0x81  /* Page Erase instruction */
#define BE1        0xC7  /* Bulk Erase instruction */
#define BE2        0x94  /* Bulk Erase instruction */
#define BE3        0x80  /* Bulk Erase instruction */
#define BE4        0x9A  /* Bulk Erase instruction */

#define BUSY_Flag  0x80 /* Ready/busy status flag */

#define Dummy_Byte 0xA5
#define BUFFER_1_READ 0xd1 //0xD4 
// buffer 2 read
#define BUFFER_2_READ 0xd3 //0xD6 

// buffer 1 write
#define BUFFER_1_WRITE 0x84

// buffer 2 write
#define BUFFER_2_WRITE 0x87

// buffer 1 to main memory page program with built-in erase
#define B1_TO_MM_PAGE_PROG_WITH_ERASE 0x83

// buffer 2 to main memory page program with built-in erase
#define B2_TO_MM_PAGE_PROG_WITH_ERASE 0x86

// buffer 1 to main memory page program without built-in erase
#define B1_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x88

// buffer 2 to main memory page program without built-in erase
#define B2_TO_MM_PAGE_PROG_WITHOUT_ERASE 0x89

// main memory page program through buffer 1
#define MM_PAGE_PROG_THROUGH_B1 0x82

// main memory page program through buffer 2
#define MM_PAGE_PROG_THROUGH_B2 0x85

// auto page rewrite through buffer 1
#define AUTO_PAGE_REWRITE_THROUGH_B1 0x58

// auto page rewrite through buffer 2
#define AUTO_PAGE_REWRITE_THROUGH_B2 0x59

// main memory page compare to buffer 1
#define MM_PAGE_TO_B1_COMP 0x60

// main memory page compare to buffer 2
#define MM_PAGE_TO_B2_COMP 0x61

// main memory page to buffer 1 transfer
#define MM_PAGE_TO_B1_XFER 0x53

// main memory page to buffer 2 transfer
#define MM_PAGE_TO_B2_XFER 0x55

// DataFlash status register for reading density, compare status,
// and ready/busy status
#define STATUS_REGISTER 0xD7

// main memory page read
#define MAIN_MEMORY_PAGE_READ 0x52

// erase a 528 byte page
#define PAGE_ERASE 0x81

// erase 512 pages
#define BLOCK_ERASE 0x50

#define DF_READ_BUFFER 1
#define DF_WRITE_BUFFER 0
#define DF_PAGE_SIZE 528
#define DF_READ_BUFFER 1
#define DF_WRITE_BUFFER 0

#define DF_BUFF_SIZE 264
#define NUMDATAFLASH 10


#define DF_RECORD_LEN 74
#define DF_RECORD_BASE 0
#define DF_RECORD_NUM_PER_PAGE 3 //3*74=222

#define DF_ALARM_LEN  8
#define DF_ALARM_BASE DF_RECORD_LEN*DF_RECORD_NUM_PER_PAGE
#define DF_ALARM_NUM_PER_PAGE 2  //2*8=16   222+16 =238

#define DF_EVENT_LEN  15
#define DF_EVENT_BASE DF_ALARM_BASE+DF_ALARM_LEN*DF_ALARM_NUM_PER_PAGE
#define DF_EVENT_NUM_PER_PAGE 1  //1*15=15   15+236 =251

#define RECORD_MAX 500

/*
 * 移植本软件包时需要修改以下的函数或宏 
 */
#define     SSEL_EN         (0 << 16)
#define     SSEL_DIS        (1 << 16)
#define     EOT_EN          (1 << 20)
#define     EOT_DIS         (0 << 20)
#define     EOF_EN          (1 << 21)
#define     EOF_DIS         (0 << 21)
#define     RXIGNORE_EN     (1 << 22)
#define     RXIGNORE_DIS    (0 << 22)
#define     FLEN(n)         (((n) - 1) << 24)
uint32_t RecordNum, RecordFlag; //当前记录号，记录满500条标记
uint32_t EventNum, EventFlag;
uint32_t AlarmNum, AlarmFlag;

typedef struct
{
    //AT45DB FLASH型号及规格

    unsigned int pageNumber;
    unsigned int hasBinaryPage;
    unsigned int pageSize;
    unsigned int pageOffset;
    unsigned char id;
    const char *name;

} At45Desc;

static const At45Desc at45Devices[] =
{
    {  512,  1, 264,   9, 0x0C, "AT45DB011D"},
    { 1024,  1, 264,   9, 0x14, "AT45DB021D"},
    { 2048,  1, 264,   9, 0x1C, "AT45DB041D"},
    { 4096,  1, 264,   9, 0x24, "AT45DB081D"},
    { 4096,  1, 528,  10, 0x2C, "AT45DB161D"},
    { 8192,  1, 528,  10, 0x34, "AT45DB321D"},
    { 8192,  1, 1056, 11, 0x3C, "AT45DB642D"},
    {16384,  1, 1056, 11, 0x10, "AT45DB1282"},
    {16384,  1, 2112, 12, 0x18, "AT45DB2562"},
    {32768,  1, 2112, 12, 0x20, "AT45DB5122"}
};

uint8_t DfBuff[DF_BUFF_SIZE] ; //缓冲区

uint8_t BinaryPageFlag, FlashType; //页方式，FLASH型号
uint16_t PageSize; //页大小

/************************************************************************
** 函数功能：通过硬件SPI发送一个字节到M45PE161
** 入口参数: TxData  命令或地址
** 出口参数: 接收到的数据
** 备    注: 无
************************************************************************/
uint8_t SPIx_ReadWriteByte ( LPC_SPI_T *pSPI, uint8_t TxData )
{
    uint8_t temp;
    pSPI->TXDAT = TxData;							// 发送该数据用以产生时钟,数据本身没有用处

    while ( 1 == ( pSPI->STAT&0x02 ) );			// 等待SPIF置位，即等待数据发送完毕

    temp = pSPI->STAT;								// 清除SPIF标志

    return ( uint8_t ) ( pSPI->RXDAT );					// 返回接收到的数据
}
void CE_Low ( void )
{
    ;//Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,19, 0 ) ; //片选为低
}

void CE_High ( void )
{
    ;//Chip_GPIO_SetPinState ( LPC_GPIO_PORT, 0 ,19, 1 ) ; //片选为高
}
/*********************************************************************************************************
** 函数名称: SendRecv_Byte
** 函数功能：一次传输内部的单字节帧发送与接收
** 输入参数: 数据线上发送出的数据
** 输出参数: 数据线上接收到的数据
** 返 回 值：无
*********************************************************************************************************/
uint8_t SendRecv_Byte (uint8_t ucData)
{
    while (!(LPC_SPI0->STAT & (1 << 1)));                               /* 等待发送准备就绪             */
    LPC_SPI0->TXDATCTL = FLEN(8) | EOF_EN | ucData;                     /* 8 位，帧结束                 */    
    
    while (!(LPC_SPI0->STAT & (1 << 0)));                               /* 等待接收数据完成             */
    ucData = LPC_SPI0->RXDAT;                                           /* 接收数据                     */
    
    return ucData;
}

/*********************************************************************************************************
** 函数名称: SendRecv_Start
** 函数功能：一次传输起始的单字节帧发送与接收
** 输入参数: 数据线上发送出的数据
** 输出参数: 数据线上接收到的数据
** 返 回 值：无
*********************************************************************************************************/
uint8_t SendRecv_Start (uint8_t ucData)
{
    while (!(LPC_SPI0->STAT & (1 << 1)));                               /* 等待发送准备就绪             */
    LPC_SPI0->TXDATCTL = FLEN(8) | EOF_EN | SSEL_EN | ucData;           /* 8 位，传输开始，帧结束       */    
    
    while (!(LPC_SPI0->STAT & (1 << 0)));                               /* 等待接收数据完成             */
    ucData = LPC_SPI0->RXDAT;                                           /* 接收数据                     */
    
    return ucData;
}

/*********************************************************************************************************
** 函数名称: SendRecv_Stop
** 函数功能：一次传输结束的单字节帧发送与接收
** 输入参数: 数据线上发送出的数据
** 输出参数: 数据线上接收到的数据
** 返 回 值：无
*********************************************************************************************************/
uint8_t SendRecv_Stop (uint8_t ucData)
{
    while (!(LPC_SPI0->STAT & (1 << 1)));                               /* 等待发送准备就绪             */
    LPC_SPI0->TXDATCTL = FLEN(8) | EOT_EN | ucData;                     /* 8 位，传输结束               */    
    
    while (!(LPC_SPI0->STAT & (1 << 0)));                               /* 等待接收数据完成             */
    ucData = LPC_SPI0->RXDAT;                                           /* 接收数据                     */
    
    return ucData;
}
/*************************************************************
  Function: uint8_t GetFlashStatus()
  Description: 读Flash当前状态Bit7=1:空闲 0:忙 Bit6=1:比较结果不匹配 0:匹配 Bit5-Bit2:芯片标志
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      状态寄存器
  Others:      无
*************************************************************/
uint8_t GetFlashStatus() //
{
    uint8_t  ret,ret1;
    CE_Low();
    SendRecv_Start ( STATUS_REGISTER );
    ret1 = SendRecv_Byte ( Dummy_Byte );
    ret = SendRecv_Stop ( Dummy_Byte );
    CE_High();
    return ret1;
}

/*************************************************************
  Function: void FlashInit(void)
  Description: 初始化SPI接口，初始化缓冲区
  Calls:
  Called By:   main()
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void FlashInit ( void )
{
    uint32_t i;
    uint8_t sta;
    LPC_SYSCON->SYSAHBCLKCTRL |=  (1 << 11);                            /* 初始化SPI0 AHB时钟           */
    LPC_SYSCON->PRESETCTRL    &= ~(1 << 0);                             /* 复位SPI0                     */
    LPC_SYSCON->PRESETCTRL    |=  (1 << 0);
    
    LPC_SPI0->DIV = SystemCoreClock / 1000000 - 1;                      /* 主机模式下SCK频率分频设置    */
    LPC_SPI0->DLY = (0 << 0) |                                          /* 起始插入延迟：0 * T_SCK      */
                    (0 << 4) |                                          /* 结束插入延迟：0 * T_SCK      */
                    (0 << 8) |                                          /* 帧间插入延迟：0 * T_SCK      */
                    (0 << 12);                                          /* 传输插入延迟：0 * T_SCK      */
    
    /*
     * TXCTRL 发送控制器设置：
     * TXCTRL
     *      16：传输从机选择    0：选择从机     1：取消选择从机     控制写数据前SSEL信号的有效性
     *      20：传输结束控制    0：传输不结束   1：传输结束         控制每次写数据后SSEL信号的有效性
     *      21：帧结束控制      0：帧不结束     1：帧结束           控制下次写数据前是否插入帧间延迟
     *      22：接收忽略控制    0：不忽略接收   1：忽略接收
     *   27:24：帧长度          0x0 ~ 0xF：对应 1 ~ 16 位帧长度
     */
    LPC_SPI0->TXCTRL = (0 << 16) |                                      /* 从机选择                     */
                       (0 << 20) |                                      /* 传输不结束                   */
                       (1 << 21) |                                      /* 帧结束                       */
                       (0 << 22) |                                      /* 不忽略接收                   */
                       (7 << 24);                                       /* 帧长度：8位                  */
    
    /*
     * CFG 配置寄存器设置：
     * CFG
     *      0：SPI使能位        0：SPI禁能      1：SPI使能
     *      2：模式选择位       0：SPI从机      1：SPI主机
     *      3：传输位序选择     0：标准(MSBF)   1：反转(LSBF)
     *      4：时钟相位选择     0：第1个沿捕获  1：第2个沿捕获
     *      5：时钟极性选择     0：帧间SCK为0   1：帧间SCK为1
     *      7：回写模式使能     0：禁止回写     1：使能回写
     *      8：SSEL有效极性选择 0：低电平有效   1：高电平有效
     */
    LPC_SPI0->CFG = (1 << 0) |                                          /* SPI使能                      */
                    (1 << 2) |                                          /* 主机模式                     */
                    (0 << 3) |                                          /* MSBF                         */
                    (0 << 4) |                                          /* CPHA = 1                     */
                    (0 << 5) |                                          /* CPOL = 1                     */
                    (0 << 7) |                                          /* 禁止回写                     */
                    (0 << 8);                                           /* SSEL低电平有效               */

    /* Initializing Buffer section ----------------------------------------------------------- */
    for ( i = 0; i < DF_BUFF_SIZE; i++ )
    {
        DfBuff[i] = 0 ;
    }

    sta = GetFlashStatus();

    for ( i = 0; i < NUMDATAFLASH; i++ )
    {
        if ( at45Devices[i].id == ( sta & 0x3c ) )
        {
            FlashType = i ;
        }
    }

    BinaryPageFlag = sta & 0x1 ; //0--264BYTE，1--256BYTE

    if ( BinaryPageFlag == 0 )
    {
        PageSize = at45Devices[FlashType].pageSize;
    }
    else
    {
        PageSize = ( at45Devices[FlashType].pageSize >> 8 ) << 8;
    }
}

/*************************************************************
  Function: static void df_wait_busy(void)
  Description: 空闲时返回或超时返回
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
static void df_wait_busy ( void )
{
    uint32_t i;
    uint8_t sta;
    /* Select the FLASH: Chip Select low */
    CE_Low();
    i = 500000;

    while ( 1 )
    {
        sta = GetFlashStatus();

        if ( ( sta & 0x80 ) )
        {
            break;; /* Busy in progress */
        }
    }

    /* Deselect the FLASH: Chip Select high */
    CE_High();
}

/*************************************************************
  Function: uint32_t GetDfAddress(uint32_t address)
  Description: 将物理地址转换为页地址+字节地址的格式
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      符合AT45DB系列要求的地址
  Others:      无
*************************************************************/
uint32_t GetDfAddress ( uint32_t address )
{
    uint32_t dfAddress;

    if ( !BinaryPageFlag )
    {
        dfAddress =
            ( ( address / ( PageSize ) ) << at45Devices[FlashType].pageOffset )
            + ( address % ( PageSize ) );
    }
    else
    {
        dfAddress = address;
    }

    return dfAddress;
}
//读主存储器指定页到读缓冲区(BUFFER1)
static void load_page_to_buffer ( uint32_t page, uint8_t buffer )
{
    CE_Low();

    if ( buffer == DF_READ_BUFFER )
    {
        SendRecv_Start (  MM_PAGE_TO_B1_XFER );
    }
    else
    {
        SendRecv_Start (  MM_PAGE_TO_B2_XFER );
    }

    SendRecv_Byte (  ( uint8_t ) ( page >> 16 ) );
    SendRecv_Byte (  ( uint8_t ) ( page >> 8 ) );
    SendRecv_Stop (  ( uint8_t ) ( page ) );

    CE_High();

    df_wait_busy();
}
//将写缓冲区内容写入到主存储器中指定页
static void write_page_from_buffer ( uint32_t page, uint8_t buffer )
{
    CE_Low();

    if ( buffer == DF_WRITE_BUFFER )
    {
        SendRecv_Start (  B2_TO_MM_PAGE_PROG_WITH_ERASE );
    }
    else
    {
        SendRecv_Start (  B1_TO_MM_PAGE_PROG_WITH_ERASE );
    }

    SendRecv_Byte (  ( uint8_t ) ( page >> 16 ) );
    SendRecv_Byte (  ( uint8_t ) ( page >> 8 ) );
    SendRecv_Stop (  ( uint8_t ) ( page ) );

    CE_High();

    df_wait_busy();
}
//从读缓冲区读数据
void read_buffer ( uint32_t addr, uint8_t *data, uint8_t size )
{
    uint8_t i;

    CE_Low();

    SendRecv_Start (  BUFFER_1_READ );
    SendRecv_Byte (  ( uint8_t ) ( addr >> 16 ) );
    SendRecv_Byte (  ( uint8_t ) ( addr >> 8 ) );
    SendRecv_Byte (  ( uint8_t ) addr );
    //SendRecv_Byte (  0 );

    for ( i = 0; i < size-1; i++ )
    {
        data[i] = SendRecv_Byte (  0 );
    }
    data[i] = SendRecv_Stop (  0 );
    CE_High();
}
//将数据写入写缓冲区
void write_buffer ( uint32_t addr, uint8_t *data, uint8_t size )
{
    uint8_t i;

    CE_Low();

    SendRecv_Start (  BUFFER_2_WRITE );
    SendRecv_Byte (  ( uint8_t ) ( addr >> 16 ) );
    SendRecv_Byte (  ( uint8_t ) ( addr >> 8 ) );
    SendRecv_Byte (  ( uint8_t ) addr );

    for ( i = 0; i < size-1; i++ )
    {
        SendRecv_Byte (  data[i] );
    }
    SendRecv_Stop(  data[i] );
    CE_High();
}

/*************************************************************
  Function: void FlashWrite (uint32_t addr, uint16_t len, uint8_t *buff )
  Description: 将缓冲区数据写到指定FLASH地址
  Calls:
  Called By:
  Input:       addr--指定地址 len--长度 buff--记录缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void FlashWrite ( uint32_t addr, uint16_t len, uint8_t *buff )
{
    uint32_t address;
    address = GetDfAddress ( addr );
    load_page_to_buffer ( address, DF_WRITE_BUFFER );
    write_buffer ( address, buff, len );
    write_page_from_buffer ( address, DF_WRITE_BUFFER );
}
/*************************************************************
  Function: void FlashRead (uint32_t addr, uint16_t len, uint8_t *buff )
  Description: 将指定FLASH地址内容读到缓冲区
  Calls:
  Called By:
  Input:       addr--指定地址 len--长度 buff--记录缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void FlashRead ( uint32_t addr, uint16_t len, uint8_t *buff )
{
    uint32_t address;
    address = GetDfAddress ( addr );
    load_page_to_buffer ( address, DF_READ_BUFFER );
    read_buffer ( address, buff, len );
}


/*************************************************************
  Function: void SaveRecord(uint32_t num,uint8_t *buff)
  Description: 存储记录到AT45DB081
  Calls:
  Called By:
  Input:       num--记录编号  buff--记录缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void SaveRecord ( uint8_t *buff )
{
    uint32_t addr , temp;

    if ( RecordNum >= 500 )
    {
        RecordNum %= 500 ;

        if ( RecordFlag == 0 )
        {
            RecordFlag = 1 ;//满500条标记
        }
    }

    //NumPerPage = (PageSize/(264-8*BinaryPageFlag))*DF_RECORD_NUM_PER_PAGE;
    temp = RecordNum / DF_RECORD_NUM_PER_PAGE ;
    addr = temp * ( 264 - 8 * BinaryPageFlag ) ;
    temp = DF_RECORD_BASE + ( RecordNum % DF_RECORD_NUM_PER_PAGE ) * DF_RECORD_LEN ;
    addr += temp ;
    FlashWrite ( addr, DF_RECORD_LEN, buff ); //写数据到缓冲区
    RecordNum++;
}

/*************************************************************
  Function: void LoadRecord(uint32_t num,uint8_t *buff)
  Description: 读AT45DB081中的记录
  Calls:
  Called By:
  Input:       num--记录编号  buff--记录缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void LoadRecord ( uint32_t num, uint8_t *buff )
{
    int32_t tmp ;
    uint32_t addr, temp;

    if ( RecordNum == 0 )
    {
        return;
    }

    tmp = RecordNum - num ; //取记录顺序为倒序

    if ( tmp < 0 )
    {
        tmp += 500 ;//循环，从尾部取
    }

    temp = tmp / DF_RECORD_NUM_PER_PAGE ;
    addr = temp * ( 264 - 8 * BinaryPageFlag ) ;
    temp = DF_RECORD_BASE + ( tmp % DF_RECORD_NUM_PER_PAGE ) * DF_RECORD_LEN ;
    addr += temp ;
    FlashRead ( addr, DF_RECORD_LEN, buff );
}

/*************************************************************
  Function: void SaveEvent(uint32_t num,uint8_t *buff)
  Description: 存储事件到AT45DB081
  Calls:
  Called By:
  Input:       num--事件编号  buff--事件缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void SaveEvent (uint8_t num, uint8_t *buff,uint8_t len )
{
    __disable_irq(); 
    FlashWrite ( num, len, buff ); //写数据到缓冲区
    __enable_irq();
}

/*************************************************************
  Function: void LoadEvent(uint32_t num,uint8_t *buff)
  Description: 读AT45DB081中的事件
  Calls:
  Called By:
  Input:       num--事件编号  buff--事件缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void LoadEvent ( uint32_t num, uint8_t *buff ,uint8_t len )
{
    FlashRead ( num, len, buff );
}

/*************************************************************
  Function: void SaveAlarm(uint32_t num,uint8_t *buff)
  Description: 存储报警事件到AT45DB081
  Calls:
  Called By:
  Input:       num--事件编号  buff--事件缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void SaveAlarm ( uint8_t *buff )
{
    uint32_t addr, temp;

    if ( AlarmNum >= 500 )
    {
        AlarmNum %= 500 ;

        if ( AlarmFlag == 0 )
        {
            AlarmFlag = 1 ;//满500条标记
        }
    }

    temp = AlarmNum / DF_ALARM_NUM_PER_PAGE ;
    addr = temp * ( 264 - 8 * BinaryPageFlag ) ;
    temp = DF_ALARM_BASE + ( AlarmNum % DF_ALARM_NUM_PER_PAGE ) * DF_ALARM_LEN ;
    addr += temp ;
    FlashWrite ( addr, DF_ALARM_LEN, buff );
    AlarmNum++;
}

/*************************************************************
  Function: void LoadAlarm(uint32_t num,uint8_t *buff)
  Description: 读AT45DB081中的报警事件
  Calls:
  Called By:
  Input:       num--事件编号  buff--事件缓冲区首地址
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void LoadAlarm ( uint32_t num, uint8_t *buff )
{
    int32_t tmp ;
    uint32_t addr, temp;

    if ( AlarmNum == 0 )
    {
        return;
    }

    tmp = AlarmNum - num ; //取记录顺序为倒序

    if ( tmp < 0 )
    {
        tmp += 500 ;//循环，从尾部取
    }

    temp = tmp / DF_ALARM_NUM_PER_PAGE ;
    addr = temp * ( 264 - 8 * BinaryPageFlag ) ;
    temp = DF_ALARM_BASE + ( tmp % DF_ALARM_NUM_PER_PAGE ) * DF_ALARM_LEN ;
    addr += temp ;
    FlashRead ( addr, DF_ALARM_LEN, buff );
}

/*************************************************************
  Function: uint16_t GetRecordNum(void)
  Description: 获取当前记录条数
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      RecordNum
  Others:      无
*************************************************************/
uint16_t GetRecordNum ( void )
{
    if ( RecordFlag )
    {
        return 500 ;
    }
    else
    {
        return RecordNum;
    }
}

/*************************************************************
  Function: uint16_t GetEventNum(void)
  Description: 获取当前事件记录条数
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      EventNum
  Others:      无
*************************************************************/
uint16_t GetEventNum ( void )
{
    if ( EventFlag )
    {
        return 500 ;
    }
    else
    {
        return EventNum;
    }
}

/*************************************************************
  Function: uint16_t GetAlarmNum(void)
  Description: 获取当前报警记录条数
  Calls:
  Called By:
  Input:       无
  Output:      无
  Return:      EventNum
  Others:      无
*************************************************************/
uint16_t GetAlarmNum ( void )
{
    if ( AlarmFlag )
    {
        return 500 ;
    }
    else
    {
        return AlarmNum;
    }

}
void ClearRecord ( void )
{
    AlarmNum = 0 ;
    AlarmFlag = 0 ;
    EventNum = 0 ;
    EventFlag = 0 ;
    RecordNum = 0 ;
    RecordFlag = 0 ;
}
void df161_test ( void )
{
    uint32_t i, j;

    for ( j = 0; j < 10; j++ )
    {
        for ( i = 0; i < 100; i++ )
        {
            DfBuff[i] = i;
        }

        RecordNum = 101 ;
        SaveRecord ( DfBuff );
        LoadRecord ( 1, DfBuff );
    }
}

