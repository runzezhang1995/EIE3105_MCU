#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

// Input Capture
void TIM4_CH1_IC1_init(){
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//Tim4 Setup
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_Prescaler = 72-1;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStruct.TIM_Period = 20000-1;
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_InitStruct);
	TIM_Cmd(TIM4,ENABLE);
	
	//Enable Tim4 Ch1 Input Capture
	TIM_ICInitTypeDef IC_InitStruct;
	IC_InitStruct.TIM_Channel = TIM_Channel_1; // Select IC1
	IC_InitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising; // Capture Rising
	IC_InitStruct.TIM_ICSelection =  TIM_ICSelection_DirectTI; // Map to T1
	IC_InitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1; // Configure Input Frequency
	IC_InitStruct.TIM_ICFilter = 0;
	TIM_ICInit(TIM4, &IC_InitStruct);
	
	//Enable Input Capture Interrupt
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1, ENABLE);// Allow updates to interrupt, allows the CC1IE to capture interrupt
	
}

void USART2_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	// USART2 ST-LINK USB
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // Omit
	USART_InitStruct.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;
	USART_Init(USART2,&USART_InitStruct);
	USART_Cmd(USART2, ENABLE);
}

void USARTsend(char *pucBuffer, unsigned long ulCount){
	while(ulCount--){
		USART_SendData(USART2,*pucBuffer++);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
	}
}

// Input Capture Interrupt Handler
u32 pulseWidth = 0;
bool pulseHigh = false;
char buffer[100];
char result[] = "The duty cycle of the PWM from AVR is: ";
void TIM4_IRQHandler(){
	if(TIM_GetITStatus(TIM4,TIM_IT_CC1) != RESET){
		if(!pulseHigh){
			pulseHigh = true;
			TIM_SetCounter(TIM4,0);
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling); // change to detect 
		}
		else{
			pulseWidth += TIM_GetCounter(TIM4);
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising);
			pulseWidth = (int)(pulseWidth*0.05 * 1.27);
			USARTsend(result,sizeof(result));
			sprintf(buffer, "%d\r\n", pulseWidth);
			USARTsend(buffer,sizeof(buffer));
			pulseWidth = 0;
			pulseHigh = false;
		}
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update|TIM_IT_CC1);
}

int main(){
	TIM4_CH1_IC1_init();
	USART2_init();
	while(1){
	}
}
