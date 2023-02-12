#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


void Initialize() {
    cli();

    //setting input capture pin to be input
    DDRB &= ~(1<<DDB0);

    //setting onboard led at PB5 to be output
    DDRB |= (1<<DDB5);

    //setting up timer1
    TCCR1B &= ~(1<<CS10);
    TCCR1B |= (1<<CS11);
    TCCR1B &= ~(1<<CS12);

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
    sei();
}

int main() {

    Initialize();
    //led should be turned off at the start
    PORTB &= ~(1 << PORTB5);
    while(1) {
    }

}
ISR(TIMER1_CAPT_vect) {
    if (PINB & (1 <<PINB0)) {
        //turn led on if button is pressed
        PORTB |= (1 << PORTB5);

        cli();
        //look for falling edge to turn LED off
        TCCR1B &= ~(1<<ICES1);
        sei();
    }
    else {
        //turn led off if button is released
        PORTB &= ~(1 << PORTB5);

        cli();
        //look for rising edge to turn LED on
        TCCR1B |= (1<<ICES1);
        sei();
    }
}
