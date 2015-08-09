
#include "stdint.h"
#include <math.h>
#include "drv_inc.h"
#include "bsp_inc.h"
#include "app_inc.h"

ANALOGVALUE g_sAnalogValue;
const uint32_t TC_Temp_Table[] =
{
    151240, 143360, 136040, 129220, 123540, 117730, 112060, 107330, 101880, 97193,
    92760, 88315, 84136, 79988, 75923, 71980, 68187, 64565, 61124, 57872,
    54810, 51935, 49242, 46725, 44373, 42180, 39939, 27899, 36023, 34284,
    32660, 31132, 29687, 28314, 27003, 25750, 24549, 23396, 22290, 21228,
    20210, 19223, 18296, 17423, 16600, 15823, 15088, 14392, 13734, 13109,
    12518, 11958, 11427, 10925, 10449, 10000, 9575,  9175,  8798,  8443,
    8110,  7779,  7460,  7151,  6853,  6566,  6288,  6021,  5767,  5523,
    5292,  5074,  4870,  4679,  4502,  4340,  4178,  4022,  3872,  3728,
    3588,  3454,  3325,  3202,  3083,  2969,  2860,  2755,  2655,  2560,
    2468,  2382,  2297,  2215,  2135,  2057,  1982,  1910,  1841,  1775,
    1712,  1652,  1595,  1542,  1492,  1445,  1402,  1361,  1321,  1282,
    1243,  1201,  1170,  1134,  1100,  1065,  1032,  1000,  967,   936,
    905,   883,   860,   836,   812,   787,   763,   739,   715,   692,
    670,   648,   627,   608,   590,   573,   557,   542,   526,   512,
    497,   484,   470,   457,   444,   432,   420,   408,   397,   386,
    376,   365,   356,   147,   340,   330,   322,   314,   306,   299,
    292,   285,   278,   271,   264,   257,   251,   244,   238,   232,
    226,   221,   215,   210,   205,   200,   196,   291,   298,   184,
    180
};
/*******************************************************************************
  Function:    uint32_t calresvalue(uint32_t sdata)
  Description: initialization of communicating with host machine
  Calls:       main
  Called By:   nothing
  Input:       nothing
  Output:      nothing
  Return:      nothing
  Others:      nothing
*******************************************************************************/
uint32_t calresvalue ( uint32_t sdata )
{
    uint32_t rx, tmp;

    if ( sdata >= S3V3 )
    {
        rx = 200000;
    }
    else if ( sdata == 0 )
    {
        rx = 0;
    }
    else
    {
        rx = sdata * RES1;
        tmp = S3V3 - sdata;
        rx /= tmp;
    }

    return rx;
}
/*******************************************************************************
  Function:    uint32_t caltmpvalue(uint32_t rdata)
  Description: initialization of communicating with host machine
  Calls:       main
  Called By:   nothing
  Input:       nothing
  Output:      nothing
  Return:      nothing
  Others:      nothing
*******************************************************************************/
uint32_t caltmpvalue ( uint32_t rdata )
{
    uint16_t tx, tmp;

    for ( tmp = 0 ; tmp < sizeof ( TC_Temp_Table ) / sizeof ( &TC_Temp_Table ); tmp++ )
    {
        if ( rdata > TC_Temp_Table[tmp] )
        {
            tx = ( tmp + 20 ) * 10;
            break;
        }
    }

    return tx;
}
/*******************************************************************************
  Function:    void dealcurrent(void)
  Description: initialization of communicating with host machine
  Calls:       main
  Called By:   nothing
  Input:       nothing
  Output:      nothing
  Return:      nothing
  Others:      nothing
*******************************************************************************/
uint32_t dealcurrent ( void )
{
    uint32_t tmp;

    tmp = GetCurrent ( 0 );
    tmp *= 660000;
    tmp /= S3V3;

    g_sAnalogValue.current = tmp;
    return tmp;
}

uint8_t GetBatSta(void)
{
    int32_t tmp1;

    tmp1 = GetFuseSta ( 0 );
    //g_ChgCtrl.CurrI =tmp1;
    if ( tmp1 > 100 )//电池电压大于200V
    {
        return(1);
    }
    else
    {
        return(0);
    }    
}
uint8_t GetAnalogSta(void)
{
    uint32_t value;
    value = GetTemp(0);
    if(value < 350)                                                       // 677
    {
        return 6;
    }
    else if(value < 452)                                                // 1015
    {
        return 9;
    }
    else                                                                 // 1354
    {
        return 12;
    } 
    
}
/*******************************************************************************
  Function:    void dealfuse(void)
  Description: calculate temperature value of inputed channel
  Calls:       main
  Called By:   nothing
  Input:       nothing
  Output:      nothing
  Return:      nothing
  Others:      nothing
*******************************************************************************/
uint8_t dealfuse ( void )
{
    int32_t tmp1, tmp2;

    tmp1 = GetFuseSta ( 0 );
    tmp2 = GetFuseSta ( 1 );
    tmp1 -= tmp2;

    if ( tmp1 < 0 )
    {
        tmp1 = -tmp1 ;
    }

    if ( tmp1 > 100 )
    {
        g_sAnalogValue.fuse = 1;
    }
    else
    {
        g_sAnalogValue.fuse = 0;
    }

    return g_sAnalogValue.fuse;
}
/*******************************************************************************
  Function:    uint16_t dealtemp(uint8_t num)
  Description: calculate temperature value of inputed channel
  Calls:       GetTemp
               calresvalue
  Called By:   nothing
  Input:       channel's number
  Output:      nothing
  Return:      value of temperature
  Others:      nothing
*******************************************************************************/
uint16_t dealtemp ( uint8_t num )
{
    uint32_t tmp;

    if ( num == 1 )
    {
        tmp = GetTemp ( 0 );
        tmp = calresvalue ( tmp );
        g_sAnalogValue.temp[0] = caltmpvalue ( tmp );;
        return g_sAnalogValue.temp[0];
    }
    else if ( num == 2 )
    {
        tmp = GetTemp ( 1 );
        tmp = calresvalue ( tmp );
        g_sAnalogValue.temp[1]  = caltmpvalue ( tmp );
        return g_sAnalogValue.temp[1];
    }

    return 0 ;
}

//end


