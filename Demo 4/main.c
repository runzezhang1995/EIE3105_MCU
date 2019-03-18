#include "stm32f10x.h"                  // Device header
#include "init.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"

typedef struct
{
	uint16_t x;
	uint16_t y;
}Pos;

Pos ball,car;
Pos prevBall,prevCar;
int PrevErr2,PrevErr,Err,Integral=0,Derivative=0,iProportion_Term,out, count;


void RCC_init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_USART2,ENABLE);
}

void LEDs_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_7);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);	//Reserved for further use
}

void USART2_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate=115200;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART2,&USART_InitStructure);
	USART_Cmd(USART2,ENABLE);
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI_Line8_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
	EXTI_InitStruct.EXTI_Line=EXTI_Line8;
	EXTI_InitStruct.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Wheels_PWM_Phase_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef timerInitStructure;
	TIM_OCInitTypeDef outputChannelInit;
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	timerInitStructure.TIM_Prescaler=144-1;
	timerInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	timerInitStructure.TIM_Period=1000-1;
	timerInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM3,&timerInitStructure);
	TIM_Cmd(TIM3,ENABLE);
		
	outputChannelInit.TIM_OCMode=TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse=1000-1;
	outputChannelInit.TIM_OutputState=TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OC1Init(TIM3,&outputChannelInit);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	TIM_OC2Init(TIM3,&outputChannelInit);
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	GPIO_ResetBits(GPIOC,GPIO_Pin_15);
}

void UARTSend(char *pucBuffer,uint32_t ulCount)
{
  while(ulCount--)
  {
		USART_SendData(USART2,*pucBuffer++);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
  }
}



int main()
{
	RCC_init();
	LEDs_init();
	USART2_init();
	EXTI_Line8_init();
	Wheels_PWM_Phase_init();
	
	count = 0;
	car.x=0;
	car.y=0;
	ball.x=0;
	ball.y=0;
	Wheel('left',0);
	Wheel('right',0);
		
	delay(7000);
	UARTSend("AT+CIPSTART=\"UDP\",\"0\",0,3105,2\r\n",40);
	delay(100);
	
	while(1)
	{
		int mode=0;
		if (count ==14)
		{
			if(mode==0)
			{
				GPIO_SetBits(GPIOB,GPIO_Pin_7);
				mode=1;
			}
			else
				GPIO_ResetBits(GPIOB,GPIO_Pin_7);
				mode = 0;
			count = 0;
		}
		count++;
			
		if(ball.x>490)
		{
			if(car.x>170)
			{
				while (car.x < 200)
					ball.x ++;
			}
			Wheel('right',0);
			Wheel('left',0);	
		}
		else
		{
			//GPIO_ResetBits(GPIOB,GPIO_Pin_7);
			Wheel('r',0);
			Wheel('l',0);	
				
				
			}
		}
	}


void EXTI9_5_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line8)==SET)
	{
		uint16_t left,right;
		right=TIM_GetCapture1(TIM3);
		left=TIM_GetCapture2(TIM3);
		TIM_SetCompare1(TIM3,0);	//Right wheel
		TIM_SetCompare2(TIM3,0);	//Left wheel
		//GPIO_ResetBits(GPIOB,GPIO_Pin_7);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==SET);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==RESET);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==SET);
		//GPIO_SetBits(GPIOB,GPIO_Pin_7);
		TIM_SetCompare1(TIM3,right);	//Right wheel
		TIM_SetCompare2(TIM3,left);	//Left wheel
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}

void USART2_IRQHandler()
{
	char ch;
	static char str[23];
	static uint8_t count;
	static char object[3];
	char XBuffer[4];
	char YBuffer[4];
	
	XBuffer[3]='\0';
	XBuffer[3]='\0';
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
	{
		ch=USART_ReceiveData(USART2);
		str[count]=ch;
		
		prevBall.x=ball.x;
		prevBall.y=ball.y;
		
		prevCar.x=car.x;
		prevCar.y=car.y;
		
	}	
	
	USART_ClearITPendingBit(USART2,USART_IT_RXNE);
}
