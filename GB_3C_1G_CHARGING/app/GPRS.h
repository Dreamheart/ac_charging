#ifndef __GPRS_H__
#define __GPRS_H__

#define heartbeat_cycle 60      //心跳周期，单位秒, 1分钟
#define Info_cycle 35           //数据上传周期，单位秒，35秒 
#define Communication_timeout 600      //通信超时，单位秒，10分钟
//功能码
#define Heartbeat_Function_code 0x01            //心跳
#define Account_Verification_Function_code 0x02 //身份认证---手动输入用户名和密码
#define QR_code_Verification_Function_code 0x03 //身份认证---扫二维码
#define Start_Function_code 0x04                //充电启动请求
#define Stop_Function_code 0x05                 //充电停止请求
#define AC_Info_Function_code 0x06              //数据上传---交流
#define DC_Info_Function_code 0x07              //数据上传---直流
#define APP_Stop_Function_code 0x08             //APP发起充电停止
#define Card_Verification_Function_code 0x09    //身份认证---卡用户
#define calibration_time_Function_code 0x0a     //校时
//数据域长度
#define Len_Heartbeat 0x02
#define Len_Account_Verification 0x12
#define Len_QR_code_Verification 0
#define Len_Start 0x10
#define Len_Stop 0x08
#define Len_AC_Info 0x0c
#define Len_DC_Info 0x0d
#define Len_APP_Stop 0
#define Len_Card_Verification 0x16
#define Len_calibration_time 0

typedef struct
{
    uint8_t tel_number[12];         //手机号,第一位0，后面11位为真实手机号，ASCII
    uint8_t password[6];        //密码，ASCII
    uint8_t card_number[16];    //卡号，ASCII
    uint8_t card_status;        //卡状态
    uint32_t Balance;           //余额
    uint32_t pay;               //消费金额
} user_info;

extern void GPRS_main( void );


#endif

