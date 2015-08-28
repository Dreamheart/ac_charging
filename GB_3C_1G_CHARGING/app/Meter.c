

/**********************************************************
�º�����Դ�Ƽ����޹�˾
File name:  hwtst.c	              // �ļ���
Author:     ���һ�                   // ����
Version:    v1.00                   // �汾��Ϣ
Date:       2015-1-23              // �������
Description:  	              // ʹ��232������GPRSģ��ͨ�ţ����ʵ�����ۺ�ƽ̨��ͨ��
Others:		              // 
Function List:	              //
History:                        //
**********************************************************/

#include "board.h"
#include "Dio.h"
#include "uart.h"
#include "eeprom.h"
#include "crc.h"

//У��������
extern UART_BUFF RS485Rx1;
uint8_t MeterData[4], MeterAddr[6];
uint8_t MeterErr, MeterCommErr, MeterAddrFlag;
uint8_t Position68;
uint32_t RdMeterTicks, RdMeterTicks1, RdMeterTicks2;       //������������ѹ��������ʱ������
uint8_t Meter_counter;
uint16_t MeterOnlineCnt;
uint32_t CurKwh;
uint32_t Cur_0_BeginTime,Cur_0_Time;
uint8_t RelaySta2;      //4852����״ָ̬ʾ�ƣ����濪��
extern uint16_t output_vol, output_cur;         //�����ѹ������
                                                //�������������λС�����������ر�����������һλ��ϵͳ�а�����λС������
                                                //��ѹ�������һλС�����������ر�����С���ڶ�λ��0��ϵͳ�а�����λС������


/*************************************************************
Function: uint32_t GetCurKwh ( void )
Description: ��ȡ��ǰ����
Calls:       ��
Called By:   
Input:       
Output:      ��
Return:      CurKwh ��ǰ�����й��ܵ���
Others:      ��
*************************************************************/
uint32_t GetCurKwh ( void )
{
    return CurKwh;
}





