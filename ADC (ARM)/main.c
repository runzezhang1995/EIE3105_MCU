#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdio.h"
#include "misc.h"

int main(void) {
	
	char buffer[50] = {'\0'};
	int adc_value;
	
	USART2_init();
	ADC1_1channel_init();
	TIM3_CH1_PWM_init();
	
	// start conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	// start conversion (will be endless as we are in continuous mode)
	
	
	//For debugging********************************************
	GPIO_InitTypeDef GPIO_InitStruct;
	// Configure I/O for LD2
	// GPIO clock for LD2
	RCC_APB2PeriphClockCmd(LD2_RCC_GPIO, ENABLE);
	
	// Configure I/O for LD2
	GPIO_InitStruct.GPIO_Pin = LD2_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LD2_GPIO, &GPIO_InitStruct);
	GPIO_WriteBit(LD2_GPIO, LD2_PIN, Bit_RESET);
	//********************************************************
	
	
	while(1) {
		
		while( ADC_GetFlagStatus( ADC1, ADC_FLAG_EOC ) == RESET ) {
			//Test if ADC_GetFlagStatus works*************************
			//GPIO_WriteBit(LD2_GPIO, LD2_PIN, Bit_SET);
			//********************************************************
		}
	
		adc_value = ADC_GetConversionValue(ADC1);
		TIM_SetCompare1(TIM3, adc_value);
		sprintf(buffer, "%d\r\n", adc_value);
		USARTSend(buffer, sizeof(buffer));
	}
}
