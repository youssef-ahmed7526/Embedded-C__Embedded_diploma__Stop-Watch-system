#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char seconds = 0,minutes = 0,hours = 0;


/* External INT0 Interrupt Service Routine to reset the stop watch */
ISR(INT0_vect)
{
	seconds = 0; minutes = 0; hours = 0;		//reset time of the stop watch
	TCNT1 = 0;									//reset timer counts
}

/* External INT1 Interrupt Service Routine to pause the stop watch */
ISR(INT1_vect)
{
	TCCR1B &= 0xF8;								//set clock bits CS12 CS11 CS10 to zero
}

/* External INT2 Interrupt Service Routine to resume the stop watch */
ISR(INT2_vect)
{
	TCCR1B |= (1<<CS11)| (1<<CS10);				//set the clock again of the timer with pre-scaler=64
}

/*Interrupt Service Routine of executed after each compare match */
ISR(TIMER1_COMPA_vect)
{
	seconds++;
	if (seconds==60)
	{
		seconds=0;
		minutes++;
		if (minutes==60)
		{
			minutes=0;
			hours++;
		}
	}
}

/* Enable and define triggering of INT0 */
void INT0_Init(void)
{
	GICR  |= (1<<INT0);						// Enable external interrupt pin INT0
	MCUCR |= (1<<ISC01);					// Trigger INT0 with the falling edge
}

/* Enable and define triggering of INT1 */
void INT1_Init(void)
{
	GICR  |= (1<<INT1);						// Enable external interrupt pin INT0
	MCUCR |= (1<<ISC00) | (1<<ISC01);		// Trigger INT1 with the rising edge
}

/* Enable and define triggering of INT2 */
void INT2_Init(void)
{
	GICR  |= (1<<INT2);						// Enable external interrupt pin INT0
	MCUCSR &= ~(1<<ISC2);					// Trigger INT2 with the falling edge
}

/* Setting compare mode of Timer1 */
void Timer1_CTC_Init(void)
{
	TCNT1 = 0;								//setting initial value	of Timer1 register to zero
	/*use Timer1 in compare mode and set the pre-scaler to 64 */
	TCCR1B = (1<<WGM12) | (1<<CS11)| (1<<CS10);
	OCR1A = 15625;							//set the value to compare with
	TIMSK |= (1<<OCIE1A);					//enable compare interrupt for Timer1
	TCCR1A = (1<<FOC1A);					//non PWM mode
}

/* Function the first digit of seconds, minutes and hours */
void display_first_digit (unsigned char time,unsigned char i)
{
	PORTC = ((time % 10) | (PORTC & 0xF0));	//set the first 4 bits of PORTC to the first digit
	PORTA = ( (1<<i) | (PORTA & ~(0x3F)));	//enable the 7_segment unit number i
	_delay_us(1);
	PORTA = PORTA & ~(0x3F);				//disable all the 7_segment units
}

/* Function the second digit of seconds, minutes and hours */
void display_second_digit (unsigned char time,unsigned char i)
{
	PORTC = ((time / 10) | (PORTC & 0xF0));	//set the first 4 bits of PORTC to the second digit
	PORTA = ( (1<<i) | (PORTA & ~(0x3F)));	//enable the 7_segment unit number i
	_delay_us(1);
	PORTA = PORTA & ~(0x3F);				//disable all the 7_segment units
}


int main(void)
{
	DDRD  &= (~(1<<PD2));					// Configure INT0/PD2 as input pin
	PORTD|= (1<<PD2);						//enable pull up of PD2

	DDRD  &= (~(1<<PD3));					// Configure INT1/PB2 as input pin

	DDRB  &= (~(1<<PB2));					// Configure INT0/PD2 as input pin
	PORTB|= (1<<PB2);						//enable pull up of PB2

	DDRA |= 0x3F;							//set enable pins (first 6 pins of PORTA) as output pins
	DDRC |= 0x0F;							//set first 4 pins of PORTC as output pins
	PORTA|= 0x3F;							//enable all the 7_segments units
	PORTC= PORTC & 0xF0;					//initialize the 7_segments units with the value of zero
	SREG |= (1<<7);							//enable global interrupt flag in the status register

	INT0_Init();							//call of INT0 function
	INT1_Init();							//call of INT1 function
	INT2_Init();							//call of INT2 function
	Timer1_CTC_Init();						//call of Timer1 function

/* Loop to be displaying all the 7_segments units at the same time */
	while(1)
	{
		display_first_digit(seconds,0);
		display_second_digit(seconds,1);
		display_first_digit(minutes,2);
		display_second_digit(minutes,3);
		display_first_digit(hours,4);
		display_second_digit(hours,5);
	}
}
