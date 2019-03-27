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
bool forward = true;


enum State stage = stop;

int pressCount = 0;

int currentTime = 0;
int integrateTimer = 0;

int leftSpeed = 0;
int rightSpeed = 0;

int wheel_count_left = 0;
int wheel_count_right = 0;

int wheel_total_count_left = 0;
int wheel_total_count_right = 0;

struct FieldObject balls[10];
struct Car car;
struct Point targetPoint;
int targetPointIndex = -1;

bool targetValid;
bool highSpeedMode;

struct Point originPoint;

int numOfBalls = 0;

float Kp = 0.60;

float Ki = 0;
float Kd = 0;

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
	setOnBoardLED(true);
	
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
		pidControl();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void USART2_IRQHandler(){
	char data;

	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET){
		data = (char) USART_ReceiveData(USART2);
		
		if(!wifiConnected){
			
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
			
			handleBufferInformation();
		}
	}
}







void startOperating(bool operate){
	if(operate) {
		//USART3Send("on\r\n", sizeof("on\r\n"));
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
		
		//setOnBoardLED(false);
		
		
		
	} else {
		USART3Send("off\r\n", sizeof("off\r\n"));
		setLeftWheelSpeed(0);
		setRightWheelSpeed(0);
		//setOnBoardLED(true);
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
	if(receivePointer == 100) {
		return;
	}
	receiveBuffer[receivePointer] = c;
	receivePointer ++;
}


void handleWifiConnectInfo(){
	
	if(receivePointer > 1 && receiveBuffer[receivePointer - 1] == 'K'){
		//sprintf(buffer, "start handle\r\n ");
		//USART3Send(buffer, sizeof(buffer));
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
			
			car.carF.updated = false;
			car.carB.updated = false;
		
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
		if (strncmp(name, balls[i].name, 3) == 0) {
			return i;
		}
	}
	return -1;
}


void ballToString(int i){
	sprintf(buffer, "\r\nname: %s\r\nx : %d\r\ny : %d\r\ndistance : %f  stop: %s\r\n",balls[i].name, balls[i].points[(balls[i].pointer + 4) % 5].x, balls[i].points[(balls[i].pointer + 4) % 5].y, balls[i].distance , balls[i].isStop ? "yes" : "no" );
	USART3Send(buffer, sizeof(buffer));
}

void carToString(){
	sprintf(buffer, "\r\nname: %s\r\nx : %d\r\ny : %d\r\n",car.carF.name, car.carF.point.x, car.carF.point.y);
	USART3Send(buffer, sizeof(buffer));

	sprintf(buffer, "\r\nname: %s\r\nx : %d\r\ny : %d\r\nangle : %f\r\n",car.carB.name, car.carB.point.x, car.carB.point.y, car.angle);
	USART3Send(buffer, sizeof(buffer));
}

void updateBallPos(int index, int x, int y){
	int i = index;
	balls[i].points[balls[i].pointer].x = x;
	balls[i].points[balls[i].pointer].y = y;
	
	
	
	double distance = calculateDistance(balls[i].points[(balls[i].pointer + 4) % 5],  balls[i].points[balls[i].pointer]);
	//double distance = 0;
	balls[i].isStop = (distance <= 2) ? true : false;

	balls[i].pointer = (balls[i].pointer + 1) % 5;
	
	balls[i].distance = distance;
	balls[i].isAvailable = true;
}

void updateCarPos(char* name, int x, int y){
	if(strncmp(name, "CHD", 3) == 0){
		car.carF.point.x = x;
		car.carF.point.y = y;
		car.carF.updated = true;
	} else {
		car.carB.point.x = x;
		car.carB.point.y = y;
		car.carB.updated = true;
	}
	
	if(car.carF.updated && car.carB.updated){
		car.angle = calculateAngle(car.carB.point, car.carF.point);
	}
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
	originPoint.x = 2700;
	originPoint.y = 286;
}

int calculateError(){
	double CTangle = calculateAngle(car.carB.point, targetPoint);
	double dAngle = CTangle - car.angle;

	dAngle = dAngle > 180 ? (dAngle - 360) : ((dAngle < -180) ? (dAngle + 360) : dAngle);
	dAngle = round(dAngle);
	return (int)dAngle;
}

void updateTarget(){

	char tName[3] = {'\0'};
	
	switch(stage) {
		
		case back:
			highSpeedMode = false;
			strncpy(tName, "IDE", 3);
			break;
		case bh:
			strncpy(tName, "ORI", 3);
			highSpeedMode = true;
			break;
		case stop:
		case wait:
			strncpy(tName, "BOE", 3);
			highSpeedMode = false;
			break;
		default:
			break;
	}
	if(strncmp(tName, "ORI", 3) == 0) {
		copyPointFromPoint(&targetPoint, &originPoint);
		//targetPointIndex = -1;
		stage ++;
		targetValid = true;
	} else if(strncmp(tName, "IDE", 3) == 0){
		//targetPointIndex = -1;
		stage ++;
		targetValid = true;
	} else {
		int index = findBallIndexForName(tName);
		if(index == -1) {
			targetValid = false;
			return;
		} else {
			targetPointIndex = index;
			copyPointFromPoint(&targetPoint, &(balls[index].points[balls[index].pointer]));
			targetValid = true;
			stage = bh;
		}
	}
	//setOnBoardLED(stage % 2);
	integrateTimer = currentTime;
	accumulateError = 0;
	
}


void pidControl(){
		if(wifiConnected){
			setOnBoardLED(stage % 2);
		}
`	
		/*sprintf(buffer, "\r\n total number of item: %d\r\n",numOfBalls);
			USART3Send(buffer, sizeof(buffer));	
			
			for(int i = 0; i < numOfBalls; i++) {
				ballToString(i);
				//sprintf(buffer, "\r\ndistance: %f\r\n", balls[i].name,balls[i].xCoor[0], balls[i].yCoor[0]);
				//USART3Send(buffer, sizeof(buffer));
			}			
			carToString();
	*/
	if(operating) {		
		if(!targetValid) {
			setLeftWheelSpeed(0);
			setRightWheelSpeed(0);
			updateTarget();
			return;
		}
		
		if(stage == bh) {
			copyPointFromPoint(&targetPoint, &(balls[targetPointIndex].points[balls[targetPointIndex].pointer]));			
		} else if(stage == wait){
			setLeftWheelSpeed(0);
			setRightWheelSpeed(0);
			if(balls[targetPointIndex].points[balls[targetPointIndex].pointer].x >= 600 && balls[targetPointIndex].isStop){
				targetValid = false;
			}

			return;
		}

		int distance = calculateDistance(car.carF.point, targetPoint);
		int error = 0;
		error = calculateError();
		
		if ((distance < 30)||(stage % 2 == 0 && car.carB.point.x > 900)||(stage == 1 && car.carF.point.x < 630)) {
			targetValid = false;
			setLeftWheelSpeed(0);
			setRightWheelSpeed(0);
			return;
		}
		forward = abs(error) >= 90 ? false : true;
		error = error > 90 ? (180 - error) : (error < -90 ? (-180 - error) : error);

		//sprintf(buffer, "\r\nerror : %d\r\ndirection : %s\r\nstage: %d\r\nrarget x: %d\r\ntarget y: %d\r\n",  error, forward ? "forward" : "back", stage, targetPoint.x, targetPoint.y );
		//USART3Send(buffer, sizeof(buffer));

		// final speed calculation
		leftSpeed =  42 + (highSpeedMode ? 10 : 4 ) + error * Kp + accumulateError * Ki / (currentTime - integrateTimer) + (error - previousError) * Kd;
		rightSpeed = 44 + (highSpeedMode ? 10 : 4 ) - error * Kp - accumulateError * Ki / (currentTime - integrateTimer) - (error - previousError) * Kd;
		
		leftSpeed = leftSpeed < 5 ? 5: leftSpeed > 60? 60: leftSpeed;
		leftSpeed *= forward ? 1 : -1;
		rightSpeed = rightSpeed < 5 ? 5: rightSpeed > 60? 60: rightSpeed;
		rightSpeed *= forward ? 1 : -1;
	
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

bool copyPointFromPoint(struct Point* destP, struct Point* sourceP){
	destP -> x = sourceP -> x;
	destP -> y = sourceP -> y; 
	return true;
}
