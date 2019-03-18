#include "stm32f10x.h"                  // Device header

void RCC_init();
void LEDs_init();
void USART2_init();
void EXTI_Line8_init();
void Wheels_PWM_Phase_init();
void UARTSend(char *pucBuffer,uint32_t ulCount);
void delay(uint32_t time_in_ms);
void Wheel(char wheel,int pulse);
