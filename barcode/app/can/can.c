#include "can.h"

#ifdef CAN_RX0_INT_ENABLE

uint8_t CAN_RX_BUFF[8];

#endif

/****************************************************************************
* Function Name  : CAN1_NVIC_Config
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void CAN1_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 使能接收的中断和中断优先级 */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);   //FIFO0消息挂号中断允许.	
}

/****************************************************************************
* Function Name  : CAN1_Config
* Description    : 初始化CAN，波特率设置为450K
* Input          : mode：用来选择要使用的工作模式：主要有四种工作模式：1、正常
*                * 模式：CAN_Mode_Normal；2、CAN_Mode_Silent ：静默模式；3、环
*                * 回模式：CAN_Mode_LoopBack；4、静默环回模式：CAN_Mode_Silent
*                * _LoopBack。
* Output         : None
* Return         : None
****************************************************************************/

void CAN1_Config(uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;

	/* 初始化IO口 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;        //PA12
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;     //上拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;        //PA11

	GPIO_Init(GPIOA, &GPIO_InitStructure);

/***************************************************************************/
/********************* CAN设置和初始化 *************************************/
/***************************************************************************/

	/* 打开时钟使能 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* 初始化CAN的参数 */

	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN 参数初始化 */
	CAN_InitStructure.CAN_TTCM = DISABLE;    //失能时间触发模式
	CAN_InitStructure.CAN_ABOM = DISABLE;    //失能自动离线管理
	CAN_InitStructure.CAN_AWUM = DISABLE;    //失能睡眠模式通过软件唤醒
	CAN_InitStructure.CAN_NART = DISABLE;    //失能非自动重传输模式（也就是会自动重传输）
	CAN_InitStructure.CAN_RFLM = DISABLE;    //失能接收FIFO锁定模式，新数据会覆盖旧数据
	CAN_InitStructure.CAN_TXFP = DISABLE;    //优先级由报文标识符决定 
	CAN_InitStructure.CAN_Mode = mode;       //有普通模式和拓展模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq; //重新同步跳跃宽度 1 个时间单位

    /* 波特率设置, 当APB1的时钟频率是36MHZ的时候。 波特率的公式为： */
    /* 波特率(Kpbs) = 36M / ((CAN_BS1 + CAN_BS2 + 1) *  CAN_Prescaler) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq; //时间段 1 为8 个时间单位 
	CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq; //时间段 2 为7 个时间单位
	CAN_InitStructure.CAN_Prescaler = 5;	 

	CAN_Init(CAN1, &CAN_InitStructure);

#ifdef CAN_RX0_INT_ENABLE
	CAN1_NVIC_Config();
#endif    

}

/****************************************************************************
* Function Name  : CAN1_SendMesg
* Description    : 发送一个报文
* Input          : id：发送的ID。
*                * len：发送的数据长度(注意发送数据长度不能超过8个字节)
*                * dat：存放数据的指针
* Output         : None
* Return         : None
****************************************************************************/

void CAN1_SendMesg(uint32_t id, uint8_t len, uint8_t *dat)
{
	uint16_t i = 0;
	CanTxMsg TxMessage;

    /* 一次发送只能发送8个字节 */
    if(len > 8)
    {
        return ;
    }
	/* 配置邮箱：设置标识符，数据长度和待发送数据 */
	TxMessage.StdId = (id & 0x7FF); //标准帧ID11位
	TxMessage.ExtId = (id >> 11);   //设置扩展标示符（拓展标示符有29位）
	TxMessage.RTR = CAN_RTR_DATA;   //设置为数据帧（或远程帧为CAN_RTR_Remote）
    if((id & 0x7FF) == 0x7FF)       //检测是标准帧还是拓展帧（拓展帧大于11位）
    {
    	TxMessage.IDE = CAN_ID_STD;	//拓展ID   
    }
    else
    {
    	TxMessage.IDE = CAN_ID_EXT;	//标准ID
    }
	TxMessage.DLC = len;	        //发送的数据长度

	/* 将数据放入到邮箱中 */
	for(i=0; i<len; i++)	         
	{
		TxMessage.Data[i] = *dat;
		dat++;	
	}
    
    /* 开始传送数据 */
	CAN_Transmit(CAN1, &TxMessage); 
}

/****************************************************************************
* Function Name  : CAN1_Config16BitFilter
* Description    : 设置CAN接收16两个标准ID（设置ID位数全部相同才能够通过）
* Input          : id1：要接收的一个ID
*                * id2：要接收的一个ID
* Output         : None
* Return         : None
****************************************************************************/

void CAN1_Config16BitFilter(uint16_t id1, uint16_t id2)
{
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    uint16_t mask = 0xFFFF;

	/* CAN filter init 屏蔽寄存器初始化 */
	CAN_FilterInitStructure.CAN_FilterNumber = 1;	               //过滤器1
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ID模式

	/* 寄存器组设置为16位 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_16bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = (id1 << 5);    //要接收的ID标示符1		
    CAN_FilterInitStructure.CAN_FilterIdLow =  (id2 << 5);	  //要接收的ID标示符2

	/* 设置为所有ID位都要相同才接收 */	
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (mask << 5); //MASK
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  = (mask << 5);
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //使能过滤器1

	CAN_FilterInit(&CAN_FilterInitStructure);
}

/****************************************************************************
* Function Name  : CAN1_Config32BitFilter
* Description    : 设置一个拓展ID的接收
* Input          : id：要接收的ID
* Output         : None
* Return         : None
****************************************************************************/

void CAN1_Config32BitFilter(uint32_t id)
{
    uint32_t mask = 0xFFFFFFFF;
    CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	/* CAN filter init 屏蔽寄存器初始化 */
	CAN_FilterInitStructure.CAN_FilterNumber = 1;	               //过滤器1
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;//ID模式

	/* 寄存器组设置为32位 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = (id >> 13);    //要接收的ID标示符1		
    CAN_FilterInitStructure.CAN_FilterIdLow =  (id << 3 ) | 4;//要接收的ID标示符2

	/* 设置为所有ID位都要相同才接收 */	
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = mask >> 13;     //MASK
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  = (mask << 3) | 4;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //使能过滤器1

	CAN_FilterInit(&CAN_FilterInitStructure);
}

/****************************************************************************
* Function Name  : CAN1_ReceiveMesg
* Description    : 接收一个报文
* Input          : receiveBuff：接收数据的数组指针
* Output         : None
* Return         : None
****************************************************************************/

void CAN1_ReceiveMesg(uint8_t *receiveBuff)
{
	uint8_t i = 0;

	CanRxMsg RxMessage;	//设置接收邮箱

	if((CAN_MessagePending(CAN1, CAN_FIFO0) != 0)) //检查FIFO0里面是否有数据
	{
    	CAN_Receive(CAN1,CAN_FIFO0,&RxMessage); //读取FIFO0里面的数据
    	for(i=0; i<RxMessage.DLC; i++)          //将读取到的数据位赋给CAN_RXSBUF
    	{
    		*receiveBuff = RxMessage.Data[i];
    		receiveBuff++;
    	}
    }			
}

/****************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : FIFO0接收一个报文数据数据（最大8个字节）
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

#ifdef CAN_RX0_INT_ENABLE

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CAN1_ReceiveMesg(CAN_RX_BUFF);	
}

#endif

