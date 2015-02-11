#include "adc.h"
#define N 12
static u8 adc_count = 0;
//static u8 vaild_value_count = 0;//有效值标号计数
static u16 vaild_value[N+1];//取出有效值作为平均计算
volatile static u16 expect_final;//最终计算值
u32 total_vaild = 0;
static u16 uart_adc_value = 0;
static u16 display_value = 0;
static u8 compute_count = 0;
static u8 compute_rate = 0;
static void compute_display_value(void);

void ADC_Init(void)
{
   ADC1_DeInit();
#if CONVERSIONMODE ==CONVERSIONMODE_SINGLE
   /**< Single conversion mode */
   /**< Analog channel 10 */
   /**< Prescaler selection fADC2 = fcpu/18 */
   /**< Conversion from Internal TIM TRGO event */
   /** DISABLE ADC2_ExtTriggerState**/
   /**< Data alignment right */
   /**< Schmitt trigger disable on AIN10 */
   /**DISABLE ADC2_SchmittTriggerState*/
   ADC1_Init(ADC1_CONVERSIONMODE_SINGLE , ADC1_CHANNEL_4, ADC1_PRESSEL_FCPU_D18,\
   ADC1_EXTTRIG_TIM, DISABLE, ADC1_ALIGN_RIGHT, ADC1_SCHMITTTRIG_CHANNEL4,DISABLE);
   //ADC1_ITConfig(ENABLE);
   
   
#elif CONVERSIONMODE ==CONVERSIONMODE_CONTINUOUS
   /**< Continuous conversion mode */
   /**< Analog channel 10 */
   /**< Prescaler selection fADC2 = fcpu/18 */
   /**< Conversion from Internal TIM TRGO event */
   /** DISABLE ADC2_ExtTriggerState**/
   /**< Data alignment right */
   /**< Schmitt trigger disable on AIN10 */
   /**DISABLE ADC2_SchmittTriggerState*/
   ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS , ADC1_CHANNEL_4, ADC1_PRESSEL_FCPU_D2,\
   ADC1_EXTTRIG_TIM, DISABLE, ADC1_ALIGN_RIGHT, ADC1_SCHMITTTRIG_CHANNEL4,DISABLE);
   //ADC1_ITConfig(DISABLE);
#endif
   
   //ADC1_DataBufferCmd(ENABLE);//启用缓存寄存器存储数据
   ADC1_ITConfig(ADC1_IT_EOCIE,DISABLE);//关闭中断功能
   
   ADC1_Cmd(ENABLE) ;
   ADC1_StartConversion();
}

u16 adc_filter(void)
{
  int i;
  u16 Conversion_Value[9] = {0};
  u16 adc_filter_value = 0;

  for(i=0;i<8;i++)
  {
    Conversion_Value[i] = ADC1_GetConversionValue();
    Conversion_Value[8] += Conversion_Value[i];
  }
  adc_filter_value = Conversion_Value[8]/8;
  
  return adc_filter_value;
}

u16 filter(void)
{
  u8 i;
  total_vaild = 0;
  
  vaild_value[N] = uart_adc_value;
  for(i=0;i<N;i++)
  {
    vaild_value[i] = vaild_value[i+1];   
    total_vaild += vaild_value[i];
  }
  expect_final = total_vaild/N;
  return expect_final;
}

void adc_irq(void)
{
  adc_count ++;
    if(adc_count > 100)
    {
      adc_count = 0;
      filter();

    }
  compute_count ++;
  if(compute_count > compute_rate)
  {
    compute_count = 0;
    compute_display_value();
  }
    return;
}

void send_uart_value(u16 uart_value)
{
  uart_adc_value = uart_value;
  return;
}

static u8 compute_gap(u16 fly_past)
{
  u8 temp;
  if(fly_past > 100)
  {
    temp = 12;
    compute_rate = 70;
  }
  else
  {
    temp = fly_past/10 + 1;
    compute_rate = 200;
  }
  return temp;
}

static void compute_display_value(void)
{
  u8 gap;
  u16 des_value;
  des_value = (u16)(700.0*expect_final/1024*5.0);
  if(des_value > 532)
    des_value = 532;
  if(display_value > des_value)
  {    
    gap = compute_gap(display_value - des_value);
    display_value -= (gap/2 + 1);
  }
  if(display_value < des_value)
  {
    gap = compute_gap(des_value - display_value);
    display_value += gap;
  }
  return;
}

u16 get_PM2_5_value(void)
{
  return display_value;
}
