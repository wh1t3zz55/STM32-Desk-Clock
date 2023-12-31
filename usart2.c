#include "sys.h"
#include "usart2.h"	  
#include <string.h>
#include <stdlib.h>
#include "rtc.h" 
#include "dht11.h"
void uart2_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  
	//USART2_TX   GPIOA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
   
  //USART2_RX	  GPIOA.3初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

  //USART2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断（一个字节中断）
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//使能空闲中断（一帧数据中断）
  USART_Cmd(USART2, ENABLE);                    //使能串口1 

}

static volatile uint8_t  g_USART2_buf[128]={0};
static volatile uint32_t g_USART2_cnt=0;
void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	uint8_t d=0;
	int i = 0;
	u16 alarm_buf[64]={0};
	u16 rtc_buf[64]={0};
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//接收串口数据
		d = USART_ReceiveData(USART2);	
		
		g_USART2_buf[g_USART2_cnt] = d;
		
		g_USART2_cnt++;

		//设置闹钟
		
		//设置时间
		if(d == 'R'|| g_USART2_cnt>= sizeof(g_USART2_buf))
		{ 		
			char *s = strtok((char *)g_USART2_buf,"- :");  //分割符是- :
			 while(s!=NULL)
			{   
			    rtc_buf[i] = atoi(s);    //2022-1-10 23:50:5R
				i++;
				s = strtok(NULL,"- :");
				g_USART2_cnt = 0;
			}
			RTC_Set(rtc_buf[0],rtc_buf[1],rtc_buf[2],rtc_buf[3], rtc_buf[4],rtc_buf[5]); //设置时间
			
			printf("%d-%d-%d %d:%d:%d  TimeSet！\r\n",rtc_buf[0],rtc_buf[1],rtc_buf[2],rtc_buf[3],rtc_buf[4],rtc_buf[5]);
		}	
		//清空串口接收中断标志位
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		//RTC_Init_LSE();
		//dht11_on();	
		
		
	} 
	else if(USART_GetITStatus(USART2,USART_IT_IDLE) != RESET)
		{
		USART2->SR;//先读SR
		USART2->DR;//再读DR

	}

} 

