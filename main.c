#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdbool.h"
#include "stdio.h"

int count = 0;

int main(void) {
		
	TIM4_CH1_IC1_init();
  TIM3_CH1_PWM_init();
  USART2_init();
	count = 0;
	while(1) {
	}
	
}



char buffer[50] = {'\0'};
bool pulseHigh = false;
u32 pulseWidth = 0;

void TIM4_IRQHandler(void) {
	TIM_SetCompare1(TIM3, 0);
	if(TIM_GetITStatus(TIM4,TIM_IT_CC1) !=RESET) {
		if(!pulseHigh) {
			
			pulseHigh=true;        //pulse starts
			TIM_SetCounter(TIM4,0);
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling); //change to detect falling
		} else {
			pulseWidth += TIM_GetCounter(TIM4);
			TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //change to detect raising
			sprintf(buffer, "%d\r\n", pulseWidth);
		  USARTSend(buffer, sizeof(buffer));
			pulseHigh= false;
			pulseWidth=0;
			TIM_SetCompare1(TIM3, 0);
		}
	}
  TIM_ClearITPendingBit(TIM4,TIM_IT_Update|TIM_IT_CC1); //Clear interrupt flag
}
