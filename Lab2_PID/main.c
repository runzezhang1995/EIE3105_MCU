#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "string.h"

	
unsigned int isOn = 0;

// PID Variables
int target_count_R  = 8;
int target_count_L  = 8;
int wheel_count_R = 0;
int wheel_count_L = 0;

// Kp = 0.95, Ki = 0.01, Kd = 1
float Kp_L        = 0.95;
float Ki_L        = 0.01;
float Kd_L        = 1;
int 	integral_L 	= 0;
int 	lastError_L  = 0;
int 	lastPWM_L = 0;
float PWM_L = 0;
float error_L = 0;
float derivative_L = 0;

// Kp = 1.2, Ki = 0.01, Kd = 1.1
float Kp_R        = 1.2;
float Ki_R        = 0.01;
float Kd_R        = 1.1;
int 	integral_R 	= 0;
int 	lastError_R  = 0;
int 	lastPWM_R = 0;
float PWM_R = 0;
float error_R = 0;
float derivative_R = 0;

void clear_PID(){
		wheel_count_L = 0;
		wheel_count_R = 0;
		integral_L 	= 0;
		integral_R 	= 0;
		error_L = 0;
		error_R = 0;
		lastError_L  = 0;
		lastError_R  = 0;
		PWM_L = 0;
		PWM_R = 0;
		lastPWM_L = 0;
		lastPWM_R = 0;
		derivative_L = 0;
		derivative_R = 0;
}
/*
void PID_Calcul_R(){
	lastError_R = error_R; //Store the last error
	lastPWM_R = PWM_R; //Store the last PWM
	
	error_R = target_count_R - wheel_count_R; // Caculate the present error
	integral_R += error_R; // Sum the error
	derivative_R = error_R - lastError_R;// Caculate the different between the present error and last error
	
	PWM_R = (lastPWM_R + (Kp_R * error_R) + (Ki_R*integral_R) + (Kd_R * derivative_R));
	
	if(PWM_R < 0){
		PWM_R = 0;
	}
	
	TIM3 -> CCR1 = PWM_R;
	wheel_count_R = 0;
}
*/

void PID_Calcul_R(){
	lastError_R = error_R; //Store the last error
	lastPWM_R = PWM_R; //Store the last PWM
	
	error_R = target_count_R - wheel_count_R; // Caculate the present error
	integral_R += error_R; // Sum the error
	derivative_R = error_R - lastError_R;// Caculate the different between the present error and last error
	
	PWM_R = (lastPWM_R + (Kp_R * error_R) + (Ki_R*integral_R) + (Kd_R * derivative_R));
	
	if(PWM_R < 0){
		PWM_R = 0;
	}
	
	TIM3 -> CCR1 = PWM_R;
	wheel_count_R = 0;
}

void PID_Calcul_L(){
	lastError_L = error_L; //Store the last error
	lastPWM_L = PWM_L; //Store the last PWM
	
	error_L = target_count_L - wheel_count_L; // Caculate the present error
	integral_L += error_L; // Sum the error
	derivative_L = error_L - lastError_L; // Caculate the different between the present error and last error
	
	PWM_L = (lastPWM_L + (Kp_L * error_L) + (Ki_L*integral_L) + (Kd_L * derivative_L));
	
	if(PWM_L < 0){
		PWM_L = 0;
	}

	TIM3 -> CCR2 = PWM_L;
	wheel_count_L = 0;
}

/*
char data_from_floor;
void SPI2_IRQHandler(){
	if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET) {
		data_from_floor = SPI_I2S_ReceiveData(SPI2);
		SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_FLAG_RXNE);
	}
}
*/

// Right Wheel Counter EXTI
void EXTI1_IRQHandler(){
	if (EXTI_GetITStatus(EXTI_Line1) != RESET){
		wheel_count_R++;
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

unsigned int pressed = 0;
// Left Wheel Counter EXTI
void EXTI9_5_IRQHandler(){
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET){
		wheel_count_L++;
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	if(EXTI_GetITStatus(EXTI_Line8)!= RESET){
		clear_PID(); // Clear all PID variables
		if(pressed==0){
			if(isOn == 0){
				TIM3 -> CCR1 = 6; // Give the right wheel a initial speed ???
				isOn = 1;
			}
			else{
				isOn = 0;
				// Set the speed to 0
				TIM3 -> CCR1 = 0;
				TIM3 -> CCR2 = 0;
			}
		}
		pressed ++;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	else{
			pressed = 0;
	}
}

int flag = 0;
int main(){
	TIM3_CH12_PWM_init();
	TIM2_Init();
	Left_Wheel_Cnt_init();
	Right_Wheel_Cnt_init();
	Wheel_Dir_Init();
	Button_init();
	OnBoard_lED_Init();
	
	SPI2_init();
	USART2_init();
	
	while(1){
		if(flag == 1){
			PID_Calcul_R();
			PID_Calcul_L();
			flag=0;
		}
	}
}
void TIM2_IRQHandler(){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		
		GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET); // On-board LED is always on
		if(isOn == 1){
			flag = 1;
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
