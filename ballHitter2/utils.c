#include "utils.h"

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




void setOnBoardLED(bool isOn){
	GPIO_WriteBit(GPIOB, GPIO_Pin_7, isOn? Bit_SET:Bit_RESET); // On-board LED is turn off if not same speed
}

int getValueForHexChar(char c){
	if(c >= '0' && c <= '9'){
		return (c -'0');
	}
	if(c >= 'A' && c <= 'F') {
		return (c - 'A' + 10);
	}

	if(c >= 'a' && c <= 'f') {
		return (c - 'a' + 10);
	}
	
	return -1;
}


bool checkValidObjectName(char* name){
	if(sizeof(name) < 3) return false;
	for (int i = 0; i < 3; i++){
		if(name[i] <= 'A' || name[i] >= 'Z') return false;
	}
	return true;
}


