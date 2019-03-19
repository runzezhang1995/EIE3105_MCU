#ifndef BASIC_ENV
#include <stdio.h>
#include "stm32f10x.h"                  // Device header
#include "PinMap.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#endif

enum State {stop = 0, bh1 = 1, back1, bh2, back2, bh3, end};


struct Point{
	int x;
	int y;
};


struct FieldObject {
	char name[3];
	struct Point points[5];
	char pointer;
	struct Point estPoint;
	
	double distance;

	bool isAvailable;
	bool isStop;
};

struct SimpleFieldObject {
	char name[3];
	struct Point point;
	bool updated;
};


struct Car {
	struct SimpleFieldObject carF;
	struct SimpleFieldObject carB;
	double angle;
};

void setLeftWheelDirection(bool forward);
void setRightWheelDirection(bool forward);
void setLeftWheelSpeed(int percent);
void setRightWheelSpeed(int percent);
void setOnBoardLED(bool isOn);
void pushCharacterToReceiveBuffer(char c);
void clearReceiveBuffer(void);
int findBallIndexForName(char *name);
void handleWifiConnectInfo(void);
void handleBufferInformation(void);
bool dataFilter(char c);
int getValueForHexChar(char c);
void initCarObject(void);
bool checkValidObjectName(char* name);
void pidControl(void);
double calculateDistance(struct Point p1, struct Point p2);
int calculateError(void);
bool copyPointFromPoint(struct Point* destP, struct Point* sourceP);
void carToString(void);

double calculateAngle(struct Point origin, struct Point p);
void updateBallPos(int index, int x, int y);
void updateCarPos(char* name, int x, int y);
