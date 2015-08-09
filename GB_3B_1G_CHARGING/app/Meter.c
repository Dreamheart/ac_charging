

/**********************************************************
德和新能源科技有限公司
File name:  hwtst.c	              // 文件名
Author:     韩忠华                   // 作者
Version:    v1.00                   // 版本信息
Date:       2015-1-23              // 完成日期
Description:  	              // 使用232串口与GPRS模块通信，间接实现与综合平台的通信
Others:		              // 
Function List:	              //
History:                        //
**********************************************************/

#include "lpc177x_8x_crc.h"
#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_gpdma.h"
#include "lpc177x_8x_pinsel.h"
#include "Bsp_inc.h"
#include "app_inc.h"

//校验码问题
extern UART_BUFF RS485Rx2, RS485Tx2;
uint8_t MeterData[4], MeterAddr[6];
uint8_t MeterErr, MeterCommErr, MeterAddrFlag;
uint8_t Position68;
uint32_t RdMeterTicks, RdMeterTicks1, RdMeterTicks2;       //读电量，读电压，读电流时间周期
uint8_t Meter_counter;
uint16_t MeterOnlineCnt;
uint32_t CurKwh;
uint32_t Cur_0_BeginTime,Cur_0_Time;
uint8_t RelaySta2;      //4852接受状态指示灯，交替开关
extern uint16_t output_vol, output_cur;         //输出电压电流，
                                                //电流电表反馈三位小数，下文做特别处理，舍掉最后一位，系统中按照两位小数传输
                                                //电压电表反馈一位小数，下文做特别处理，小数第二位补0，系统中按照两位小数传输


/*************************************************************
Function: uint32_t GetCurKwh ( void )
Description: 获取当前电量
Calls:       无
Called By:   
Input:       
Output:      无
Return:      CurKwh 当前正向有功总电量
Others:      无
*************************************************************/
uint32_t GetCurKwh ( void )
{
    return CurKwh;
}
/*************************************************************
Function: uint8_t CSCalc(UART_BUFF f)
Description: 计算从站接受应答的校验码
Calls:       无
Called By:   void MeterGetRs232();void MeterCalc(void);
Input:       UART_BUFF f 应答帧数据
Output:      无
Return:      uint8_t cs 校验码
Others:      无
*************************************************************/
uint8_t CSCalc ( UART_BUFF f )
{
    uint8_t cs = 0x00, i, len, j;
    len = f.Len;
    Position68 = 0 ;

    for ( i = 0; i < len; i++ )
    {
        if ( f.Buff[i] == 0x68 )
        {
            Position68 = i ;

            for ( j = i; j < len - 2; j++ )
            {
                cs += f.Buff[j];
            }

            break;
        }
    }

    return cs;
}

