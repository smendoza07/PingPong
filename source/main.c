
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
//-----------end Globals----------------


enum P1_States { P1_Init, P1_Wait, P1_MoveUp, P1_MoveDown };

int P1_Movement (int state) {
	static unsigned char paddle[3] = { 0xF8, 0xF1, 0xE3 };
	static unsigned char i = 1;

	switch (state) {
		case P1_Init:
			state = P1_Wait;
			break;
		case P1_Wait:
			state = P1_MoveDown;
			break;
		case P1_MoveUp:
			if ( i == 0)
				state = P1_MoveDown;
			else
				state = P1_MoveUp;
			break;
		case P1_MoveDown:
			if ( i == 2 )
				state = P1_MoveUp;
			else 
				state = P1_MoveDown;
			break;
		default:
			state = P1_Wait;
			break;
	}

	switch (state) {
		case P1_Init:
			break;
		case P1_Wait:
			break;
		case P1_MoveUp:
			P1_Rows[0]  = paddle[i];
			i--;
			break;
		case P1_MoveDown:
			P1_Rows[0] = paddle[i];
			i++;
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
			state = P2_Wait;
			break;
		case P2_Wait:
			state = P2_MoveUp;
			break;
		case P2_MoveUp:
			if ( i == 0)
				state = P2_MoveDown;
			else
				state = P2_MoveUp;
			break;
		case P2_MoveDown:
			if ( i == 2 )
				state = P2_MoveUp;
			else 
				state = P2_MoveDown;
			break;
		default:
			state = P2_Wait;
			break;
	}

	switch (state) {
		case P2_Init:
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

enum Ball_States { B_Init, shift_left, shift_right };

int Ball_Movement(int state) {
	static unsigned char x = 0x00;
	static unsigned char i = 3;
	switch (state) {
		case B_Init:
			state = shift_right;
			break;
		case shift_right:
			if ( i == 6 )
				state = shift_left;
			else
				state = shift_right;
			break;
		case shift_left:
			if ( i == 1 )
				state = shift_right;
			else
				state = shift_left;
			break;
		default:
			state = B_Init;
			break;

	}

	switch (state) {
		case B_Init:
			break;
		case shift_right:
			x = Ball_Location[i];
			Ball_Location[i] = Ball_Location[i+1];
			Ball_Location[i+1] = x;
			i++;
			break;
		case shift_left:
			x = Ball_Location[i];
			Ball_Location[i] = Ball_Location[i-1];
			Ball_Location[i-1] = x;
			i--;
		default:
			break;
	}
	return state;
}



//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
enum Demo_States {shift};

int Demo_Tick(int state) {

	// Local Variables
	static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
	static unsigned char i = 0;
	static unsigned char P1_Paddle = 0x00;
	static unsigned char P2_Paddle = 0x00;
	static unsigned char Ball = 0x00;
	// Transitions
	switch (state) {
		case shift:	
			break;
		default:	
			state = shift;
			break;
	}	
	// Actions
	switch (state) {
		case shift:	
			if (pattern == 0x01) { // Reset demo 
				i = 0;
				P1_Paddle = P1_Rows[i];
				P2_Paddle = P2_Rows[i];
				Ball = Ball_Location[i];
				pattern = 0x80;
				i++;		
			}
			else { // Shift LED one spot to the right on current row
				P1_Paddle = P1_Rows[i];
				P2_Paddle = P2_Rows[i];
				Ball = Ball_Location[i];
				pattern >>= 1;
				i++;
			
			}
			break;
		default:
			break;
	}
	PORTC = pattern;	// Pattern to display
	PORTD = P1_Paddle & P2_Paddle & Ball;		// Row(s) displaying pattern	
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
	// Task 1 (Paddle 1)
	task1.state = start;//Task initial state.
	task1.period = 200;//Task Period.
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &P1_Movement;//Function pointer for the tick.
	// Task 2 (Paddle 2)
	task2.state = start;//Task initial state.
	task2.period = 200;//Task Period.
	task2.elapsedTime = task2.period;//Task current elapsed time.
	task2.TickFct = &P2_Movement;//Function pointer for the tick.
	// Task 3 (Ball_Movement)
	task3.state = start;//Task initial state.
	task3.period = 100;//Task Period.
	task3.elapsedTime = task3.period;//Task current elapsed time.
	task3.TickFct = &Ball_Movement;//Function pointer for the tick.
	// Task 4 (DisplaySM)
	task4.state = start;//Task initial state.
	task4.period = 1;//Task Period.
	task4.elapsedTime = task4.period;//Task current elapsed time.
	task4.TickFct = &Demo_Tick;//Function pointer for the tick.

	
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
