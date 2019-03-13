#include "stm32f10x.h"                  // Device header


//500 HZ PWM and Duty cycle is 50%

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
	OC_InitStruct.TIM_Pulse = 500-1;
	OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
	OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3,&OC_InitStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
}


int main(void) {
	TIM3_CH1_PWM_init();
	while(1);
	return 0;
}

