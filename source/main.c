/*	Author: Saul Mendoza
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #10  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: 
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
unsigned char ball_row = 0x00;
unsigned char ball_column = 0x00;
unsigned char P1_column = 0x00;
unsigned char P1_row = 0x00;
unsigned char P2_column = 0x00;
unsigned char P2_row = 0x00;
//-----------end Globals----------------

//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Ball_States {shift_right, shift_left};

int Ball_Movement(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	//static unsigned char row = 0xFB;  	// Row(s) displaying pattern. 
							// 0: display pattern on row
							// 1: do NOT display pattern on row
	// Transitions
	switch (state) {
		case shift_right:
			if ( pattern == 0x01 )
				state = shift_left;
			else
				state = shift_right;	
			break;
		case shift_left:
			if ( pattern == 0x80 )
				state = shift_right;
			else
				state = shift_left;
			break;
		default:	
			state = shift_right;
			break;
	}	
	// Actions
	switch (state) {
		case shift_right:	
				pattern >>= 1;
			break;
		case shift_left:
				pattern <<= 1;
			break;
		default:
			break;
	}
	ball_column = pattern;	// Pattern to display
	//ball_row = row;		// Row(s) displaying pattern	
	return state;
}

enum P1_States { P1_Wait, P1_MoveUp, P1_MoveDown };

int P1_Movement (int state) {
	static unsigned char column = 0x80;
	static unsigned char rows[8] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	static unsigned char i = 0;
	
	switch (state) {
		case P1_Wait:
			state = P1_Wait;
			break;
		case P1_MoveDown:
			break;
		default:
			state = P1_Wait;
			break;
	}
	
	switch (state) {
		case P1_Wait:
			if ( i < 8 ) {
				PORTD = rows[i];
				i++;
			}
			else
				i = 0;
			break;
		case P1_MoveDown:
			break;
	}
			
	
	P1_column = column;
	return state;
}

enum P2_States { P2_Wait, P2_MoveUp, P2_MoveDown };

int P2_Movement (int state) {
	static unsigned char column = 0x01;
	static unsigned char rows[5] = { 0x00, 0xFD, 0xFB, 0xF7, 0xFF };
	static unsigned char i = 0;

	switch (state) {
		case P2_Wait:
			state = P2_Wait;
			break;
		case P2_MoveDown:
			break;
		default:
			state = P2_Wait;
			break;
	}
	
	switch (state) {
		case P2_Wait:
			if( i  < 4 ) {
				PORTD = rows[i];
				i++;
			}
			else
				i = 0;
			break;
		case P2_MoveDown:
			break;
	}
	
	P2_column = column;
	return state;
}

enum display_States { display };

// Combine outputs from SM's, and output on PORTB
int displaySMTick(int state) {
	// Local Variables
	static unsigned char Column_output = 0x80;
	static unsigned char Row_output;
	
	switch (state) { //State machine transitions
		case display: 
			state = display; 
			break;
		default: 
			state = display; 
			break;
	}
	switch(state) { //State machine actions
		case display:	
			if (Column_output == 0x01 )
				Column_output = 0x80;
			else
				Column_output
			Column_output =; // write shared outputs
			Row_output = P1_row | P2_row/* | ball_row*/;	// to local variables
		break;
	}
	PORTC = Column_output;
	//PORTD = Row_output;// Write combined, shared output variables to PORTB
	return state;
}

			
	
int main() {
	DDRC = 0xFF; PORTC = 0x00; // PORTB set to output, outputs init 0s
	DDRD = 0xFF; PORTD = 0x00; // PC7..4 outputs init 0s, PC3..0 inputs init 1s

	//Declare an array of tasks 
	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = 0;
	// Task 1 (Ball_Movement)
/*	task1.state = start;//Task initial state.
	task1.period = 100;//Task Period.
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &Ball_Movement;//Function pointer for the tick.
*/	// Task 2 (P1_Movement)
	task1.state = start;//Task initial state.
	task1.period = 1;//Task Period.
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &P1_Movement;//Function pointer for the tick.
	// Task 3 (P1_Movement)
	task2.state = start;//Task initial state.
	task2.period = 1;//Task Period.
	task2.elapsedTime = task2.period;//Task current elapsed time.
	task2.TickFct = &P2_Movement;//Function pointer for the tick.
	// Task 4 (displaySMTick)
	task3.state = start;//Task initial state.
	task3.period = 1;//Task Period.
	task3.elapsedTime = task3.period;//Task current elapsed time.
	task3.TickFct = &displaySMTick;//Function pointer for the tick.
	
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
