#include "utils.h"

void ballToString(int i);

char wifiConnect1[] = "AT+CWJAP=\"IntegratedProject\",\"31053106\"\r\n";
char wifiConnect2[] = "AT+CIPSTART=\"UDP\",\"0\",0,3105,2\r\n";
char receiveBuffer [100] = {""};
int receivePointer = 0;

int currentHandleItemNumber = 0;
int totalItemNumber = 0;


bool wifiConnected = false;
bool shouldStartGetCountNum= false;
bool shouldStartGetItem = false;


int pressCount = 0;

int currentTime = 0;

int leftSpeed = 0;
int rightSpeed = 0;

int wheel_count_left = 0;
int wheel_count_right = 0;

int wheel_total_count_left = 0;
int wheel_total_count_right = 0;

struct FieldObject balls[10];
struct Car car;
struct Point targetPoint;
struct Point originPoint;

int numOfBalls = 0;

float Kp = 2.80;

float Ki = .6;
float Kd = 1.1;

int accumulateError = 0;
int previousError = 0;


bool operating = false;
char buffer[100] = {'\0'};



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
	initCarObject();
	
}

int main(void) {
		
	init();
	setLeftWheelDirection(true);
	setRightWheelDirection(true);
	
	startOperating(false);
	
	USART2Send(wifiConnect1, sizeof(wifiConnect1));
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
		
		if(!wifiConnected){
			USART_SendData(USART3,data);
			USART_ClearITPendingBit(USART3, USART_IT_RXNE);
			if(dataFilter(data)) pushCharacterToReceiveBuffer(data);
			
			if(data == 'K') {
			sprintf(buffer, "start handle\r\n ");
			USART3Send(buffer, sizeof(buffer));
			USART2Send(wifiConnect2, sizeof(wifiConnect2));
			clearReceiveBuffer();
			wifiConnected = true;
			}	
			return;
		}
		
		if(dataFilter(data))  {			
			if(shouldStartGetItem && receivePointer == 0 && (data <= 'A' || data >= 'Z')){
			return;
		}
				pushCharacterToReceiveBuffer(data);
		}
		
		if(data == ',') {
				shouldStartGetCountNum = true;
				setOnBoardLED(true);
				clearReceiveBuffer();
		}
		if(shouldStartGetCountNum && receivePointer == 2) {
			handleBufferInformation();
		}

		if(data == ':') {
				shouldStartGetItem = true;
				clearReceiveBuffer();
		}
		
		if(shouldStartGetItem && receivePointer == 9) {
			//sprintf(buffer, "go to handle name\r\n");
			//USART3Send(buffer, sizeof(buffer));
			
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


void handleWifiConnectInfo(){
	if(receivePointer > 1 && receiveBuffer[receivePointer - 1] == 'K'){
		sprintf(buffer, "start handle\r\n ");
		USART3Send(buffer, sizeof(buffer));
		USART2Send(wifiConnect2, sizeof(wifiConnect2));
		clearReceiveBuffer();
		wifiConnected = true;
	}
	
}

void handleBufferInformation(){
	if(shouldStartGetCountNum) {
		totalItemNumber = (receiveBuffer[0] - '0')*10 + (receiveBuffer[1] - '0');
		totalItemNumber = (totalItemNumber - 5) / 11;
		clearReceiveBuffer();
		shouldStartGetCountNum = false;
		return;
	}
	
	if(shouldStartGetItem) {
		char itemName[3];
		strncpy(itemName, receiveBuffer, 3);
		if(strncmp(itemName, "CMD", 3) != 0 && strncmp(itemName, "CHD", 3) != 0 && strncmp(itemName, "CTL", 3) != 0 && checkValidObjectName(itemName)) {				
			int xPos = getValueForHexChar(receiveBuffer[3]) * 256 + getValueForHexChar(receiveBuffer[4]) * 16 + getValueForHexChar(receiveBuffer[5]);
			int yPos = getValueForHexChar(receiveBuffer[6]) * 256 + getValueForHexChar(receiveBuffer[7]) * 16 + getValueForHexChar(receiveBuffer[8]);
			
			bool ballExists = false;

			for(int i = 0; i < numOfBalls; i++ ){
				if(strncmp(balls[i].name, itemName, 3) == 0) {
					updateBallPos(i, xPos, yPos);
					ballExists = true;
					break;
				} 
			}
			if(!ballExists){
				strcpy(balls[numOfBalls].name, itemName);
				balls[numOfBalls].points[0].x = xPos;
				balls[numOfBalls].points[0].y = yPos;
			
				balls[numOfBalls].pointer = 1;
				balls[numOfBalls].distance = 0;
				balls[numOfBalls].isAvailable = true;
				balls[numOfBalls].isStop = false;
				numOfBalls ++;
			}
		}

		if(strncmp(itemName, "CHD", 3) == 0 || strncmp(itemName, "CTL", 3) == 0) {
			int xPos = getValueForHexChar(receiveBuffer[3]) * 256 + getValueForHexChar(receiveBuffer[4]) * 16 + getValueForHexChar(receiveBuffer[5]);
			int yPos = getValueForHexChar(receiveBuffer[6]) * 256 + getValueForHexChar(receiveBuffer[7]) * 16 + getValueForHexChar(receiveBuffer[8]);
			updateCarPos(itemName, xPos, yPos);
		}
		
		currentHandleItemNumber ++;
		if(currentHandleItemNumber == totalItemNumber) {
			
			sprintf(buffer, "\r\n total number of item: %d\r\n",numOfBalls);
			USART3Send(buffer, sizeof(buffer));
			
	
			for(int i = 0; i < numOfBalls; i++) {
				ballToString(i);
				//sprintf(buffer, "\r\ndistance: %f\r\n", balls[i].name,balls[i].xCoor[0], balls[i].yCoor[0]);
				//USART3Send(buffer, sizeof(buffer));
			}			
			setOnBoardLED(false);
			shouldStartGetItem = false;
			totalItemNumber = 0;
			currentHandleItemNumber = 0;
			//numOfBalls = 0;
		}
		clearReceiveBuffer();
		return;
	} 
}

int findBallIndexForName(char *name){
	for (int i = 0; i < numOfBalls; i++) {
		if (strcmp(name, balls[i].name) == 0) {
			return i;
		}
	}
	return -1;
}


void ballToString(int i){
	sprintf(buffer, "\r\nname: %s\r\nx : %d\r\ny : %d\r\ndistance : %f  stop: %s\r\n",balls[i].name, balls[i].points[(balls[i].pointer + 4) % 5].x, balls[i].points[(balls[i].pointer + 4) % 5].y, balls[i].distance , balls[i].isStop ? "yes" : "no" );
	USART3Send(buffer, sizeof(buffer));
}


void updateBallPos(int index, int x, int y){
	int i = index;
	balls[i].points[balls[i].pointer].x = x;
	balls[i].points[balls[i].pointer].y = y;
	
	
	
	double distance = calculateDistance(balls[i].points[(balls[i].pointer + 4) % 5],  balls[i].points[balls[i].pointer]);
	//double distance = 0;
	balls[i].isStop = (distance <= 10) ? true : false;

	balls[i].pointer = (balls[i].pointer + 1) % 5;
	
	balls[i].distance = distance;
	balls[i].isAvailable = true;
}

void updateCarPos(char* name, int x, int y){
	if(strncmp(name, "CHD", 3) == 0){
		car.carF.point.x = x;
		car.carF.point.y = y;
	} else {
		car.carB.point.x = x;
		car.carB.point.y = y;
	}
	
	car.angle = calculateAngle(car.carB.point, car.carF.point);
}

double calculateAngle(struct Point origin, struct Point p){
	int dx = p.x - origin.x;
	int dy = p.y - origin.y;
	return atan2((double)dy, (double)dx) * 57.296;
}


double calculateDistance(struct Point p1, struct Point p2){
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

void initCarObject(){
	strncpy(car.carF.name, "CHD", 3);
	strncpy(car.carB.name, "CTL", 3);
	originPoint.x = 82;
	originPoint.y = 286;
}
