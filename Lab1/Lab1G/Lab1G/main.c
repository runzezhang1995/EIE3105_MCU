/*
 * Lab1G.c
 *
 * Created: 2019/2/19 4:09:02
 * Author : ZRZ-default
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>


#define  F_CPU 16000000
#define PRESCALER 256
#define F_PWM 500
#define PWM_CACL ((F_CPU/(F_PWM * PRESCALER)) - 1)

#define reff 0x02  // Higher, the bigger chance to be dark
unsigned char HighBYTE,LowBYTE;
int ADCnum=0;

void init();

int main(void)
{
	/* Replace with your application code */
	init();
	while (1)
	{
	}
}

void init(){
	TCCR0A = (1<<COM0A1)|(1<<COM0B1)|(1<<WGM01)|(1<<WGM00); //Timer 0 Fast PWM Mode
	TCCR0B = (1<<WGM02)|(1<<CS02); //Timer 0 Fast PWM, Prescaler 256
	OCR0A = PWM_CACL;

	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11)|(1<<WGM10);
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<CS12);
	OCR1A = PWM_CACL;

	TCCR2A = (1<<COM2A1)|(1<<COM2B1)|(1<<WGM21)|(1<<WGM20);
	TCCR2B = (1<<WGM22)|(1<<CS22)|(1<<CS21);
	OCR2A = PWM_CACL;

	DDRB |= (1<<2); // PortB.2 output for PWM
	DDRD |= (1<<5)|(1<<3); // PortD.3, 5 output for PWM
	DDRC = (~(1<<0))&(~(1<<1))&(~(1<<2)); // PortC.0, 1, 2 as input for ADC

	ADCSRA= (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Make ADC enable and interrupt select ck/128. The higher the better
	ADMUX = (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);//Internal 1.1Vref. and ADC0 single Ended Input, input LEFT-justified data
	sei();
	ADCSRA |= (1<<ADSC);//start conversion
}

ISR(ADC_vect){
	// The brighter, the resistor will decrease, the voltage of resistor will decrease the voltage of ADC will increase, the value of ADC will be bigger
	LowBYTE = ADCL;
	HighBYTE = ADCH;
	if (ADCnum==0){
		if (HighBYTE < reff){
			OCR0B = 0;
		}
		else{
			OCR0B = PWM_CACL;
		}
		ADMUX |= (1<<MUX0); //Next Step ADC1 Single Ended Input
		ADCnum = 1;
	}
	else if (ADCnum==1){
		if (HighBYTE < reff){
			OCR1B = 0;
		}
		else{
			OCR1B = PWM_CACL;
		}
		ADMUX &= ~(1<<MUX0);
		ADMUX |= (1<<MUX1); //Next Step ADC2 Single Ended Input
		ADCnum=2;
	}
	else if(ADCnum==2){
		if (HighBYTE < reff){
			OCR2B = 0;
		}
		else{
			OCR2B=PWM_CACL;
		}
		ADMUX &= ~(1<<MUX1);
		//Next Step ADC0 Single Ended Input
		ADCnum=0;
	}
	ADCSRA |= (1<<ADSC); //start conversion
}



