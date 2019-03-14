#include "utils.h"


char wifiConnect1[] = "AT+CWJAP=\"IntegratedProject\",\"31053106\"\r\n";
char wifiConnect2[] = "AT+CIPSTART=\"UDP\",\"0\",0,3105,2\r\n";
char receiveBuffer [50] = {""};
int receivePointer = 0;
int currentHandleItemNumber = 0;
int totalItemNumber = 0;

int pressCount = 0;

int currentTime = 0;

int leftSpeed = 0;
int rightSpeed = 0;

int wheel_count_left = 0;
int wheel_count_right = 0;

int wheel_total_count_left = 0;
int wheel_total_count_right = 0;

struct FieldObject balls[10];
int numOfBalls = 0;

float Kp = 2.80;

float Ki = .6;
float Kd = 1.1;

int accumulateError = 0;
int previousError = 0;


bool operating = false;
char buffer[50] = {'\0'};



void startOperating(bool operate);


void init(){
	TIM3_PWM_init();
	EXTI_wheel_counter_init();
	
	USART2_init();
	USART3_init();
	
	Wheel_Dir_Init();
	User_Button_Init();
	Board_LED_Init();
	TIM2_Init();
	//SPI2_Init();
	
}

int main(void) {
		
	init();
	setLeftWheelDirection(true);
	setRightWheelDirection(true);
	
	startOperating(false);
	
	while(1) {
			
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
		//startOperating(!operating);
		switch(pressCount) {

			case 0:
				setOnBoardLED(true);
				USART2Send(wifiConnect1, sizeof(wifiConnect1));
			break;
			case 1:
				setOnBoardLED(false);
				USART2Send(wifiConnect2, sizeof(wifiConnect2));
			break;
			default:
				setOnBoardLED(false);
				break;
		}
		pressCount ++;
		
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
			sprintf(buffer, "%dRR\r\n ", currentTime);
			USART3Send(buffer, sizeof(buffer));
		}
		
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void USART2_IRQHandler(){
	char data;
	//setOnBoardLED(true);

	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET){
		data = (char) USART_ReceiveData(USART2);
		

		if(dataFilter(data))  {
				pushCharacterToReceiveBuffer(data);
		} 
		if(receivePointer == 50) {
			//pushCharacterToReceiveBuffer('\r');
			//pushCharacterToReceiveBuffer('\n');
			handleBufferInformation();
		}

		//USART_SendData(USART3,data);
		//USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}







void startOperating(bool operate){
	if(operate) {
		USART3Send("on\r\n", sizeof("on\r\n"));
		currentTime = 0;
		leftSpeed = 40;
		rightSpeed = 45;

		wheel_count_left = 0;
		wheel_count_right = 0;

		wheel_total_count_left = 0;
		wheel_total_count_right = 0;
		
		accumulateError = 0;
		previousError = 0;
		
	

		
		setLeftWheelSpeed(leftSpeed);
		setRightWheelSpeed(rightSpeed);
		
		setOnBoardLED(false);
		
		
		
	} else {
		USART3Send("off\r\n", sizeof("off\r\n"));
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
		setOnBoardLED(true);
	}
	operating = operate;
	
}

bool dataFilter(char c){
	if(c >= '0' && c <='9') return true;
	if(c >= 'a' && c <='z') return true;
	if(c >= 'A' && c <='Z') return true;
	if(c == '+' || c == ',' || c== ':') return true;
	return false;
}

void clearReceiveBuffer(){
	char c[] = {""};
	strcpy(receiveBuffer, c);
	receivePointer = 0;	
}

void pushCharacterToReceiveBuffer(char c){
	if(receivePointer == 50) {
		setOnBoardLED(true);
		return;
	}
	receiveBuffer[receivePointer] = c;
	receivePointer ++;
}

void handleBufferInformation(){
	USART3Send(receiveBuffer, sizeof(receiveBuffer));
	sprintf(buffer, "number of bits: %d\r\n", receivePointer);
	USART3Send(buffer, sizeof(buffer));
	
	/*
	int starter = 0;
	if(currentHandleItemNumber == 0) {
		totalItemNumber = ((receiveBuffer[5] - '0') * 10 + (receiveBuffer[6] - '0') - 5) / 11; 
		starter = 8;
	}
	char currentName[] = {""};
	strncpy(currentName, (receiveBuffer + starter), 3);
	*/
	clearReceiveBuffer();
}

int findBallIndexForName(char *name){
	for (int i = 0; i < numOfBalls; i++) {
		if (strcmp(name, balls[i].name) == 0) {
			return i;
		}
	}
	return -1;
}


