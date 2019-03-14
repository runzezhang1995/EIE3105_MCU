#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdbool.h"
#include "stdio.h"
#define debug 0

int currentTime = 0;

int leftSpeed = 0;
int rightSpeed = 0;

int wheel_count_left = 0;
int wheel_count_right = 0;

int wheel_total_count_left = 0;
int wheel_total_count_right = 0;

int edgeDetectorTimer = 0;
int integrateTimer = 0;
int speedSwitchTimer = 0;

int forceRightStartTimer = 0;
int forceRightEndTimer = 0;


int lineResult[8];

int lastLineDetectedNums[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
int lastLineDetectedPointer = 0;

int edgePassed = 0;

int lineCalculator[8] = {24,16,8,0, 0,-8,-16,-24};
float multiplierIndex[9] = {1, 1, 0.5, 0.333, 0.25, 0.33, 0.5, 1, 1};




bool shouldReadTransistor = false;
bool haveReadTransistor = false;
bool ledIsOn = true;
bool shouldSpeedSwich = false;
bool highSpeedMode = false;
bool isRightGoInner = false;
bool shouldForceRight = false;

float Kp = 2.80;

float Ki = .6;
float Kd = 1.1;

int accumulateError = 0;
int previousError = 0;


bool operating = false;
char buffer[50] = {'\0'};



void setLeftWheelDirection(bool forward);
void setRightWheelDirection(bool forward);
void setLeftWheelSpeed(int percent);
void setRightWheelSpeed(int percent);
void setOnBoardLED(bool isOn);

void startOperating(bool operate);
void PID_Control(int l_c, int r_c);
void lineTrackControl(void);
void readLineArray(void);

void init(){
	TIM3_PWM_init();
	EXTI_wheel_counter_init();
	
	USART2_init();
	Wheel_Dir_Init();
	User_Button_Init();
	Board_LED_Init();
	TIM2_Init();
	SPI2_Init();
	
}

int main(void) {
		
	init();
	setLeftWheelDirection(true);
	setRightWheelDirection(true);
	
	startOperating(false);
	
	while(1) {
		
		if(shouldReadTransistor){
			readLineArray();
			shouldReadTransistor = false;
			haveReadTransistor = true;
		}
		
		
	}
	
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
		currentTime ++;
		
		if(debug){
			sprintf(buffer, "%d\r\n", currentTime);
			USARTSend(buffer, sizeof(buffer));
		}
		
		if(shouldReadTransistor == false) {
			shouldReadTransistor = true;
		}
		
		if(haveReadTransistor){
			lineTrackControl();
			haveReadTransistor = false;
		}
		
		
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
		if(percent < 0)  {
		setLeftWheelDirection(false);
		percent *= -1;
	} else{
		setLeftWheelDirection(true);
	}
	
	
	int dutyCycle = percent * 1 - 1;
	dutyCycle = dutyCycle >= 0 ? dutyCycle : 0;
	TIM3 -> CCR2 =  dutyCycle;
}


void setRightWheelSpeed(int percent) {
	if(percent < 0)  {
		setRightWheelDirection(false);
		percent *= -1;
	} else{
		setRightWheelDirection(true);
	}
	
	int dutyCycle = percent * 1 - 1;
	
	dutyCycle = dutyCycle >= 0 ? dutyCycle : 0;
	TIM3 -> CCR1 =  dutyCycle;
}




void startOperating(bool operate){
	if(operate) {
		USARTSend("on\r\n", sizeof("on\r\n"));
		
		
		currentTime = 0;
		edgeDetectorTimer = 0;
		integrateTimer = 0;
		speedSwitchTimer = 0;
		forceRightStartTimer = 0;
		forceRightEndTimer = 200;
		
		
		leftSpeed = 40;
		rightSpeed = 45;

		edgePassed = 0;
		
		wheel_count_left = 0;
		wheel_count_right = 0;

		wheel_total_count_left = 0;
		wheel_total_count_right = 0;
		
		accumulateError = 0;
		previousError = 0;
		
		
 	 	shouldReadTransistor = false;
	 	haveReadTransistor = false;
	 	shouldSpeedSwich = false;
	 	highSpeedMode = false;
	 	isRightGoInner = false;
	 	shouldForceRight = true;

		
		setLeftWheelSpeed(leftSpeed);
		setRightWheelSpeed(rightSpeed);
		
		setOnBoardLED(false);
		
		
		
	} else {
		USARTSend("off\r\n", sizeof("off\r\n"));
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
		setOnBoardLED(true);
	}
	operating = operate;
	
}


void setOnBoardLED(bool isOn){
	GPIO_WriteBit(GPIOB, GPIO_Pin_7, isOn? Bit_SET:Bit_RESET); // On-board LED is turn off if not same speed
}


bool isBlackContinuous(void){
	int i = 0;
	int j = 7;
	
	while(lineResult[i] == 1 && i <= 7) {i++;}
	while(lineResult[j] == 1 && j >= 0) {j--;}
	if(i == j) return true;
	while(lineResult[i] == 0 && i < j) i++;
	if(i < j) return false;
	else return true;
}

void followRightDetection(void){
	int detectedTrans = 0;
		
		for(int i = 0; i < 8; i ++) {
			if(lineResult[i] == 0) {
				detectedTrans += 1;
			}	 
		}
	if (detectedTrans <= 3) {
	return;
	}
	
	int i = 0;
	while (lineResult[i] == 1 && i <= 7) {i++;}
	if(i == 8) return;
	int j =  (i + 2 ) > 7 ? 7 : (i + 2);	
	while (i < j) {
		lineResult[i] = 0;
		i++;
	} 
	while(i <= 7){
		lineResult[i] = 1;
		i++;
	}
}

void followLeftDetection(void){
	int detectedTrans = 0;
		
		for(int i = 0; i < 8; i ++) {
			if(lineResult[i] == 0) {
				detectedTrans += 1;
			}	 
		}
	if (detectedTrans <= 3 || detectedTrans >= 7) {
	return;
	}
	
	int i = 7;
	while (lineResult[i] == 1 && i >= 0) {i--;}
	if(i == -1) return;
	int j =  (i - 2 ) <  0 ? 0 : (i - 2);	
	while (i > j) {
		lineResult[i] = 0;
		i--;
	} 
	while(i >= 0){
		lineResult[i] = 1;
		i--;
	}
}


void handleEdgePassing(void){
		
			edgeDetectorTimer = currentTime;
			accumulateError = 0;
			integrateTimer = currentTime;
			edgePassed += 1;
			setOnBoardLED(edgePassed % 2);
			//return;
			switch(edgePassed){
				case 1:
					forceRightStartTimer = currentTime ;
					forceRightEndTimer = currentTime + 40;
					shouldForceRight = true;
					break;
				case 2:
					forceRightStartTimer = currentTime ;
					forceRightEndTimer = currentTime + 180;
					shouldForceRight = true;
					highSpeedMode = true;
					Kp = 2.5;
					break;
				case 4:
					forceRightStartTimer = currentTime + 30;
					forceRightEndTimer = currentTime + 230;
					shouldForceRight = true;
					break;
				case 5:
					Kp = 2.8;
					break;
				case 6:
					forceRightStartTimer = currentTime + 140;
					forceRightEndTimer = currentTime + 150;
					shouldForceRight = true;
					break;
				case 7:
					forceRightStartTimer = currentTime;
					forceRightEndTimer = currentTime + 500;
					shouldForceRight = true;	
					highSpeedMode = false;
					wheel_count_left = 0;
					Kp = 2.8;
					break;
				case 8:
					forceRightStartTimer = currentTime;
					forceRightEndTimer = currentTime + 400;
					shouldForceRight = true;
					
					break;
				case 9:
					Kp = 2.5;
					break;
				
				case 13:
					forceRightStartTimer = currentTime;
					forceRightEndTimer = currentTime + 80;
					//shouldForceRight = true;
					break;
				default:
					//speedSwitchTimer = currentTime;
					//shouldSpeedSwich = true;
					break;
			}
}

void lineTrackControl(void) {
	if(operating) {
		// stop if stack
		if(currentTime > 500 && (wheel_count_left + wheel_count_right) == 0) {
			//startOperating(false);
		}
		


		// speed switch 
		/*
		if(shouldSpeedSwich && (currentTime - speedSwitchTimer) > (100 + highSpeedMode * 50) ) {
			highSpeedMode = !highSpeedMode;

			//Kp = 3.0 - highSpeedMode * 0.8;
			//Ki = 0.5 + highSpeedMode * 0.4;
			//Kd = 0 + highSpeedMode * 1;

			setOnBoardLED(highSpeedMode);
			shouldSpeedSwich = false;
		}
		*/

		
		
		
		if((shouldForceRight == true) && currentTime > forceRightStartTimer && currentTime < forceRightEndTimer) {
			switch(edgePassed) {
				case 0:
						followRightDetection();
				
				break;
				case 1:
					setLeftWheelSpeed(85);
					setRightWheelSpeed(15);
					break;
				case 2:
					followRightDetection();
				
					break;				
				
				case 4:
					followLeftDetection();
					break;
				case 6: 
					setLeftWheelSpeed(0);
					setRightWheelSpeed(0);
					break;
				case 7:
					setLeftWheelSpeed(-65);
					setRightWheelSpeed(65);
					if(wheel_count_left == 83) {
						forceRightEndTimer = currentTime;
					}
				
					break;	
				case 8:
					followLeftDetection();

					//setLeftWheelSpeed(80);
					//setRightWheelSpeed(30);
					break;
				
				case 13:
					setLeftWheelSpeed(0);
					setRightWheelSpeed(0);	
					break;
				default:
					break;
				}
			
			//setOnBoardLED(true);
				if(edgePassed != 2 && edgePassed != 4 && edgePassed != 0 && edgePassed != 8)
					return;
	
		} else if(shouldForceRight && currentTime >= forceRightEndTimer) {
			//setOnBoardLED(highSpeedMode);
			
			if(edgePassed == 1 || edgePassed == 6 || edgePassed == 7) {
				handleEdgePassing();
				return;	
			} else if(edgePassed == 13) {
				startOperating(false);
			}
			
			setLeftWheelSpeed(30);
			setRightWheelSpeed(30);
			shouldForceRight = false;
		}
		
		if(edgePassed == 8 || edgePassed == 9 ) {
			followLeftDetection();
		}
		
		// process spi data
		int error = 0;
		int detectedTrans = 0;
		
		for(int i = 0; i < 8; i ++) {
			if(lineResult[i] == 0) {
				detectedTrans += 1;
			}	 
		}
		
		// edge or Y cross detection
		if(detectedTrans >= (edgePassed == 5? 6 : 7) ) {
			if( isBlackContinuous() &&(currentTime - edgeDetectorTimer) > (edgePassed == 7? 500 : 240)){
				handleEdgePassing();
				return;
			}
		}
	
		
		lastLineDetectedNums[lastLineDetectedPointer] = detectedTrans;
		lastLineDetectedPointer ++; 
		lastLineDetectedPointer %= 8;

		// calculate error 
		for(int i = 0; i < 8; i ++) {
			if(lineResult[i] == 0) {
				error += lineCalculator[i];
			}	 
		}
		
		if(detectedTrans != 0) {
			error *= multiplierIndex[detectedTrans];
		}
		
		// final speed calculation
		leftSpeed = 43 + (highSpeedMode ? 10 : 5) + error * Kp - accumulateError * Ki / (currentTime - integrateTimer) + (error - previousError) * Kd;
		rightSpeed = 45 + (highSpeedMode ? 10 : 5) - error * Kp + accumulateError * Ki / (currentTime - integrateTimer) - (error - previousError) * Kd;
		
		leftSpeed = leftSpeed < 5 ? 5: leftSpeed > 80? 80: leftSpeed;
		
		rightSpeed = rightSpeed < 5 ? 5: rightSpeed > 80? 80: rightSpeed;
		
		setLeftWheelSpeed(leftSpeed);
		setRightWheelSpeed(rightSpeed);
		
		// record I_error and P_error
		accumulateError += error;
		previousError = error;
		
	} else {
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
	}
}



void readLineArray(void){

	// spi reader 
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2, '0');
		
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	int spi2c = SPI_I2S_ReceiveData(SPI2);
	
	if(spi2c == 0) {
		lineResult[0] = 0;
	}  else {
		lineResult[0] = 1;
	}
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2, '0');
	
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	spi2c = SPI_I2S_ReceiveData(SPI2);
	
	 unsigned opt = 0b000000010;  
	for(unsigned i = 7; i >0 ; i --){
		if(opt & spi2c){
			lineResult[i] = 1;
		} else {
			lineResult[i] = 0;
		}
		opt = opt << 1;
	}
}

