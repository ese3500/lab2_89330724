#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

/*--------------------Libraries---------------------------*/
#include <stdio.h>
#include "uart.h"

/*--------------------Variables---------------------------*/
char String[25];
int Morse[5];
int len = 0;
long edge1 = 0;
long edge2 = 0;
long dif = 0;
uint16_t timePressed = 0;
uint16_t timeReleased = 0;
int pressed = 0;
int released = 0;
int overflow = 0;
int over = 0;
int expecting = 1;
/*-----------------------------------------------------------*/


void Initialize() {
    cli();

    //setting input capture pin to be input
    DDRB &= ~(1<<DDB0);

    //setting onboard led to output
    DDRB |= (1<<DDB5);

    //setting led at PB1 and PB2 to output for displaying dots and dashes
    DDRB |= (1<<DDB1);
    DDRB |= (1<<DDB2);

    //setting up timer1 to 62.5 kHz freq by prescaling by 256
    TCCR1B &= ~(1<<CS10);
    TCCR1B &= ~(1<<CS11);
    TCCR1B |= (1<<CS12);

    //setting timer1 normal
    TCCR1A &= ~(1<<WGM10);
    TCCR1A &= ~(1<<WGM11);
    TCCR1B &= ~(1<<WGM12);
    TCCR1B &= ~(1<<WGM13);


    //look for rising edge
    TCCR1B |= (1<<ICES1);
    //enable noise canceler
    TCCR1B |= (1<<ICNC1);

    //clear input capture flag
    TIFR1 = (1<<ICF1);
    //Enable input capture interrupt
    TIMSK1 |= (1<<ICIE1);

    //clear output compare flag
    TIFR1 = (1<<TOV1);
    //Enable overflow interrupt
    TIMSK1 |= (1<<TOIE1);
    sei();
}
void morseToAscii(const int morse[], int length);

int main() {

    UART_init(BAUD_PRESCALER);
    Initialize();
    sprintf(String,"Hello world! \n");
    UART_putstring(String);

    PORTB &= ~(1<<PORTB5);
    PORTB &= ~(1<<PORTB1);
    PORTB &= ~(1<<PORTB2);

    while(1) {
        if (released && timePressed > 30) {
            if (timePressed < 400) {
                //led at PB1 to turn on when it is a dot
                PORTB |= (1<<PORTB1);
                _delay_ms(50);
                PORTB &= ~(1<<PORTB1);

                sprintf(String,"DOT ");
                UART_putstring(String);
            } else {
                if (timePressed < 1200) {
                    //led at PB2 to turn on when it is a dot
                    PORTB |= (1<<PORTB2);
                    _delay_ms(50);
                    PORTB &= ~(1<<PORTB2);

                    sprintf(String,"DASH ");
                    UART_putstring(String);
                }
                else {
                    sprintf(String,"LONG PRESS ");
                    UART_putstring(String);
                }
            }
            released = 0;
        }
        if (pressed && timeReleased > 1200) {
            sprintf(String,"] : ");
            UART_putstring(String);
            morseToAscii(Morse, len);
            pressed  = 0;
        }

    }

}

ISR(TIMER1_CAPT_vect) {
    if (PINB & (1 <<PINB0)) {
        edge1 = ICR1;
        dif = (((edge1 / 125) * 2) + (overflow * over)) - ((edge2/ 125) * 2);
        timeReleased = dif;

        PORTB |= (1<<PORTB5);
        overflow = 0;
        expecting = 2;
        pressed = 1;
        cli();
        //look for falling edge
        TCCR1B &= ~(1<<ICES1);
        sei();
    }
    else {
        edge2 = ICR1;
        dif = (((edge2 / 125) * 2) + (overflow * over)) - ((edge1/ 125) * 2);
        timePressed = dif;

        if (timePressed < 400) {
            Morse[len] = 0;
            len++;
        } else {
            if (timePressed < 1200) {
                Morse[len] = 1;
                len++;
            }
        }

        PORTB &= ~(1<<PORTB5);
        overflow = 0;
        released = 1;
        expecting = 1;
        cli();
        //look for rising edge
        TCCR1B |= (1<<ICES1);
        sei();
    }
}

