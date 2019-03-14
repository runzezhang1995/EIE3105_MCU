#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdbool.h"
#include "stdio.h"

int count = 0;

int current_time_in_s_over_ten = 0;

int leftSpeed = 0;
int rightSpeed = 0;

int wheel_count_left = 0;
int wheel_count_right = 0;

int wheel_total_count_left = 0;
int wheel_total_count_right = 0;

int error = 0;


float Kp = 0.9;
float Ki = 1;
float Kd = 1;


bool operating = false;




void setLeftWheelDirection(bool forward);
void setRightWheelDirection(bool forward);
void setLeftWheelSpeed(int percent);
void setRightWheelSpeed(int percent);

void startOperating(bool operate);
void PID_Control(int l_c, int r_c);

void init(){
	TIM3_PWM_init();
	EXTI_wheel_counter_init();
	
	USART2_init();
	Wheel_Dir_Init();
	User_Button_Init();
	Board_LED_Init();
	TIM2_Init();
}

int main(void) {
		
	init();
					GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // On-board LED is turn off if not same speed
	count = 0;
	setLeftWheelDirection(true);
	setRightWheelDirection(true);

	
	while(1) {
	}
	
}



char buffer[50] = {'\0'};
bool pulseHigh = false;
u32 pulseWidth = 0;
int duty_cycle = 1000 - 1;



void TIM4_IRQHandler(void) {
	//TIM_SetCompare1(TIM3, 0);
	if(TIM_GetITStatus(TIM4,TIM_IT_CC1) != RESET) {
		if(!pulseHigh) {
			//TIM_SetCompare1(TIM3, 0);
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
		}
	}
  TIM_ClearITPendingBit(TIM4,TIM_IT_Update|TIM_IT_CC1); //Clear interrupt flag
}



unsigned int pressed = 0;
// Left Wheel Counter EXTI
void EXTI9_5_IRQHandler(){
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET){
		wheel_count_left++;
		wheel_total_count_left++;
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	if(EXTI_GetITStatus(EXTI_Line8)!= RESET){
		startOperating(!operating);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}



void EXTI1_IRQHandler(){
	if (EXTI_GetITStatus(EXTI_Line1) != RESET){
		wheel_count_right++;
		wheel_total_count_right++;
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

void TIM2_IRQHandler(){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // On-board LED is always on
		current_time_in_s_over_ten ++;
		
		if(current_time_in_s_over_ten % 10 == 0) {
			sprintf(buffer, "time is: %d\r\n", current_time_in_s_over_ten);
			USARTSend(buffer, sizeof(buffer));	
			
			sprintf(buffer, "left counter: %d\r\n", wheel_total_count_left);
			USARTSend(buffer, sizeof(buffer));
				
			sprintf(buffer, "right counter: %d\r\n", wheel_total_count_right);
			USARTSend(buffer, sizeof(buffer));
		
		}
			
		PID_Control(wheel_count_left, wheel_count_right);
		
		
		wheel_count_left = 0;
		wheel_count_right = 0;
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}









void setLeftWheelDirection(bool forward) {
	GPIO_WriteBit(WHEEL_DIR_LEFT_GPIO, WHEEL_DIR_LEFT_PIN, forward? Bit_SET: Bit_RESET);
}


void setRightWheelDirection(bool forward) {
	GPIO_WriteBit(WHEEL_DIR_RIGHT_GPIO, WHEEL_DIR_RIGHT_PIN, forward? Bit_SET: Bit_RESET);
}

void setLeftWheelSpeed(int percent) {
	int dutyCycle = percent * 1 - 1;
	dutyCycle = dutyCycle >= 0 ? dutyCycle : 0;
	TIM3 -> CCR2 =  dutyCycle;
}


void setRightWheelSpeed(int percent) {
	int dutyCycle = percent * 1 - 1;
	dutyCycle = dutyCycle >= 0 ? dutyCycle : 0;
	TIM3 -> CCR1 =  dutyCycle;
}

void startOperating(bool operate){
	if(operate) {
		USARTSend("on\r\n", sizeof("on\r\n"));
		
		
		current_time_in_s_over_ten = 0;

		leftSpeed = 30;
		rightSpeed = 35;

		wheel_count_left = 0;
		wheel_count_right = 0;

		wheel_total_count_left = 0;
		wheel_total_count_right = 0;
		
		setLeftWheelSpeed(leftSpeed);
		setRightWheelSpeed(rightSpeed);
		
		
	} else {
		USARTSend("off\r\n", sizeof("off\r\n"));
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
		GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // On-board LED is turn off if not same speed

	}
	operating = !operating;
	
}

void PID_Control(int l_c, int r_c) {
	// calculate error of double side
	
	if(operating) {
		
		if(current_time_in_s_over_ten > 20 && wheel_count_left == 0) {
			startOperating(false);
		}
		
		
		int lastError = error;
		
		error = l_c - r_c ;
		
		int integratedError = wheel_total_count_left - wheel_total_count_right;
		int differenciateError = error - lastError;
		int totalError= (int)(error +  integratedError / current_time_in_s_over_ten);

		
		if(totalError == 0) {
				GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // On-board LED is turn off if not same speed
				return;
		} else {
				GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET); // On-board LED is trun on if same speed 
		}
		
		
		
		leftSpeed = leftSpeed - totalError;
		leftSpeed = leftSpeed < 20 ? 20: leftSpeed > 60? 60: leftSpeed;
		rightSpeed = rightSpeed + totalError;
		rightSpeed = rightSpeed < 20 ? 20: rightSpeed > 60? 60: rightSpeed;

		
	
			sprintf(buffer, "time have passed: %d\r\n", current_time_in_s_over_ten);
		//	USARTSend(buffer, sizeof(buffer));
									
			sprintf(buffer, "left counter: %d\r\n", l_c);
		//	USARTSend(buffer, sizeof(buffer));
				
			sprintf(buffer, "right counter: %d\r\n", r_c);
		//	USARTSend(buffer, sizeof(buffer));
			
			sprintf(buffer, "left speed: %d\r\n", leftSpeed);
			//USARTSend(buffer, sizeof(buffer));
				
			sprintf(buffer, "right speed: %d\r\n", rightSpeed);
		//	USARTSend(buffer, sizeof(buffer));
			
			sprintf(buffer, "error: %d\r\n", totalError);
			USARTSend(buffer, sizeof(buffer));
			USARTSend("=================", sizeof("================="));
			
		
		setLeftWheelSpeed(leftSpeed);
		setRightWheelSpeed(rightSpeed);
		
	} else {
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
	}
}



