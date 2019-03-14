/*
 * usartReader.c
 *
 * Created: 2019/2/26 15:05:58
 * Author : 14111
 */ 

#include <avr/io.h>
#define FOSC 1843200
#define BAUD 115200
#define MYUBRR FOSC/16/BAUD-1

void usart_init(void){
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01)| (1 << UCSZ00);
	UBRR0L = 0x67;

}

void usart_send(unsigned char ch) {
	while(!(UCSR0A & (1 <<UDRE0)));
	UDR0 = ch;
}

int main(void)
{
	usart_init();
	unsigned char ch;
    /* Replace with your application code */
    while (1) 
    {
		while(!(UCSR0A & (1 << RXC0)));
		ch = UDR0;
		usart_send(ch);
    }
}

