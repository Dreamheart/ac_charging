#include "stm32f10x.h"
#include  "uip.h"  
#include  "enc28j60.h"
#include "uipopt.h"
#include "uip_arch.h"
#include <string.h>	 

#define LED1_ON()	GPIO_SetBits(GPIOB,  GPIO_Pin_5);
#define LED2_ON()	GPIO_SetBits(GPIOD,  GPIO_Pin_6);
#define LED3_ON()	GPIO_SetBits(GPIOD,  GPIO_Pin_3);

#define LED1_OFF()	GPIO_ResetBits(GPIOB,  GPIO_Pin_5);
#define LED2_OFF()	GPIO_ResetBits(GPIOD,  GPIO_Pin_6);
#define LED3_OFF()	GPIO_ResetBits(GPIOD,  GPIO_Pin_3);

extern unsigned short LPORT;

extern void tcp_demo_appcall(void);
void myudp_appcall(void);


/*******************************************************************************
*	函数名: void myudp_send(char *str,short n) 
*	参  数: 
*	返  回: 无
*	功  能: UDP 数据包发送
**************************************************************************/
void myudp_send(char *str,short n) 
{ 

   char   *nptr;   
   nptr = (char *)uip_appdata;       
   memcpy(nptr, str, n); 
   uip_udp_send(n);   				//发送n个数据 
} 

/*******************************************************************************
*	函数名: void UDP_newdata(void) 
*	参  数: 
*	返  回: 无
*	功  能: UDP 数据包发送
**************************************************************************/
void UDP_newdata(void) 
{ 
    char   *nptr; 
    short len; 
	len = uip_datalen();									//读取数据长度 
    nptr = (char *)uip_appdata; 							//取得数据起始指针 
	if(len<4)myudp_send("Please check the command!\n",26); 
    else if(strncmp(nptr,"getname",7)==0) myudp_send("我是奋斗STM32开发板\n",19); 
	else if(strncmp(nptr,"ledon 1",7)==0){
		LED1_ON();
		myudp_send("LED1 亮\n",8);
	} 
	else if(strncmp(nptr,"ledon 2",7)==0){
		LED2_ON();
		myudp_send("LED2 亮\n",8);
	}
	else if(strncmp(nptr,"ledon 3",7)==0){
		LED3_ON();
		myudp_send("LED3 亮\n",8);
	}
	else if(strncmp(nptr,"ledoff 1",8)==0){
		LED1_OFF();
		myudp_send("LED1 灭\n",8);
	} 
	else if(strncmp(nptr,"ledoff 2",8)==0){
		LED2_OFF();
		myudp_send("LED2 灭\n",8);
	}
	else if(strncmp(nptr,"ledoff 3",8)==0){
		LED3_OFF();
		myudp_send("LED3 灭\n",8);
	}

    else myudp_send("Unkown command!\n",16); 
} 
/*******************************************************************************
*	函数名: void myudp_appcall(void) ) 
*	参  数: 
*	返  回: 无
*	功  能: UDP主函数
**************************************************************************/
void myudp_appcall(void) 
{ 
   if(uip_newdata()) 						
   { 
       UDP_newdata();      
   } 
} 

/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/


