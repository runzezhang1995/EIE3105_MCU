/*
 * Lab1F_AVR_Capture.c
 *
 * Created: 2019/2/19 3:44:36
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
	

	unsigned char resultString[] = "Width of the pulse is: ";

	unsigned char width;
	init();

    /* Replace with your application code */
    while (1) 
    {
		DDRB &= ~(1<<0); //PB0 (ICP1) Input
		PORTB=1;//Pull-up;
		TCCR1A=0; //Timer Mode = normal
		TCCR1B=(1<<ICES1)|(1<<CS12)| (0<<CS11) | (0<<CS10); //Rising edge, prescaler=256, no noise canceller
		TIFR1=(1<<ICF1);//Clear input capture flag 
		while((TIFR1&(1<<ICF1))==0); //wait until ICF1 is set 
		width=ICR1L;
		
		/* ICF1 
		This flag is set when a capture event occurs on the ICP1 pin. When the Input Capture Register
		(ICR1) is set by the WGM13:0 to be used as the TOP value, the ICF1 Flag is set when the counter
		reaches the TOP value.
		*/
		TIFR1=(1<<ICF1);//Clear Flag
		TCCR1B=(0<<ICES1) | (1<<CS12) | (0<<CS11) | (0<<CS10) ;//Falling edge, prescaler 256
		while((TIFR1&(1<<ICF1))==0); //Wait until capture
		width	=ICR1L-width; //Capture and pulse width = falling - rising
		TIFR1=(1<<ICF1); //Clear ICF1 

		width *= 0.79;

		for(int i=0; i<strlen(resultString);i++){
			USART_Transmit(resultString[i]);
		}
		
		USART_Transmit(width / 10 + '0');
		USART_Transmit(width % 10 +'0');
		USART_Transmit('.');
		USART_Transmit(' ');
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
