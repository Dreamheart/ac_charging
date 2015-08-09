
#include "lpc177x_8x_adc.h"
#include "lpc177x_8x_pinsel.h"
#include "string.h"

//ADC input preset on this board
#define BRD_ADC_PREPARED_CHANNEL0       (ADC_CHANNEL_0)
#define BRD_ADC_PREPARED_CHANNEL1       (ADC_CHANNEL_1)
#define BRD_ADC_PREPARED_CHANNEL2       (ADC_CHANNEL_4)
#define BRD_ADC_PREPARED_CHANNEL3       (ADC_CHANNEL_5)
#define BRD_ADC_PREPARED_CHANNEL4       (ADC_CHANNEL_6)
#define BRD_ADC_PREPARED_CHANNEL5       (ADC_CHANNEL_7)

#define BRD_ADC_PREPARED_INTR0	        (ADC_ADINTEN0)
#define BRD_ADC_PREPARED_INTR1	        (ADC_ADINTEN1)
#define BRD_ADC_PREPARED_INTR2	        (ADC_ADINTEN4)
#define BRD_ADC_PREPARED_INTR3		(ADC_ADINTEN5)
#define BRD_ADC_PREPARED_INTR4		(ADC_ADINTEN6)
#define BRD_ADC_PREPARED_INTR5		(ADC_ADINTEN7)

#define ADC0_PREPARED_CH_PORT		(0)  /*AD INPUT0*/
#define ADC0_PREPARED_CH_PIN		(23)
#define ADC0_PREPARED_CH_FUNC_NO	(1)

#define ADC1_PREPARED_CH_PORT		(0)  /*AD INPUT1*/
#define ADC1_PREPARED_CH_PIN		(24)
#define ADC1_PREPARED_CH_FUNC_NO	(1)

#define ADC2_PREPARED_CH_PORT		(1)  /*AD INPUT4*/
#define ADC2_PREPARED_CH_PIN		(30)
#define ADC2_PREPARED_CH_FUNC_NO	(3)

#define ADC3_PREPARED_CH_PORT		(1)  /*AD INPUT5*/
#define ADC3_PREPARED_CH_PIN		(31)
#define ADC3_PREPARED_CH_FUNC_NO	(3)

#define ADC4_PREPARED_CH_PORT		(0)  /*AD INPUT6*/
#define ADC4_PREPARED_CH_PIN		(12)
#define ADC4_PREPARED_CH_FUNC_NO	(3)

#define ADC5_PREPARED_CH_PORT		(0)  /*AD INPUT7*/
#define ADC5_PREPARED_CH_PIN		(12)
#define ADC5_PREPARED_CH_FUNC_NO	(3)

//==============================================================================

#define ADC_CHANNEL_MAX                  8
#define ADC_RESULT_MAX                   16

uint16_t ad_rlt[ADC_CHANNEL_MAX][ADC_RESULT_MAX];
uint8_t ad_cnt[ADC_CHANNEL_MAX];

const uint8_t ad_channel_sel[ADC_CHANNEL_MAX] =
{
    ADC_CHANNEL_0, ADC_CHANNEL_1, 0, 0,
    ADC_CHANNEL_4, ADC_CHANNEL_5, ADC_CHANNEL_6, ADC_CHANNEL_7
};