/*************************************************************
Function: void MeterDeal(void)
Description: 读取电表表号
Calls:       无
Called By:
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void ReadMeterAddr ( void ) //读取电表表号
{

}
/*************************************************************
Function: void BcdToHex(uint32_t *dat ,uint8_t *ptr)
Description: BCD数据转换为16进制
Calls:       无
Called By:   
Input:       *dat--HEX数据指针 *ptr--BCD数据指针
Output:      无
Return:      无
Others:      无
*************************************************************/
void BcdToHex(uint32_t *dat ,uint8_t *ptr)
{
    uint32_t tmp ;
    uint8_t tmp1;
    tmp = 0 ;
    tmp1 = *ptr ;
    tmp = (tmp1>>4)*10 ;
    tmp1 &= 0x0f;
    tmp = tmp + tmp1 ;
    
    tmp1 = *(ptr+1);
    tmp += (tmp1>>4)*1000 ;
    tmp1 &= 0x0f ;
    tmp += tmp1*100;
    
    tmp1 = *(ptr+2);
    tmp += (tmp1>>4)*100000 ;
    tmp1 &=0x0f ;
    tmp += tmp1*10000 ;
    
    tmp1 = *(ptr+3);
    tmp += (tmp1>>4)*10000000 ;
    tmp1 &= 0x0f;
    tmp += tmp1*1000000 ;
    *dat = tmp ;    
}
/*************************************************************
Function: void MeterGetRs232()
Description: 从站应答帧
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void GetMeterMsg ( void ) //从站应答帧
{

    uint8_t  len, i, j, CS;
    uint8_t tmp_[4];
    uint8_t temp;
    uint32_t tmp,tmp1,tmp2;
    if ( GetRs485Ch2Sta() == 1 ) //帧接收结束标记
    {
        //接收状态指示灯
        RelaySta2 = ~ RelaySta2 ;
        temp = RelaySta2;
        if ( temp & 0x1 )
        {
            SetOutput ( LED2 );
        }
        else
        {
            ClrOutput ( LED2 );
        }
        //
        MeterOnlineCnt = 1;
        len = RS485Rx2.Len;
        CS = CSCalc ( RS485Rx2 );
        RS485Rx2.Flag = 0 ;
        RS485Rx2.Len = 0 ;
        RS485Rx2.Idx = 0 ;        

        if ( CS == RS485Rx2.Buff[len-2] )
        {
            if ( RS485Rx2.Buff[8+Position68] == 0x91 ) //读取正向有功总电量,正常应答
            {
                switch (RS485Rx2.Buff[9 + Position68])
                {
                case 0x06:
                    {
                        output_vol = 0;
                        for ( i = 14 + Position68, j = 0; j < 2; i++, j++ )
                        {
                            tmp_[j] = RS485Rx2.Buff[i] - 0x33;
                        }
                        tmp = ((tmp_[0] >> 4) & 0x0f) * 100 + (tmp_[0] & 0x0f) *10;
                        tmp1 = ((tmp_[1] >> 4) & 0x0f) * 10000 + (tmp_[1] & 0x0f) *1000;
                        output_vol = tmp + tmp1 ;
                        break;
                    }
                case 0x07:
                    {
                        output_cur = 0;
                        for ( i = 14 + Position68, j = 0; j < 3; i++, j++ )
                        {
                            tmp_[j] = RS485Rx2.Buff[i] - 0x33;
                        }
                        tmp = ((tmp_[0] >> 4) & 0x0f) ;
                        tmp1 = ((tmp_[1] >> 4) & 0x0f) * 100 + (tmp_[1] & 0x0f) *10;
                        tmp2 = ((tmp_[2] >> 4) & 0x0f) * 10000 + (tmp_[2] & 0x0f) *1000;
                        output_cur = tmp + tmp1 + tmp2; 
                        if (output_cur > 100)           //电流大于1A
                        {
                           Cur_0_BeginTime = SysTickCnt; 
                        }
                        break;
                    }
                case 0x08://读出电表数
                    {
                        for ( i = 14 + Position68, j = 0; j < 4; i++, j++ )
                        {
                            MeterData[j] = RS485Rx2.Buff[i] - 0x33;
                        }
                        BcdToHex ( &CurKwh, MeterData ); //电表返回BCD码,转为HEX    
                        break;
                    }
                }
            }
            else if ( RS485Rx2.Buff[8+Position68] == 0xD1 ) //读电表数，从站异常应答
            {
                MeterErr = RS485Rx2.Buff[len-3]; //读取错误信息字
            }
        }
    }

}

/*************************************************************
Function: void MeterCalc(void)
Description: 读取正向有功总电量
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void MeterCalc ( void ) //读取正向有功总电量 68 AA AA AA AA AA AA 68 11 04 33 33 34 33 AE 16
{

    uint8_t CS;
    RS485Tx2.Buff[0] = 0xFE; //前导字节
    RS485Tx2.Buff[1] = 0xFE;
    RS485Tx2.Buff[2] = 0xFE;
    RS485Tx2.Buff[3] = 0xFE;
    RS485Tx2.Buff[4] = 0x68; //帧起始符

    RS485Tx2.Buff[5] = 0xaa;//MeterAddr[0];//地址域
    RS485Tx2.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx2.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx2.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx2.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx2.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx2.Buff[11] = 0x68; //帧起始符
    RS485Tx2.Buff[12] = 0x11; //控制码
    RS485Tx2.Buff[13] = 0x04; //数据域长度
    RS485Tx2.Buff[14] = 0x33; //数据标识码
    RS485Tx2.Buff[15] = 0x33;
    RS485Tx2.Buff[16] = 0x34;
    RS485Tx2.Buff[17] = 0x33;

    RS485Tx2.Len = 20;//发送长度
    CS = CSCalc ( RS485Tx2 );
    RS485Tx2.Buff[18] = CS; //校验码
    RS485Tx2.Buff[19] = 0x16; //结束符

    RS485Ch2SendMsg();//读取电表总电量请求帧

}

/*************************************************************
Function: void MeterVol(void)
Description: 读电压
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void MeterVol ( void ) //读取正向有功总电量 68 AA AA AA AA AA AA 68 11 04 33 34 34 35 B1 16
{

    uint8_t CS;
    RS485Tx2.Buff[0] = 0xFE; //前导字节
    RS485Tx2.Buff[1] = 0xFE;
    RS485Tx2.Buff[2] = 0xFE;
    RS485Tx2.Buff[3] = 0xFE;
    RS485Tx2.Buff[4] = 0x68; //帧起始符

    RS485Tx2.Buff[5] = 0xaa;//MeterAddr[0];//地址域
    RS485Tx2.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx2.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx2.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx2.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx2.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx2.Buff[11] = 0x68; //帧起始符
    RS485Tx2.Buff[12] = 0x11; //控制码
    RS485Tx2.Buff[13] = 0x04; //数据域长度
    RS485Tx2.Buff[14] = 0x33; //数据标识码
    RS485Tx2.Buff[15] = 0x34;
    RS485Tx2.Buff[16] = 0x34;
    RS485Tx2.Buff[17] = 0x35;

    RS485Tx2.Len = 20;//发送长度
    CS = CSCalc ( RS485Tx2 );
    RS485Tx2.Buff[18] = CS; //校验码
    RS485Tx2.Buff[19] = 0x16; //结束符

    RS485Ch2SendMsg();//读取电表总电量请求帧

}

/*************************************************************
Function: void MeterCur(void)
Description: 读电流
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void MeterCur ( void ) //读取正向有功总电量 68 AA AA AA AA AA AA 68 11 04 33 34 35 35 B2 16
{

    uint8_t CS;
    RS485Tx2.Buff[0] = 0xFE; //前导字节
    RS485Tx2.Buff[1] = 0xFE;
    RS485Tx2.Buff[2] = 0xFE;
    RS485Tx2.Buff[3] = 0xFE;
    RS485Tx2.Buff[4] = 0x68; //帧起始符

    RS485Tx2.Buff[5] = 0xaa;//MeterAddr[0];//地址域
    RS485Tx2.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx2.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx2.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx2.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx2.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx2.Buff[11] = 0x68; //帧起始符
    RS485Tx2.Buff[12] = 0x11; //控制码
    RS485Tx2.Buff[13] = 0x04; //数据域长度
    RS485Tx2.Buff[14] = 0x33; //数据标识码
    RS485Tx2.Buff[15] = 0x34;
    RS485Tx2.Buff[16] = 0x35;
    RS485Tx2.Buff[17] = 0x35;

    RS485Tx2.Len = 20;//发送长度
    CS = CSCalc ( RS485Tx2 );
    RS485Tx2.Buff[18] = CS; //校验码
    RS485Tx2.Buff[19] = 0x16; //结束符

    RS485Ch2SendMsg();//读取电表总电量请求帧

}

/*************************************************************
Function:     void MeterJs(void)
Description:  广播校时
Calls:       无
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void MeterJs ( void ) //广播校时
{


}

/*************************************************************
Function:     uint8_t GetMeterOnlineSta(void)
Description:  获取电表通信状态
Calls:       无
Called By:   无
Input:       无
Output:      无
Return:      0-在线 1-掉线
Others:      无
*************************************************************/
uint8_t GetMeterOnlineSta(void)
{
    return MeterCommErr;
}

/*************************************************************
Function:     void MeterDeal ( void )
Description:  读电表，判断是否掉线
Calls:       无
Called By:   无
Input:       无
Output:      无
Return:      无
Others:      无
*************************************************************/
void MeterDeal ( void )
{
    GetMeterMsg();

    if ( MeterOnlineCnt >= 10 )         //10次通信超时则认为电表通信故障
    {
        MeterCommErr = 1 ;
        ERRORLEDOPEN();
    }
    else
    {
        MeterCommErr = 0 ;
        ERRORLEDCLOSE();
    }
    
    if (SysTickCnt > RdMeterTicks + 1000 )//每1s读一次
    {
        MeterOnlineCnt += 1;
        if (Meter_counter == 1)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCalc();//读电表电量
            Meter_counter += 1;
        }
        else if (Meter_counter == 2)
        {       
            RdMeterTicks = SysTickCnt ;
            MeterVol();//读电表电压
            Meter_counter += 1;
        }
        else if (Meter_counter == 3)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCur();//读电表电流
            Meter_counter += 1;
        }
        else
        {
            Meter_counter = 1;
        }
    }
}












