#include "adctemp.h"

/****************************************************************************
* Function Name  : ADCTEMP_Config
* Description    : 初始化内部ADC
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void ADCTEMP_Config(void)
{    
	ADC_InitTypeDef  ADC_InitStructure;

	/* 打开时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);           //72M/6=12,ADC 最大时钟不能超过 14M
    
    ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值

   	/* 初始化ADC参数 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;          //工作方式为独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	            //不使用扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;         //不使用连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//软件触发模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;      //数据对齐方式是右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;        //顺序进行规则转换的ADC 通道的数目1 

	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_TempSensorVrefintCmd(ENABLE);   //开启内部温度传感器
    ADC_Cmd(ADC1, ENABLE);              //打开ADC使能

	ADC_ResetCalibration(ADC1);                  //开启复位校准    
	while(ADC_GetResetCalibrationStatus(ADC1));  //等待复位校准结束 

	ADC_StartCalibration(ADC1);                  //开启AD校准 
	while(ADC_GetCalibrationStatus(ADC1));       //等待校准结束        
   
}

/****************************************************************************
* Function Name  : ADCTEMP_GetAdc
* Description    : 读取温度传感器的AD值
* Input          : None
* Output         : None
* Return         : 返回读取到的AD值
****************************************************************************/

static uint16_t ADCTEMP_GetAdc(void)
{
    uint16_t value;

    /* 温度传感器是通道16，规则采样顺序值为1，采样时间为 239.5周期。 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5 );  
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1 的软件转换功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	/* 读取ADC1规则组的转换结果 */	
	value = ADC_GetConversionValue(ADC1); 
    
    return value;
}

/****************************************************************************
* Function Name  : ADCTEMP_GetTemp
* Description    : 读取温度值
* Input          : None
* Output         : None
* Return         : 返回读取到的温度值（放大100倍，保留两位小数点）
****************************************************************************/

int16_t ADCTEMP_GetTemp(void)
{
    uint32_t adValue = 0;
    int16_t tempValue;
    float adVoltage;
    uint8_t time;
    
    /*  读取十次然后求出平均值 */
    for(time = 0; time < 10; time++)
    {
        adValue += ADCTEMP_GetAdc();
    }
    adValue = adValue / 10;

    /* 将AD值转换为电压值 */
    adVoltage = (float)adValue * 3.3 / 4096;

    /* 根据公式求出温度值 */
    adVoltage = ((float)(ADCTEMP_Vsence25 - adVoltage) / ADCTEMP_AvgSlope) + 25; 
    tempValue = adVoltage * 100;   //保留两位小数点

    return tempValue;
}







