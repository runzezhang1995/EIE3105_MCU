/*
 * Lab1E_AVR_Generate.c
 *
 * Created: 2019/2/19 3:32:41
 * Author : ZRZ-default
 */ 

#include <avr/io.h>
#include <string.h>

#define  F_CPU 16000000
#define  BAUD 9600
#define  UBRR ((F_CPU/(16L * BAUD)) - 1)

#define PRESCALER 256
#define F_PWM 500
#define PWM_CACL ((F_CPU/(F_PWM * PRESCALER)) - 1)

void init();
void USART_Transmit(char data);
unsigned char USART_Receive(void);
void USART_ReceiveNSendBackCapital(void);

int main(void)
{	
	
	unsigned char readyString[] = "Received pulse number: ";
	unsigned char resultString[] = "Width of the pulse is: ";
	unsigned int pulse_width = 0;
	unsigned char buf;
	unsigned char width;
	init();

    /* Replace with your application code */
    while (1) 
    {
		pulse_width = 0;
		while(pulse_width < 10) {
			buf = USART_Receive();
			if(buf >= '0' && buf <= '9') {
				pulse_width = pulse_width * 10 + buf - '0';  
			}
		}

		for(int i=0;i<strlen(readyString);i++)
		{
			USART_Transmit(readyString[i]);
		}

		USART_Transmit(pulse_width / 10 + '0');
		USART_Transmit(pulse_width % 10 + '0');
		USART_Transmit(' ');
			
	 
		OCR0B=pulse_width-1;
		 // code for measure pulse width 
		pulse_width = 0;
    }
}


void init(){
	// usart init
	UCSR0B=(1<<TXEN0)|(1<<RXEN0);// Transmitter Enable and Receiver Enable
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);// 8-bit character
	UBRR0=UBRR;//9600 Baud rate
	
	// pwm init
	TCCR0A=(1<<COM0B1)|(1<<WGM01)|(1<<WGM00);
	// COM0A1, COM0A0, COM0B1 and COM0B0 control the behavior of the OCOA (PD6) and OCOB(PD5) pins
	// These pins can be controlled from the Output Compare Registers (OCR0A/OCR0B)
	// WGM02, WGM01, WGM00 = 1,1,1 -> Fast PWM, TOP is OCRA,
	TCCR0B=(1<<WGM02)|(1<<CS02);
	
	OCR0A=PWM_CACL; //500Hz PWM
	OCR0B=0; //No duty Cycle at the beginning
	DDRD=(1<<5); //PD5 (OC0B) output
}




void USART_Transmit(char data) {
	while( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
}

unsigned char USART_Receive(void){
	while( !( UCSR0A & (1<<RXC0)) );
	return UDR0;
}
