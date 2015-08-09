/**********************************************************
江苏嘉钰新能源技术有限公司
File name:  Emac_uIP.c	              // 文件名
Author:     王辉                    // 作者
Version:    v1.00                   // 版本信息
Date:       2013-9-6               // 完成日期
Description:  	              // 实现UIP初始化及通信功能
Others:		              // 无
Function List:	              //
**********************************************************/
#include <stdio.h>
#include <string.h>
#include "drv_inc.h"
#include "clock-arch.h"
#include "timer.h"
#include "uip-conf.h"
#include "uipopt.h"
#include "uip_arp.h"
#include "uip.h"
#include "emac.h"
#include "tcpip.h"


unsigned char net_timeover10=0;
/************************** PRIVATE DEFINITIONS ***********************/
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
uint16_t net_time05=0;
uint16_t net_time10=0;
uint8_t ConnectedFlag;

void NetTimerDeal(void)
{
    net_time05++;
    net_time10++;
}
/*******************************************************************************
*	NetLowLevelInit
*	输  入:
*	输  出:
*	功能说明：初始化网络硬件、UIP协议栈、配置本机IP地址
************************************************************/
void NetLowLevelInit(void)
{
    uip_ipaddr_t ipaddr;
    tapdev_init();                     		 //硬件初始化
    uip_arp_init();
    uip_init();								 //UIP协议栈初始化
    uip_ethaddr.addr[0] = MyMAC[0];
    uip_ethaddr.addr[1] = MyMAC[1];
    uip_ethaddr.addr[2] = MyMAC[2];
    uip_ethaddr.addr[3] = MyMAC[3];
    uip_ethaddr.addr[4] = MyMAC[4];
    uip_ethaddr.addr[5] = MyMAC[5];
    uip_setethaddr(uip_ethaddr);    
    
    uip_ipaddr(ipaddr, MyIP[0]&0xff,MyIP[0]>>8,MyIP[1]&0xff,MyIP[1]>>8);		 //设置IP地址
    //uip_ipaddr(ipaddr, 192,168,1,15);
    uip_sethostaddr(ipaddr);
    
    uip_ipaddr(ipaddr, GatewayIP[0]&0xff,GatewayIP[0]>>8,GatewayIP[1]&0xff,GatewayIP[1]>>8);//设置默认路由器IP地址
    //uip_ipaddr(ipaddr, 192,168,1,1);
    uip_setdraddr(ipaddr);
    
    uip_ipaddr(ipaddr, SubnetMask[0]&0xff,SubnetMask[0]>>8,SubnetMask[1]&0xff,SubnetMask[1]>>8);		 //设置网络掩码
    //uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
    
    /* 创建一个TCP监听端口和http监听端口，端口号为1200，80 */
    uip_listen(HTONS(TCPLocalPort));//
}

void ConnToServer(void)
{
    struct uip_conn *u;
    uip_ipaddr_t ipaddr;
    
    uip_ipaddr(ipaddr, 192,168,100,100); // 目的地址  
    u=uip_connect((uip_ipaddr_t*)ipaddr , HTONS(8000) ); 
    if(u==NULL)
    {
        //这里要做错误处理。                
        return ;
    }
    uip_periodic_conn(u);
    if(uip_len== 0)
    {
        uip_periodic_conn(u);//由于在uip_connect时conn->timer = 1,"
        //"所以必须调用uip_periodic_conn两次uip才会准备数据"
    } 
    if(uip_len > 0)
    {
        uip_arp_out();
        tapdev_send(uip_buf,uip_len);
    }      
}
/****************************************************************************
* 名    称：void EthPoll(void)
* 功    能：定时查询TCP连接收发状态	ARP表更新， 并响应
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void EthPoll(void){
    unsigned char i=0;
    if(net_time05>=500)			/* 0.5秒定时器超时 */
    {
        net_time05=0;
//        net_timeover05=0;			/* 复位0.5秒定时器 */
        
        /* 轮流处理每个TCP连接, UIP_CONNS缺省是10个 */
        for(i = 0; i < UIP_CONNS; i++)
        {
            uip_periodic(i);		/* 处理TCP通信事件 */
            /*
            当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
            需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)
            */
            if(uip_len > 0)
            {
                uip_arp_out();		//加以太网头结构，在主动连接时可能要构造ARP请求
                tapdev_send(uip_buf,uip_len);		//发送数据到以太网（设备驱动程序）
            }
        }
        
#if UIP_UDP
        /* 轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个 */
        for(i = 0; i < UIP_UDP_CONNS; i++)
        {
            uip_udp_periodic(i);	/*处理UDP通信事件 */
            /* 如果上面的函数调用导致数据应该被发送出去，全局变量uip_len设定值> 0 */
            if(uip_len > 0)
            {
                uip_arp_out();		//加以太网头结构，在主动连接时可能要构造ARP请求
                tapdev_send(uip_buf,uip_len);		//发送数据到以太网（设备驱动程序）
            }
        }
#endif /* UIP_UDP */
        
        /* 每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃*/
        if (net_time10>=10000)
        {
            net_time10=0;
            //net_timeover10=0;		/* 复位10秒定时器 */
            uip_arp_timer();
        }
    }
}
/*******************************************************************************
*	函数名：UipPro
*	输  入:
*	输  出:
*	功能说明：中断触发读取网络接收缓存
********************************************************************/
void UipPro(void)
{
      uip_len = tapdev_read(uip_buf);	//从网络设备读取一个IP包,返回数据长度
      if(uip_len > 0)			    //收到数据
      {
          /* 处理IP数据包(只有校验通过的IP包才会被接收) */
          if(BUF->type == htons(UIP_ETHTYPE_IP))   //是IP包吗？
          {
              uip_arp_ipin();		   //去除以太网头结构，更新ARP表
              uip_input();		   //IP包处理
              /*
              当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
              需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)
              */
              if (uip_len > 0)		//有带外回应数据
              {
                  uip_arp_out();		//加以太网头结构，在主动连接时可能要构造ARP请求
                  tapdev_send(uip_buf,uip_len);		//发送数据到以太网（设备驱动程序）
              }
          }
          /* 处理arp报文 */
          else if (BUF->type == htons(UIP_ETHTYPE_ARP))	//是ARP请求包
          {
              uip_arp_arpin();		//如是是ARP回应，更新ARP表；如果是请求，构造回应数据包
              /*
              当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
              需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)
              */
              if (uip_len > 0)		//是ARP请求，要发送回应
              {
                  tapdev_send(uip_buf,uip_len);		//发ARP回应到以太网上
              }
          }
      }
      else
      {
          EthPoll(); //定时查询TCP及UDP连接收发状态	ARP表更新， 并响应
      }
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*************************************************************
Function: void NetPro(void)
Description: 处理UIP事件
Calls:
Called By:   
Input:       
Output:      
Return:      
Others:      
*************************************************************/
void NetPro(void)
{
    /* 处理uip事件，必须插入到用户程序的循环体中 */
/*    if (ConnectedFlag==0)
    {
        ConnToServer();
        ConnectedFlag = 2 ;
    }*/
    UipPro();	//读取网络接收缓存
}