ISR(TIMER1_OVF_vect) {
    if (overflow < 50) {
        overflow++;
    }
    if (overflow == 1) {
        if (expecting == 1) {
            over = (int)(((65536 - edge2) / 125) * 2);
            edge2 = 0;
        } else {
            over = (int)(((65536 - edge1) / 125) * 2);
            edge1 = 0;
        }
        if (edge1 == 0 && edge2 == 0) {
            over = 1050;
        }
    } else {
        over = 1050;
    }
}

//0 to represent dot and 1 to represent dash
void morseToAscii(const int morse[], int length) {
    if (length == 1) {
        if (morse[0]) {
            sprintf(String,"T");
            UART_putstring(String);
        } else {
            sprintf(String,"E");
            UART_putstring(String);
        }
    }

    if (length == 2) {
        if (morse[0] && morse[1]) {
            sprintf(String,"M");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1]) {
            sprintf(String,"I");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1]) {
            sprintf(String,"A");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1]) {
            sprintf(String,"N");
            UART_putstring(String);
        }
    }

    if (length == 3) {
        if (morse[0] && !morse[1] && !morse[2]) {
            sprintf(String,"D");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && !morse[2]) {
            sprintf(String,"G");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1] && morse[2]) {
            sprintf(String,"K");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && morse[2]) {
            sprintf(String,"O");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1] && !morse[2]) {
            sprintf(String,"R");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2]) {
            sprintf(String,"S");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && morse[2]) {
            sprintf(String,"U");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1] && morse[2]) {
            sprintf(String,"W");
            UART_putstring(String);
        }
    }

    if (length == 4) {
        if (morse[0] && !morse[1] && !morse[2] && !morse[3]) {
            sprintf(String,"B");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1] && morse[2] && !morse[3]) {
            sprintf(String,"C");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && morse[2] && !morse[3]) {
            sprintf(String,"F");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2] && !morse[3]) {
            sprintf(String,"H");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1] && morse[2] && morse[3]) {
            sprintf(String,"J");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1] && !morse[2] && !morse[3]) {
            sprintf(String,"L");
            UART_putstring(String);
        }
        if (!morse[0] && morse[1] && morse[2] && !morse[3]) {
            sprintf(String,"P");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && !morse[2] && morse[3]) {
            sprintf(String,"Q");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2] && morse[3]) {
            sprintf(String,"V");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1] && !morse[2] && morse[3]) {
            sprintf(String,"X");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1] && morse[2] && morse[3]) {
            sprintf(String,"Y");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && !morse[2] && !morse[3]) {
            sprintf(String,"Z");
            UART_putstring(String);
        }
    }

    if (length == 5) {
        if (!morse[0] && morse[1] && morse[2] && morse[3] && morse[4]) {
            sprintf(String,"1");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && morse[2] && morse[3] && morse[4]) {
            sprintf(String,"2");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2] && morse[3] && morse[4]) {
            sprintf(String,"3");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2] && !morse[3] && morse[4]) {
            sprintf(String,"4");
            UART_putstring(String);
        }
        if (!morse[0] && !morse[1] && !morse[2] && !morse[3] && !morse[4]) {
            sprintf(String,"5");
            UART_putstring(String);
        }
        if (morse[0] && !morse[1] && !morse[2] && !morse[3] && !morse[4]) {
            sprintf(String,"6");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && !morse[2] && !morse[3] && !morse[4]) {
            sprintf(String,"7");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && morse[2] && !morse[3] && !morse[4]) {
            sprintf(String,"8");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && morse[2] && morse[3] && !morse[4]) {
            sprintf(String,"9");
            UART_putstring(String);
        }
        if (morse[0] && morse[1] && morse[2] && morse[3] && morse[4]) {
            sprintf(String,"0");
            UART_putstring(String);
        }
    }

    sprintf(String,"\n\n[");
    UART_putstring(String);
    len = 0;
}

