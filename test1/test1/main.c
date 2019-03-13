
/*

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>



int main(void)
{
	DDRD = 0xFF;
	DDRB &= ~(1 << 2);
	PORTD = 0xAA;
	unsigned char state = 1;
	while(1)
	{
		
		if(PINB & (1 << 2)) {
			state = 1-state;
		} 
		if(state == 1) {
			PORTD = 0xAA;
		} else {
		PORTD = 0x55;
		}

		
		_delay_ms(10000);

	}
	return 0;
}
*/

#include <avr/io.h>


void genereateOneSecondDelay(void);
int main(void)
{
	while(1)
	{
		DDRD = 0xFF;
		PORTD = 0x55;
		genereateOneSecondDelay();
		PORTD = 0xAA;
		genereateOneSecondDelay();
		
	}
}

void genereateOneSecondDelay(void){
	//TCNT0 = 0xC0;
	//TCCR0A = 0x00;
	//TCCR0B = 0x05;
	//while ((TIFR0 & (1<<TOV0) )==0);
	//TCCR0B = 0;
	//TIFR0 = (1<<TOV0);

	TCNT1H = 0x00;
	TCNT1L = 0x00;
	OCR1BH = 0x40;
	OCR1BL = 0x00;
	OCR1AH = 0x80;
	OCR1AL = 0x00;
	TCCR1A = 0x00;
	TCCR1B = 0x05;
	while((TIFR1&(1<<OCF1A))==0);
	TIFR1 = 1<<OCF1A;
	TCCR1B = 0;
	TCCR1A = 0;

}
