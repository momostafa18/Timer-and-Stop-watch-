/*
 * My_project.c
 *
 *  Created on: Sep 14, 2022
 *      Author: mohamedmostafamohame
 */
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
//--------------------------------------------------------------------------------------
//Some global variables
unsigned char SECONDS=0;   //Seconds Variable
unsigned char MINUTES=0;   //Minutes Variable
unsigned char HOURS=0;     //Hours Variable
//--------------------------------------------------------------------------------------
//Prototypes of the functions
void Timer1_init(void);
void INT0_init_RESET(void);
void INT1_init_PAUSE(void);
void INT2_init_RESUME(void);
//--------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	/*
	 1.Always incrementing the seconds after each Interrupt's fire which is after 1 second.
	 2.If the seconds counter reaches the maximum value so it's time to increment the minutes.
	 3.If the minutes counter reaches the maximum value so it's time to increment hours.
	 4.If the hours counter reaches the maximum value, the counter will reset itself and begin from scratch.
	*/
	SECONDS++;
	if(SECONDS>59)
	{
		SECONDS=0;
		MINUTES++;
	}
	if(MINUTES>59)
	{
		MINUTES=0;
		HOURS++;
	}
	if(HOURS>12)
	{
		HOURS=0;
	}
}

ISR(INT0_vect)
{
	/*
	 1.Resetting the timer needs just to put zero everywhere
	 2.Calling the function of TImer1 because when we reset after the pause option the timer will complete incrementing
	 3.Clearing the TCNT1 to make sure that the timer every time we reset starts from 0.
	 */
	SECONDS=0;
	MINUTES=0;
	HOURS=0;
	TCNT1=0;
	Timer1_init();
}

ISR(INT1_vect)
{
	/*
		To pause the stop watch we need only to stop the timer by closing the clock so we cleared the register TCCR1B.
	*/
	TCCR1B=0x00;
}

ISR(INT2_vect)
{
	/*
	 To resume the timer, it's only required to go back to the last setting of the timer before stopping it so we call the timer function to return to the last settings
	*/
	TCCR1B =(1<<CS10)|(1<<CS12);
}
int main(void)
{
	DDRC|=0x0F;                     //Setting the first 4 pins in port c as output
	PORTC&=0xF0;                    //Clearing the first 4 bits in port c so the 7-segment displays 0
	DDRA|=0x3F;                     //Setting the first 6 pins in port a as output
	PORTA|=0x3F;                    //Setting the first 6 pins in port a as they are the enable of the multiplexed 7-segment
	//Calling the functions.
	INT0_init_RESET();
	INT1_init_PAUSE();
	INT2_init_RESUME();
	Timer1_init();
	while(1)
	{
		/*
		 1.The logic of the code depends on that the delay is so small that can't be noticed by eye
		 so it appears that all the multiplexed 7-segment displays together but in real each 7-segment displays
		 alone by activating only this 7-segment and turning off the other ones then wait for 3 milliseconds
		 and the next one starts to display the number which it's his turn by activating it and turning of the
		 others and so on, 3 milliseconds is not a noticeable time so it appears that all the multiplexed
		 7-segment works together.

		 2.The Units of the seconds or the minutes or the hours is calculated using the modulus operator
		 as doing modulus by 10 always gives a number from 0 to 9 which is the units of the original
		 number.
		       EX:  19%10=9 ----> so we got the units of the original number 19 true by using modulus
		 3.The Tens of the seconds or the minutes or the hours is calculated using the devision operator
		 as doing devision by 10 always give number from 0 to 9 which is the tens of the original number.
		       EX:  19/10=1 -----> so we got the tens of the original number 19 true by using division
		 4.we always activate one of the 7-segments depending on the turn this technique is used with PC
		 and called round robin where the PC gives a slice of time for each running task,which is the used
		 technique.
		 */
		PORTA=(1<<PA5);
		PORTC=(PORTC&0xF0)|(SECONDS%10);
		_delay_ms(5);
		PORTA=(1<<PA4);
		PORTC=(PORTC&0xF0)|(SECONDS/10);
		_delay_ms(5);
		PORTA=(1<<PA3);
		PORTC=(PORTC&0xF0)|(MINUTES%10);
		_delay_ms(5);
		PORTA=(1<<PA2);
		PORTC=(PORTC&0xF0)|(MINUTES/10);
		_delay_ms(5);
		PORTA=(1<<PA1);
		PORTC=(PORTC&0xF0)|(HOURS%10);
		_delay_ms(5);
		PORTA=(1<<PA0);
		PORTC=(PORTC&0xF0)|(HOURS/10);
		_delay_ms(5);
	}
}
void Timer1_init(void)
{
	/*  //Function for Initializing the Timer1 settings//
	 1.Dealing with CTC mode so Setting bit FOC1A.
	 2.Dealing with Input capture register since the value of the compare is fixed.
	 3.Because of dealing with ICR as the compare register we set bits WGM12 and WGM13
	 4.Setting the compare value to 1000 by calculating it.
	 5.Using prescaler 1024 so we sit bits CS10 and CS12
	 6.The reason of using the prescaler is that the time of one tick without the prescalar would be 1us so we have to count 1000000 which is not available in this timer1.
	 7.Setting the I bit and the Timer Compare interrupt bit, So when the match occurs the flag will be set and the cpu will move to the ISR of Timer1
	*/
	TCCR1A=(1<<FOC1A);
	TCCR1B=(1<<CS10)|(1<<WGM12)|(1<<CS12)|(1<<WGM13);
	ICR1=1000;
	SREG|=(1<<7);
	TIMSK|=(1<<OCIE1A);
}
void INT0_init_RESET(void)
{
	/*//Function for initializing INT0 settings//
	 1.Setting the External interrupt bit .
	 2.Setting the I bit.
	 3.Setting the MCUCR register on the falling edge.
	*/
	DDRD&=~(1<<PD2);                //Setting Pin 2 in port D as input
	PORTD|=(1<<PD2);                //Since PD2 is an input pin and we make it's output =1 so the Pull up resistor is enabled
	GICR|=(1<<INT0);
	SREG|=(1<<7);
	MCUCR=(1<<ISC01);
}
void INT1_init_PAUSE(void)
{
	/*//Function for initializing INT1 settings//
		 1.Setting the External interrupt bit .
		 2.Setting the I bit.
		 3.Setting the MCUCR register on the rising edge.
		*/
	DDRD&=~(1<<PD3);      //Setting PD3 as input
	GICR|=(1<<INT1);
    SREG|=(1<<7);
    MCUCR=(1<<ISC11)|(1<<ISC10);
}
void INT2_init_RESUME(void)
{
	/*//Function for initializing INT1 settings//
	       1.Setting the External interrupt bit .
		   2.Setting the I bit.
		   3.We don't need to clear the MCUCSR because it's initial value is the required value which is 0 to deal with the falling edge.
	*/
	DDRB&=~(1<<PB2);                //Setting PB2 as input
	PORTB|=(1<<PB2);            //Since PB2 is an input pin and we make it's output =1 so the Pull up resistor is enabled
	GICR|=(1<<INT2);
    SREG|=(1<<7);
}
