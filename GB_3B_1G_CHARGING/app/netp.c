#include <string.h>
#include <time.h>
#include "drv_inc.h"

#include "app_inc.h"
#include "bsp_inc.h"

// API global vars and flags
unsigned short TCPRxDataCount;            // nr. of bytes rec'd
unsigned short TCPTxDataCount;            // nr. of bytes to send

unsigned short TCPLocalPort;              // TCP ports
unsigned short TCPRemotePort;

unsigned short RemoteMAC[3];              // MAC and IP of current TCP-session
unsigned short RemoteIP[2];

unsigned short MyIP[2];
unsigned short SubnetMask[2];
unsigned short GatewayIP[2];
unsigned char MyMAC[6];
extern void NetLowLevelInit(void);

unsigned char SocketStatus;

const unsigned short MyIP_init[] =                  // "MYIP1.MYIP2.MYIP3.MYIP4"
{
    MYIP_1 + ( MYIP_2 << 8 ),                             // use 'unsigned int' to
    MYIP_3 + ( MYIP_4 << 8 )                             // achieve word alignment
};

const unsigned short SubnetMask_init[] =// "SUBMASK1.SUBMASK2.SUBMASK3.SUBMASK4"
{
    SUBMASK_1 + ( SUBMASK_2 << 8 ),                       // use 'unsigned int' to
    SUBMASK_3 + ( SUBMASK_4 << 8 )                       // achieve word alignment
};

const unsigned short GatewayIP_init[] =             // "GWIP1.GWIP2.GWIP3.GWIP4"
{
    GWIP_1 + ( GWIP_2 << 8 ),                             // use 'unsigned int' to
    GWIP_3 + ( GWIP_4 << 8 )                             // achieve word alignment
};

const unsigned char MyMAC_init[6] =                       // "M1-M2-M3-M4-M5-M6"
{
    EMAC_ADDR0, EMAC_ADDR1, EMAC_ADDR2,
    EMAC_ADDR3, EMAC_ADDR4, EMAC_ADDR5
};


#define RPT_CNT   0X3
void NetInit ( void )
{
    uint16_t rpt;

    for ( rpt = 0; rpt < RPT_CNT; rpt++ )
    {
        if ( LoadEthernet() == 0 )
        {
            break;
        }
    }

    if ( rpt >= RPT_CNT )
    {
        memcpy ( MyIP, MyIP_init, sizeof ( MyIP ) );
        memcpy ( SubnetMask, SubnetMask_init, sizeof ( SubnetMask ) );
        memcpy ( GatewayIP, GatewayIP_init, sizeof ( GatewayIP ) );
        TCPLocalPort = 2408;
        memcpy ( MyMAC, MyMAC_init, sizeof ( MyMAC ) );

        SaveEthernet();
    }
    NetLowLevelInit();
}

