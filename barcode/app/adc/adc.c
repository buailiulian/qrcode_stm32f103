#include "adc.h"

/****************************************************************************
* Function Name  : ADC_Config
* Description    : 初始化ADC
* Input          : adc：选择初始化的ADC，（我们开发板上面有三个ADC输入）
* Output         : None
* Return         : None
****************************************************************************/

void ADC_Config(uint8_t adc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;

/***************************************************************************/
/********************** IO口模式配置 ***************************************/
/***************************************************************************/

    if((adc & ADC_Chn0) == ADC_Chn0) //如果选择打开ADC0
    {
        /* 打开时钟 */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;          //PB0
    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;      //模拟输入
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    if((adc & ADC_Chn1) == ADC_Chn1)
    {
        /* 打开时钟 */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;          //PB1
    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;      //模拟输入
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    if((adc & ADC_Chn2) == ADC_Chn2)
    {
        /* 打开时钟 */
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;          //PA1
    	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;      //模拟输入
    	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        GPIO_Init(GPIOA, &GPIO_InitStructure); 
    }

/***************************************************************************/
/*********************** ADC参数设置 ***************************************/
/***************************************************************************/

	/* 打开时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);           //72M/6=12,ADC 最大时钟不能超过 14M

	/* 初始化ADC参数 */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;          //工作方式为独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	            //不使用扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;         //不使用连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//软件触发模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;      //数据对齐方式是右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;        //顺序进行规则转换的ADC 通道的数目1 

	ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Cmd(ADC1, ENABLE);                       //打开ADC使能

	ADC_ResetCalibration(ADC1);                  //开启复位校准    
	while(ADC_GetResetCalibrationStatus(ADC1));  //等待复位校准结束 

	ADC_StartCalibration(ADC1);                  //开启AD校准 
	while(ADC_GetCalibrationStatus(ADC1));       //等待校准结束        
}

/****************************************************************************
* Function Name  : ADC_ReadChn0
* Description    : 读取一次ADC0的转换结果
* Input          : None
* Output         : None
* Return         : 一个12位的转换结果
****************************************************************************/

uint16_t ADC_ReadChn0(void)
{
    uint16_t Value;

    /* ADC0是PB0是通道8，规则采样顺序值为1，采样时间为 239.5周期。 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5 );  
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1 的软件转换功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	/* 读取ADC1规则组的转换结果 */	
	Value = ADC_GetConversionValue(ADC1); 
    
    return Value;
}

/****************************************************************************
* Function Name  : ADC_ReadChn1
* Description    : 读取一次ADC1的转换结果
* Input          : None
* Output         : None
* Return         : 一个12位的转换结果
****************************************************************************/

uint16_t ADC_ReadChn1(void)
{
    uint16_t Value;

    /* ADC0是PB1是通道9，规则采样顺序值为1，采样时间为 239.5周期。 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5 );  
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1 的软件转换功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	/* 读取ADC1规则组的转换结果 */	
	Value = ADC_GetConversionValue(ADC1); 
    
    return Value;
}

/****************************************************************************
* Function Name  : ADC_ReadChn2
* Description    : 读取一次ADC2的转换结果
* Input          : None
* Output         : None
* Return         : 一个12位的转换结果
****************************************************************************/

uint16_t ADC_ReadChn2(void)
{
    uint16_t Value;

    /* ADC0是PA1是通道1，规则采样顺序值为1，采样时间为 239.5周期。 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5 );  
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);        //使能指定的ADC1 的软件转换功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	/* 读取ADC1规则组的转换结果 */	
	Value = ADC_GetConversionValue(ADC1); 
    
    return Value;
}

