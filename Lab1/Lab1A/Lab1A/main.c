/*
 * Lab1A.c
 *
 * Created: 2019/2/18 17:22:05
 * Author : ZHANG Runze 14111582d
 */ 

#include <avr/io.h>


int main(void)
{
	
	TCCR0A = (1<< COM0A1) | (0 << COM0A0)|(1<< COM0B1) | (0 << COM0B0)| (1<< WGM01) | (1 << WGM00); // Fast PWM, top at OCR0A, 
	TCCR0B = (1 << WGM02) | (1 << CS02) | (0 << CS01) | (0 << CS00); // prescaler = 256
	OCR0A = 31;  // ocra = 16Mhz / (256 * 500hz) = 128
	OCR0B = 15; // 50 duty cycle
	DDRD = 0b00100000; // output pin PD5
    while (1);
}

