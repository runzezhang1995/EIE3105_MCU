#ifndef BASIC_ENV
#include <stdio.h>
#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#define debug 0
#endif




void setLeftWheelDirection(bool forward);
void setRightWheelDirection(bool forward);
void setLeftWheelSpeed(int percent);
void setRightWheelSpeed(int percent);
void setOnBoardLED(bool isOn);
void pushCharacterToReceiveBuffer(char c);
void clearReceiveBuffer(void);
int findBallIndexForName(char *name);
void handleBufferInformation(void);
bool dataFilter(char c);
struct FieldObject {
	char name[3];
	int xCoor[5];
	int yCoor[5];
	char xPointer;
	char yPointer; 
	int xCoorEst;
	int yCoorEst;


	bool isAvailable;
};