uint32_t GetAdAvgValue ( uint8_t num )
{
    uint8_t rpt;
    uint32_t rt = 0;

    for ( rpt = 0; rpt < ADC_RESULT_MAX; rpt++ )
    {
        rt += ad_rlt[num][rpt];
    }

    rt /= ADC_RESULT_MAX;
    return rt;

}
/*************************************************************
  Function: uint32_t GetCurrent(uint8_t num)
  Description: 获取当前电流值
  Calls:
  Called By:
  Input:       num--通道号
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
uint32_t GetCurrent ( uint8_t num )
{
    return GetAdAvgValue ( BRD_ADC_PREPARED_CHANNEL3 );
}

/*************************************************************
  Function: uint32_t GetFuseSta(uint8_t num)
  Description: 获取当前熔丝状态
  Calls:
  Called By:
  Input:       num--通道号
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
uint32_t GetFuseSta ( uint8_t num )
{
    return GetAdAvgValue ( BRD_ADC_PREPARED_CHANNEL4 + num );
}

/*************************************************************
  Function: uint32_t GetTemp(uint8_t num)
  Description: 获取当前温度
  Calls:
  Called By:
  Input:       num--通道号
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
uint32_t GetTemp ( uint8_t num )
{
    return GetAdAvgValue ( BRD_ADC_PREPARED_CHANNEL0 + num );
}
/*************************************************************
  Function: void FlashInit(void)
  Description: 初始化ADC接口，初始化ADC缓冲区
  Calls:
  Called By:   main()
  Input:       无
  Output:      无
  Return:      无
  Others:      无
*************************************************************/
void AdcInit ( void )
{
    memset ( ( uint8_t * ) ad_rlt, 0, sizeof ( ad_rlt ) );
    memset ( ad_cnt, 0, sizeof ( ad_cnt ) );
    /* Initialize ADC ----------------------------------------------------*/
    /*
     * Init ADC pin that currently is being used on the board
     */
    PINSEL_ConfigPin ( ADC0_PREPARED_CH_PORT, ADC0_PREPARED_CH_PIN, ADC0_PREPARED_CH_FUNC_NO );
    PINSEL_ConfigPin ( ADC1_PREPARED_CH_PORT, ADC1_PREPARED_CH_PIN, ADC1_PREPARED_CH_FUNC_NO );
    //PINSEL_ConfigPin ( ADC2_PREPARED_CH_PORT, ADC2_PREPARED_CH_PIN, ADC2_PREPARED_CH_FUNC_NO );
    PINSEL_ConfigPin ( ADC3_PREPARED_CH_PORT, ADC3_PREPARED_CH_PIN, ADC3_PREPARED_CH_FUNC_NO );
    PINSEL_ConfigPin ( ADC4_PREPARED_CH_PORT, ADC4_PREPARED_CH_PIN, ADC4_PREPARED_CH_FUNC_NO );
    PINSEL_ConfigPin ( ADC5_PREPARED_CH_PORT, ADC5_PREPARED_CH_PIN, ADC5_PREPARED_CH_FUNC_NO );
    /* Configuration for ADC :
     *  ADC conversion rate = 400Khz
     */
    ADC_Init ( LPC_ADC, 400000 );
    ADC_BurstCmd ( LPC_ADC, ENABLE );

    ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR0, DISABLE );
    ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR1, DISABLE );
    //ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR2, DISABLE );
    ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR3, DISABLE );
    ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR4, DISABLE );
    ADC_IntConfig ( LPC_ADC, BRD_ADC_PREPARED_INTR5, DISABLE );
    ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL0, ENABLE );
    ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL1, DISABLE );
    //ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL2, ENABLE );
    ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL3, DISABLE );
    ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL4, DISABLE );
    ADC_ChannelCmd ( LPC_ADC, BRD_ADC_PREPARED_CHANNEL5, DISABLE );

    // Start conversion
    ADC_StartCmd ( LPC_ADC, ADC_START_CONTINUOUS );

}

void adc_main ( void )
{
    uint32_t adc_value, tmp;

    tmp = ADC_GlobalGetData ( LPC_ADC );

    if ( tmp & ADC_DR_OVERRUN_FLAG )
    {
        tmp = ADC_GDR_CH ( ADC_GlobalGetData ( LPC_ADC ) );
        adc_value = ADC_ChannelGetData ( LPC_ADC, ad_channel_sel[tmp] );
        ad_rlt[tmp][ad_cnt[tmp]++] = adc_value;
        ad_cnt[tmp] %= ADC_RESULT_MAX;
    }
}