/*************************************************************
Function: uint8_t Request_CSCalc(UART_BUFF f)
Description: ���㷢�������У����
Calls:       ��
Called By:   
Input:       UART_BUFF f Ӧ��֡����
Output:      ��
Return:      uint8_t cs У����
Others:      ��
*************************************************************/
uint8_t Request_CSCalc ( UART_BUFF f )
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
Function: uint8_t Answer_CSCalc(UART_BUFF f)
Description: ����Ӧ���У����
Calls:       ��
Called By:   
Input:       UART_BUFF f Ӧ��֡����
Output:      ��
Return:      uint8_t cs У����
Others:      ��
*************************************************************/
uint8_t Answer_CSCalc ( UART_BUFF f )
{
    uint8_t cs = 0x00, i, len, j;
    len = f.Len;
    Position68 = 0 ;

    for ( i = 0; i < len; i++ )
    {
        if ( f.Buff[i] == 0x68 )
        {
            Position68 = i ;
            f.Len = f.Buff[i+9] + 16;
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
Description: ��ȡ�������
Calls:       ��
Called By:
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void ReadMeterAddr ( void ) //��ȡ�������
{

}
/*************************************************************
Function: void BcdToHex(uint32_t *dat ,uint8_t *ptr)
Description: BCD����ת��Ϊ16����
Calls:       ��
Called By:   
Input:       *dat--HEX����ָ�� *ptr--BCD����ָ��
Output:      ��
Return:      ��
Others:      ��
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
Description: ��վӦ��֡
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void GetMeterMsg ( void ) //��վӦ��֡
{

    uint8_t  len, i, j, CS;
    uint8_t tmp_[4];
    uint8_t temp;
    uint32_t tmp,tmp1,tmp2;
    
    uint16_t temp_;
    uint16_t *data;
    uint8_t crcl, crch;
    uint16_t addr = 0;
      
    if ( GetRs485Ch1Sta() == 1 ) //֡���ս������
    {
        if (len<8) 
        {
          RS485Rx1.Flag = 0 ;
          RS485Rx1.Len = 0 ;
          RS485Rx1.Idx = 0 ; 
          return;
        }
        MeterOnlineCnt = 1;
        CS = Answer_CSCalc ( RS485Rx1 );
        len = RS485Rx1.Len;
        RS485Rx1.Flag = 0 ;
        RS485Rx1.Len = 0 ;
        RS485Rx1.Idx = 0 ;        
  
///////////////////////////////////////////////////////////////////////////////////////////////////////    
///////////////////////////////////////////////////////////////////////////////////////////////////////  
    
    
          
       if ( RS485Rx1.Buff[0] == 1)//���ж��Ƿ񱾻���ַ
        {
            Chip_CRC_Init();
          crch = CRC16(RS485Rx1.Buff,(len-2),Hi);
          crcl = CRC16(RS485Rx1.Buff,(len-2),Lo);

            if ( ( crcl == RS485Rx1.Buff[len-2] ) && ( crch == RS485Rx1.Buff[len-1] ) ) //�Ƚ�CRC
            {
                switch ( RS485Rx1.Buff[1] ) //������
                {
                    case 0x03:// ����վ���յ��ģ���ַ 03 ��ַ�� ��ַ�� �Ĵ��������� �Ĵ��������� CRCL CRCH
                        // ���͸���վ�ģ���ַ 03 �ֽڳ��� ����1 ����2 ����N CRCL CRCH
                    {
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//��ַ
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//03������

                        temp = RS485Rx1.Buff[4] ;
                        temp <<= 8 ;
                        temp |= RS485Rx1.Buff[5] ;//temp��Ϊ��վҪ��ĳ���
                        temp *= 2 ;              //1���Ĵ���Ϊ2�ֽڣ����Գ���X2
                        RS485Tx1.Buff[2] = temp ;//����
                        len = temp ;
                        addr = * ( ( uint16_t * ) ( &RS485Rx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ;
                        
                        switch(addr)
                        {
                            case 0x64:
                            {
                                LoadParameter();
                                RS485Tx1.Buff[3] = g_sParaConfig.id[0];
                                RS485Tx1.Buff[4] = g_sParaConfig.id[1];
                                RS485Tx1.Buff[5] = g_sParaConfig.id[2];
                                RS485Tx1.Buff[6] = g_sParaConfig.id[3];
                                break;
                            }
                        }

                        temp += 3 ;//���������� + ֡ͷ
                        
   //                     Chip_CRC_Init ();
   //                     data = ( uint16_t * )RS485Rx1.Buff;
   //                     temp_ = Chip_CRC_CRC16 ( data, temp );
   //                     crcl = temp_ ;
   //                     crch = temp_ >> 8 ;
                        
                         crch = CRC16(RS485Tx1.Buff,(temp),Hi);
                         crcl = CRC16(RS485Tx1.Buff,(temp),Lo);                       
                                                
                        RS485Tx1.Buff[temp] = crcl;
                        RS485Tx1.Buff[temp+1] = crch;
                        RS485Tx1.Len = temp + 2 ; //�����ͳ���
                        RS485Ch1SendMsg();
                        break;
                    }
                    case 0x10:// ����վ���յ��ģ���ַ 0x10 ��ַ�� ��ַ�� �Ĵ��������� �Ĵ��������� �ֽ��� ����1  ����2 ����N CRCL CRCH
                    {
                    
                        // ���͸���վ�ģ���ַ 0x10 ��ַ�� ��ַ�� �Ĵ��������� �Ĵ��������� CRCL CRCH
                        RS485Tx1.Buff[0] = RS485Rx1.Buff[0] ;//��ַ
                        RS485Tx1.Buff[1] = RS485Rx1.Buff[1] ;//01������
                        RS485Tx1.Buff[2] = RS485Rx1.Buff[2] ;//
                        RS485Tx1.Buff[3] = RS485Rx1.Buff[3] ;//
                        RS485Tx1.Buff[4] = RS485Rx1.Buff[4] ;//
                        RS485Tx1.Buff[5] = RS485Rx1.Buff[5] ;//

     //                   Chip_CRC_Init ();
     //                   data = ( uint16_t * )RS485Rx1.Buff;
     //                   temp_ = Chip_CRC_CRC16 ( data, 6 );
     //                   crcl = temp_ ;
      //                  crch = temp_ >> 8 ;
                        
                        crch = CRC16(RS485Rx1.Buff,6,Hi);
                        crcl = CRC16(RS485Rx1.Buff,6,Lo);                      
                        
                        RS485Tx1.Buff[6] = crcl;
                        RS485Tx1.Buff[7] = crch;

                        RS485Tx1.Len = 8 ;//�����ͳ���
                        RS485Ch1SendMsg(); //�ȷ��ͻ�Ӧ����վ���������ݴ���ʱ���������ͨ�Ŵ���

                        temp = RS485Tx1.Buff[4] ;
                        temp <<= 8 ;
                        temp |= RS485Tx1.Buff[5] ;//temp��Ϊ��վҪ��ĳ���
                        temp *= 2 ;             //1���Ĵ���Ϊ2�ֽڣ����Գ���X2
                        RS485Rx1.Buff[2] = temp;//����
                        len = temp ;//ȡ����
                        addr = * ( ( uint16_t * ) ( &RS485Tx1.Buff[2] ) );
                        addr =  SWAP ( addr ) ; //ȡ��ַ
                        i = 7;
                        
                        switch(addr)
                        {
                            case 0x64:
                            {
                                g_sParaConfig.id[0] = RS485Rx1.Buff[7];
                                g_sParaConfig.id[1] = RS485Rx1.Buff[8];
                                g_sParaConfig.id[2] = RS485Rx1.Buff[9];
                                g_sParaConfig.id[3] = RS485Rx1.Buff[10];
                                SaveParameter();
                                break;
                            }
                        }

                    }

                }
            }
            else
            {
                return;
            }
        }
        
        
///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////      
        
        if ( CS == RS485Rx1.Buff[len-2] )
        {
            if ( RS485Rx1.Buff[8+Position68] == 0x91 ) //��ȡ�����й��ܵ���,����Ӧ��
            {

                      //����״ָ̬ʾ��
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
              
              
              switch (RS485Rx1.Buff[9 + Position68])
                {
                case 0x06:
                    {
                        output_vol = 0;
                        for ( i = 14 + Position68, j = 0; j < 2; i++, j++ )
                        {
                            tmp_[j] = RS485Rx1.Buff[i] - 0x33;
                        }
                        tmp = ((tmp_[0] >> 4) & 0x0f) * 100 + (tmp_[0] & 0x0f) *10;
                        tmp1 = ((tmp_[1] >> 4) & 0x0f) * 10000 + (tmp_[1] & 0x0f) *1000;
                        output_vol = tmp + tmp1 ;
                        break;
                    }
                case 0x07:
                    {
                        //output_cur = 0;
                        for ( i = 14 + Position68, j = 0; j < 3; i++, j++ )
                        {
                            tmp_[j] = RS485Rx1.Buff[i] - 0x33;
                        }
                        tmp = ((tmp_[0] >> 4) & 0x0f) ;
                        tmp1 = ((tmp_[1] >> 4) & 0x0f) * 100 + (tmp_[1] & 0x0f) *10;
                        tmp2 = ((tmp_[2] >> 4) & 0x0f) * 10000 + (tmp_[2] & 0x0f) *1000;
                        output_cur = tmp + tmp1 + tmp2; 
                        if (output_cur > 100)           //��������1A
                        {
                           Cur_0_BeginTime = SysTickCnt; 
                        }
                        break;
                    }
                case 0x08://���������
                    {
                        for ( i = 14 + Position68, j = 0; j < 4; i++, j++ )
                        {
                            MeterData[j] = RS485Rx1.Buff[i] - 0x33;
                        }
                        BcdToHex ( &CurKwh, MeterData ); //�������BCD��,תΪHEX    
                        break;
                    }
                }
            }
            else if ( RS485Rx1.Buff[8+Position68] == 0xD1 ) //�����������վ�쳣Ӧ��
            {
                MeterErr = RS485Rx1.Buff[len-3]; //��ȡ������Ϣ��
            }
        }
    }

}

/*************************************************************
Function: void MeterCalc(void)
Description: ��ȡ�����й��ܵ���
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void MeterCalc ( void ) //��ȡ�����й��ܵ��� 68 AA AA AA AA AA AA 68 11 04 33 33 34 33 AE 16
{

    uint8_t CS;
    RS485Tx1.Buff[0] = 0xFE; //ǰ���ֽ�
    RS485Tx1.Buff[1] = 0xFE;
    RS485Tx1.Buff[2] = 0xFE;
    RS485Tx1.Buff[3] = 0xFE;
    RS485Tx1.Buff[4] = 0x68; //֡��ʼ��

    RS485Tx1.Buff[5] = 0xaa;//MeterAddr[0];//��ַ��
    RS485Tx1.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx1.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx1.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx1.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx1.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx1.Buff[11] = 0x68; //֡��ʼ��
    RS485Tx1.Buff[12] = 0x11; //������
    RS485Tx1.Buff[13] = 0x04; //�����򳤶�
    RS485Tx1.Buff[14] = 0x33; //���ݱ�ʶ��
    RS485Tx1.Buff[15] = 0x33;
    RS485Tx1.Buff[16] = 0x34;
    RS485Tx1.Buff[17] = 0x33;

    RS485Tx1.Len = 20;//���ͳ���
    CS = Request_CSCalc ( RS485Tx1 );    
    RS485Tx1.Buff[18] = CS; //У����
    RS485Tx1.Buff[19] = 0x16; //������
//for (CS=0;CS<20;CS++) RS485Tx1.Buff[CS] = 0X55;
    RS485Ch1SendMsg();//��ȡ����ܵ�������֡

}

/*************************************************************
Function: void MeterVol(void)
Description: ����ѹ
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void MeterVol ( void ) //��ȡ�����й��ܵ��� 68 AA AA AA AA AA AA 68 11 04 33 34 34 35 B1 16
{

    uint8_t CS;
    RS485Tx1.Buff[0] = 0xFE; //ǰ���ֽ�
    RS485Tx1.Buff[1] = 0xFE;
    RS485Tx1.Buff[2] = 0xFE;
    RS485Tx1.Buff[3] = 0xFE;
    RS485Tx1.Buff[4] = 0x68; //֡��ʼ��

    RS485Tx1.Buff[5] = 0xaa;//MeterAddr[0];//��ַ��
    RS485Tx1.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx1.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx1.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx1.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx1.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx1.Buff[11] = 0x68; //֡��ʼ��
    RS485Tx1.Buff[12] = 0x11; //������
    RS485Tx1.Buff[13] = 0x04; //�����򳤶�
    RS485Tx1.Buff[14] = 0x33; //���ݱ�ʶ��
    RS485Tx1.Buff[15] = 0x34;
    RS485Tx1.Buff[16] = 0x34;
    RS485Tx1.Buff[17] = 0x35;

    RS485Tx1.Len = 20;//���ͳ���
    CS = Request_CSCalc ( RS485Tx1 );    
    RS485Tx1.Buff[18] = CS; //У����
    RS485Tx1.Buff[19] = 0x16; //������
//for (CS=0;CS<20;CS++) RS485Tx1.Buff[CS] = 0X55;
    RS485Ch1SendMsg();//��ȡ����ܵ�������֡

}

/*************************************************************
Function: void MeterCur(void)
Description: ������
Calls:       uint8_t CSCalc(UART_BUFF f)
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void MeterCur ( void ) //��ȡ�����й��ܵ��� 68 AA AA AA AA AA AA 68 11 04 33 34 35 35 B2 16
{

    uint8_t CS;
    RS485Tx1.Buff[0] = 0xFE; //ǰ���ֽ�
    RS485Tx1.Buff[1] = 0xFE;
    RS485Tx1.Buff[2] = 0xFE;
    RS485Tx1.Buff[3] = 0xFE;
    RS485Tx1.Buff[4] = 0x68; //֡��ʼ��

    RS485Tx1.Buff[5] = 0xaa;//MeterAddr[0];//��ַ��
    RS485Tx1.Buff[6] = 0xaa;//MeterAddr[1];
    RS485Tx1.Buff[7] = 0xaa;//MeterAddr[2];
    RS485Tx1.Buff[8] = 0xaa;//MeterAddr[3];
    RS485Tx1.Buff[9] = 0xaa;//MeterAddr[4];
    RS485Tx1.Buff[10] = 0xaa;//MeterAddr[5];

    RS485Tx1.Buff[11] = 0x68; //֡��ʼ��
    RS485Tx1.Buff[12] = 0x11; //������
    RS485Tx1.Buff[13] = 0x04; //�����򳤶�
    RS485Tx1.Buff[14] = 0x33; //���ݱ�ʶ��
    RS485Tx1.Buff[15] = 0x34;
    RS485Tx1.Buff[16] = 0x35;
    RS485Tx1.Buff[17] = 0x35;

    RS485Tx1.Len = 20;//���ͳ���
    CS = Request_CSCalc ( RS485Tx1 );    
    RS485Tx1.Buff[18] = CS; //У����
    RS485Tx1.Buff[19] = 0x16; //������
//for (CS=0;CS<20;CS++) RS485Tx1.Buff[CS] = 0X55;
    RS485Ch1SendMsg();//��ȡ����ܵ�������֡

}

/*************************************************************
Function:     void MeterJs(void)
Description:  �㲥Уʱ
Calls:       ��
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void MeterJs ( void ) //�㲥Уʱ
{


}

/*************************************************************
Function:     uint8_t GetMeterOnlineSta(void)
Description:  ��ȡ���ͨ��״̬
Calls:       ��
Called By:   ��
Input:       ��
Output:      ��
Return:      0-���� 1-����
Others:      ��
*************************************************************/
uint8_t GetMeterOnlineSta(void)
{
    return MeterCommErr;
}

/*************************************************************
Function:     void MeterDeal ( void )
Description:  ��������ж��Ƿ����
Calls:       ��
Called By:   ��
Input:       ��
Output:      ��
Return:      ��
Others:      ��
*************************************************************/
void MeterDeal ( void )
{
    GetMeterMsg();

    if ( MeterOnlineCnt >= 10 )         //10��ͨ�ų�ʱ����Ϊ���ͨ�Ź���
    {
        MeterCommErr = 1 ;
    }
    else
    {
        MeterCommErr = 0 ;
    }
    
    if (SysTickCnt > RdMeterTicks + 1000 )//ÿ1s��һ��
    {
        MeterOnlineCnt += 1;
        if (Meter_counter == 1)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCalc();//���������
            Meter_counter += 1;
        }
        else if (Meter_counter == 2)
        {       
            RdMeterTicks = SysTickCnt ;
            MeterVol();//�������ѹ
            Meter_counter += 1;
        }
        else if (Meter_counter == 3)
        {
            RdMeterTicks = SysTickCnt ;
            MeterCur();//���������
            Meter_counter += 1;
        }
        else
        {
            Meter_counter = 1;
        }
    }
}










