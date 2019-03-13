#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "stdio.h"
#include "string.h"

void TIM3_CH1_PWM_init(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//Tim3 Set up
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_Prescaler = 72-1; // 1/(72MHz/72) = 0.1ms
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 2000-1; // 0.1 * 2000 = 200ms
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&TIM_InitStruct);
	TIM_Cmd(TIM3,ENABLE);
	
	//Enable Tim3 Ch1 PWM
	TIM_OCInitTypeDef OC_InitStruct;
	OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OC_InitStruct.TIM_Pulse = 1000 - 1;
	OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
	OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3,&OC_InitStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
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

	// Enable the USART2 RX Interrupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
}

void USARTsend(char *pucBuffer, unsigned long ulCount){
	while(ulCount--){
		USART_SendData(USART2,*pucBuffer++);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
	}
}


char buffer[100];
char data0,data1;
int tmp0, tmp1, pulse_width,width;
char ready[] = "Received number from USART: ";
int countNum = 0;
// USART Interrupt Handler
void USART2_IRQHandler(){
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET && countNum == 0){
		data0 = (char) USART_ReceiveData(USART2);
		countNum = 1;
	}
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET && countNum == 1){
		data1 = (char) USART_ReceiveData(USART2);
		countNum = 2;
	}
	if(countNum == 2){
		tmp0 = (int)(data0-'0');
		tmp1 = (int)(data1-'0');
		width = tmp0*10 + tmp1;
			
		pulse_width = (int)(width*0.01*2000);
		TIM3-> CCR1 = pulse_width; // Change the pulse_width
			
		USARTsend(ready,sizeof(ready));
		sprintf(buffer, "%d\r\n", width);
		USARTsend(buffer,sizeof(buffer));
		countNum = 0;
	}
}

int main(){
	TIM3_CH1_PWM_init();
	USART2_init();
	while(1){
	}
}