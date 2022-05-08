/*	Author: Saul Mendoza
 *  	Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #11  Exercise #1
 *	Exercise Description: Ping Pong game vs AI using lcd screen.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtube.com/shorts/cyE-3rrtRfc
 *
 */


#include <avr/io.h>
#include "timer.h"
#include "scheduler.h"
#include "bit.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//-----------Gobal Declarations---------
unsigned char Ball_Location[8] = { 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char P1_Rows[8] = { 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char P2_Rows[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1 };
int current = 3;
int Ball_Speed = 100;
unsigned int start = 0;
//-----------end Globals----------------

enum Start_States { Reset, Start, Reset_Hold, Start_Hold };

int StartSM (int state) {
	switch (state) {
		case Reset:
			if ((~PINA & 0x10) == 0x10)
				state = Start_Hold;
			else if((~PINA & 0x10) == 0x00)
				state = Reset;
			break;
		case Reset_Hold:
			if ((~PINA & 0x10) == 0x10)
				state = Reset_Hold;
			else if((~PINA & 0x10) == 0x00)
				state = Reset;
			break;
		case Start_Hold:
			if ((~PINA & 0x10) == 0x10)
				state = Start_Hold;
			else if ((~PINA & 0x10) == 0x00)
				state = Start;
			break;
		case Start:
			if ((~PINA & 0x10) == 0x10)
				state = Reset_Hold;
			else if ((~PINA & 0x10) == 0x00)
				state = Start;
			break;
		default:
			state = Reset;
			break;
	}

	switch (state) {
		case Reset:
			start = 0;
			break;
		case Reset_Hold:
			break;
		case Start_Hold:
			break;
		case Start:
			start = 1;
			break;
		default:
			break;
	}
	
	return state;
}

enum P1_States { P1_Init, P1_Wait, P1_MoveUp, P1_MoveDown };

int P1_Movement (int state) {
	static unsigned char paddle[3] = { 0xF8, 0xF1, 0xE3 };
	static unsigned char i = 1;

	switch (state) {
		case P1_Init:
			if(start == 1)
				state = P1_Wait;
			else if(start == 0)
				state = P1_Init;
			break;
		case P1_Wait:
			if(start == 0)
				state = P1_Init;
			else if(((~PINA & 0x01) == 0x01) && i>0 )
				state = P1_MoveUp;
			else if(((~PINA & 0x02) == 0x02) && i<2 )
				state = P1_MoveDown;
			else
				P1_Wait;
			break;
		case P1_MoveUp:
			if (start == 0)
				state = P1_Init;
			else
				state = P1_Wait;
			break;
		case P1_MoveDown:
			if (start == 0)
				state = P1_Init;
			else 
				state = P1_Wait;
			break;
		default:
			state = P1_Init;
			break;
	}

	switch (state) {
		case P1_Init:
			P1_Rows[0] = 0xF1;
			i = 1;
			break;
		case P1_Wait:
			break;
		case P1_MoveUp:
			i--;
			P1_Rows[0]  = paddle[i];
			break;
		case P1_MoveDown:
			i++;
			P1_Rows[0] = paddle[i];
			break;
	}



	return state;
}

enum P2_States { P2_Init, P2_Wait, P2_MoveUp, P2_MoveDown };

int P2_Movement (int state) {
	static unsigned char paddle[3] = { 0xF8, 0xF1, 0xE3 };
	static unsigned char i = 1;

	switch (state) {
		case P2_Init:
			if(start == 1)
				state = P2_Wait;
			else if(start == 0)
				state = P2_Init;
			break;
		case P2_Wait:
			if(start == 0)
				state = P2_Init;
			else
				state = P2_MoveUp;
			break;
		case P2_MoveUp:
			if(start == 0)
				state = P2_Init;
			else if (i == 0)
				state = P2_MoveDown;
			else
				state = P2_MoveUp;
			break;
		case P2_MoveDown:
			if(start == 0)
				state = P2_Init;
			else if (i == 2)
				state = P2_MoveUp;
			else 
				state = P2_MoveDown;
			break;
		default:
			state = P2_Init;
			break;
	}

	switch (state) {
		case P2_Init:
			P2_Rows[7] = 0xF1;
			i = 1;
			break;
		case P2_Wait:
			break;
		case P2_MoveUp:
			P2_Rows[7]  = paddle[i];
			i--;
			break;
		case P2_MoveDown:
			P2_Rows[7] = paddle[i];
			i++;
			break;
	}



	return state;
}

enum BallY_States { BY_Init, BY_Wait, shift_down, shift_up };

int Ball_YMovement(int state){
	static unsigned char previous = 0x00;

	switch(state) {
		case BY_Init:
			if(start == 1)
				state = shift_down;
			else if(start == 0)
				state = BY_Init;
			break;
		case BY_Wait:
			if(start == 0)
				state = BY_Init;
			else if( Ball_Location[current] == ((previous >> 1) | 0x80))
				state = shift_up;
			else if( Ball_Location[current] == ((previous << 1) | 0x01))
				state = shift_down;
			else
				state = BY_Wait;
			break;
		case shift_down:
			if(start == 0)
				state = BY_Init;
			else if ( Ball_Location[current] == 0xEF && previous == 0xF7 )
				state = shift_up;
			else
				state = shift_down;	
			break;
		case shift_up:
			if(start == 0)
				state = BY_Init;
			else if ( Ball_Location[current] == 0xFE && previous == 0xFD )
				state = shift_down;
			else
				state = shift_up;
			break;
		default:
			state = BY_Init;
			break;
	}

	switch(state) {
		case BY_Init:
			previous = 0x00;
			break;
		case BY_Wait:
			break;
		case shift_down:
			previous = Ball_Location[current];
			Ball_Location[current] = ( Ball_Location[current] << 1 ) | 0x01;
			break;
		case shift_up:
			previous = Ball_Location[current];
			Ball_Location[current] = ( Ball_Location[current] >> 1 ) | 0x80;
			break;
		default:
			break;
	}

	return state;
}

enum BallX_States { BX_Init, shift_left, shift_right };

int Ball_XMovement(int state) {
	static unsigned char x = 0x00;
	
	switch (state) {
		case BX_Init:
			if(start == 1)
				state = shift_right;
			else
				state = BX_Init;
			break;
		case shift_right:
			if(start == 0)
				state = BX_Init;
			else if ( current == 6 )
				state = shift_left;
			else
				state = shift_right;
			break;
		case shift_left:
			if(start == 0)
				state = BX_Init;
			else if ( current == 1 )
				state = shift_right;
			else
				state = shift_left;
			break;
		default:
			state = BX_Init;
			break;

	}

	switch (state) {
		case BX_Init:
			x = 0x00;
			current = 3;
			Ball_Location[0] = Ball_Location[1] = Ball_Location[2] = 0xFF;
			Ball_Location[4] = Ball_Location[5] = Ball_Location[6] = 0xFF;
			Ball_Location[7] = 0xFF;
			Ball_Location[3] = 0xFB;
			break;
		case shift_right:
			x = Ball_Location[current];
			Ball_Location[current] = Ball_Location[current+1];
			Ball_Location[current+1] = x;
			current++;
			break;
		case shift_left:
			x = Ball_Location[current];
			Ball_Location[current] = Ball_Location[current-1];
			Ball_Location[current-1] = x;
			current--;
		default:
			break;
	}
	return state;
}



//--------------------------------------
// LED Matrix Display SM
//--------------------------------------
enum Display_States { display };

int DisplaySM(int state) {

	// Local Variables
	static unsigned char column = 0x80;
	static unsigned char i = 0;
	static unsigned char P1_Paddle = 0x00;
	static unsigned char P2_Paddle = 0x00;
	static unsigned char Ball = 0x00;
	// Transitions
	switch (state) {
		case display:	
			break;
		default:	
			state = display;
			break;
	}	
	// Actions
	switch (state) {
		case display:	
			if (column == 0x01) {
				i = 0;
				P1_Paddle = P1_Rows[i];
				P2_Paddle = P2_Rows[i];
				Ball = Ball_Location[i];
				column = 0x80;
				i++;		
			}
			else {
				P1_Paddle = P1_Rows[i];
				P2_Paddle = P2_Rows[i];
				Ball = Ball_Location[i];
				column >>= 1;
				i++;
			
			}
			break;
		default:
			break;
	}
	PORTC = column	;				// Pattern to display
	PORTD = P1_Paddle & P2_Paddle & Ball;		// Row(s) displaying pattern	
	return state;
}

int main() {
	DDRA = 0x00; PORTA = 0xFF; // Initialize PORTA to input
	DDRC = 0xFF; PORTC = 0x00; // Initialize PORTC to ouput
	DDRD = 0xFF; PORTD = 0x00; // PC7..4 outputs init 0s, PC3..0 inputs init 1s

	//Declare an array of tasks 
	static task task1, task2, task3, task4, task5, task6;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	const char start = 0;
	// Task 1 (Start)
	task1.state = start;//Task initial state.
	task1.period = 100;//Task Period.
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &StartSM;//Function pointer for the tick.
	// Task 2 (Paddle 1)
	task2.state = start;//Task initial state.
	task2.period = 100;//Task Period.
	task2.elapsedTime = task2.period;//Task current elapsed time.
	task2.TickFct = &P1_Movement;//Function pointer for the tick.
	// Task 2 (Paddle 2)
	task3.state = start;//Task initial state.
	task3.period = 200;//Task Period.
	task3.elapsedTime = task3.period;//Task current elapsed time.
	task3.TickFct = &P2_Movement;//Function pointer for the tick.
	// Task 4 (Ball Y Axis)
	task4.state = start;//Task initial state.
	task4.period = Ball_Speed;//Task Period.
	task4.elapsedTime = task4.period;//Task current elapsed time.
	task4.TickFct = &Ball_YMovement;//Function pointer for the tick.
	//Task 5 (Ball X Axis)
	task5.state = start;//Task initial state.
	task5.period = Ball_Speed;//Task Period.
	task5.elapsedTime = task5.period;//Task current elapsed time.
	task5.TickFct = &Ball_XMovement;//Function pointer for the tick.
	// Task 6 (Display)
	task6.state = start;//Task initial state.
	task6.period = 1;//Task Period.
	task6.elapsedTime = task6.period;//Task current elapsed time.
	task6.TickFct = &DisplaySM;//Function pointer for the tick.

	
	unsigned short j;
	unsigned long GCD = tasks[0]->period;
	for ( j = 1; j < numTasks; j++ ) {
		GCD = findGCD(GCD,tasks[j]->period);
	}	


	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	unsigned short i; // Scheduler for-loop iterator
	while(1) {	
		for ( i = 0; i < numTasks; i++ ) { // Scheduler code
			if ( tasks[i]->elapsedTime == tasks[i]->period ) { // Task is ready to tick
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state); // Set next state 
				tasks[i]->elapsedTime = 0; // Reset the elapsed time for next tick.
			}
			tasks[i]->elapsedTime += GCD;
		}
		while(!TimerFlag);
			TimerFlag = 0;
	}
	return 0; // Error: Program should not exit!
}
