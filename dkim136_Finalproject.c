#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
volatile unsigned char timer_on = 0;
volatile unsigned char timer_done = 0;
volatile unsigned char timer_min_on = 0;
volatile unsigned char Clock_sec = 0;
volatile unsigned char Clock_sec_10 = 0;
volatile unsigned char on_alarm = 0;
volatile unsigned char done_alarm = 0;
volatile unsigned char min_on_alarm = 0;
volatile unsigned char hour_A_cnt = 0;
volatile unsigned char hour_A_cnt2 = 0;
volatile unsigned char hour_A_cnt3 = 0;
volatile unsigned char min_A_cnt = 0;
volatile unsigned char min_cnt_10 = 0;
volatile unsigned char min_A_cnt_10 = 0;
volatile unsigned char min_A_cnt2 = 0;
volatile unsigned char min_A_cnt3 = 0;
volatile unsigned char hour_cnt_10 = 0;
volatile unsigned char hour_A_cnt_10 = 0;
volatile unsigned char hour_cnt = 0;
volatile unsigned char hour_cnt2 = 0;
volatile unsigned char hour_cnt3 = 0;
volatile unsigned char min_cnt = 0;
volatile unsigned char min_cnt2 = 0;
volatile unsigned char A_flag = 0;
volatile unsigned char T_flag = 0;

void set_PWM(double frequency) {

	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on()
{
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum SET_TIME_STATES {init_time, hour_change, min_change, WaiT_T, Enter_TH, Enter_TM, Display} set_time_state;




void set_time()
{
	char button_hour = ~PINA & 0x10;
	char button_min = ~PINA & 0x08;
	char button_enter = ~PINA & 0x04;

	switch(set_time_state)
	{
		case init_time:
		if(T_flag)
		{
			set_time_state = hour_change;
		}
		else
		{
			set_time_state = init_time;
		}
		break;

		case hour_change:
		if(button_hour)
		{
			set_time_state = hour_change;
		}
		else if(!button_hour)
		{

			set_time_state = hour_change;
		}
		else if(button_enter)
		{
			set_time_state = Enter_TH;
		}
		break;

		case WaiT_T:
		if(button_hour)
		{
			set_time_state = hour_change;
		}
		// if(hour_cnt > 23 || button_min)
		// {
		//   set_time_state = min_change;
		// }
		if(!button_hour|| !button_min)
		{
			set_time_state = WaiT_T;
		}
		break;

		case min_change:
		if(button_min)
		{
			set_time_state = WaiT_T;
		}
		if(button_enter)
		{
			set_time_state = Enter_TM;
		}
		break;

		case Enter_TH:
		if(button_enter)
		{
			set_time_state = min_change;
		}



		default:
		break;
	}

	switch(set_time_state)
	{
		case init_time:
		LCD_Cursor(1);
		break;


		case hour_change:
		if(button_hour)
		{
			if(hour_cnt < 9)
			{
				hour_cnt = hour_cnt + 1;
				LCD_Cursor(1);
				LCD_WriteData(0 + '0');
				LCD_WriteData(hour_cnt + '0');

			}
			else if(hour_cnt < 19)
			{
				hour_cnt = hour_cnt + 1;
				LCD_Cursor(1);
				LCD_WriteData(1 + '0');
				LCD_Cursor(2);
				LCD_WriteData(hour_cnt2 + '0');
				hour_cnt2 = hour_cnt2 + 1;
			}
			else if(hour_cnt < 23)
			{
				hour_cnt = hour_cnt + 1;
				LCD_Cursor(1);
				LCD_WriteData(2 + '0');
				LCD_Cursor(2);
				LCD_WriteData(hour_cnt3 + '0');

				hour_cnt3 = hour_cnt3 + 1;
			}
			else if(hour_cnt >= 23)
			{
				LCD_Cursor(1);
				LCD_WriteData (0 + '0');
				LCD_Cursor(2);
				LCD_WriteData(0 + '0');
				hour_cnt = 0; hour_cnt2 = 0; hour_cnt3 = 0;
			}
		}
		if(button_enter)
		{
			break;
		}
		break;

		case WaiT_T:
		break;

		case Enter_TH:
		break;

		case Display:
		break;
	}
}


enum SETTING_TIME_STATES {Init, Flag} setting_time_state;
void SETTING_TIME()
{
	unsigned char button = ~PINA & 0x60;
	switch(setting_time_state)
	{
		case Init:
		if(button == 0x40)
		{
			setting_time_state = Flag;
		}
		else
		{
			setting_time_state = Init;
		}
		break;

		case Flag:
		setting_time_state = Flag;
		break;
	}

	switch(setting_time_state)
	{
		case Init:
		break;
		case Flag:
		T_flag = 1;
	}
}

enum SETTING_ALARM_STATES {Init_A, Flag_A} setting_alarm_state;
void SETTING_ALARM()
{
	unsigned char button_AF = ~PINA & 0x60;
	switch(setting_alarm_state)
	{
		case Init:
		if(button_AF == 0x40)
		{
			setting_alarm_state = Flag_A;
		}
		else
		{
			setting_alarm_state = Init_A;
		}
		break;

		case Flag_A:
		setting_alarm_state = Flag_A;
		break;
	}

	switch(setting_alarm_state)
	{
		case Init:
		break;
		case Flag_A:
		A_flag = 1;
	}
}
enum SET_ALARM_STATES {init_alarm, hour_a_change, min_a_change, Wait_A, Enter_AH, Enter_AM, Display_A} set_alarm_state;




void set_alarm()
{
	char button_hour_A = ~PINA & 0x10;
	char button_min_A = ~PINA & 0x08;
	char button_enter_A = ~PINA & 0x04;

	switch(set_alarm_state)
	{
		case init_alarm:
		if(A_flag)
		{
			set_alarm_state = hour_a_change;
		}
		else
		{
			set_alarm_state = init_alarm;
		}
		break;

		case hour_a_change:
		if(button_hour_A)
		{
			set_alarm_state = hour_a_change;
		}
		// else if(!button_hour_A)
		// {
		//
		// 	set_alarm_state = hour_a_change;
		// }
		else if(button_enter_A)
		{
			set_alarm_state = Enter_AH;
		}
		break;

		case Wait_A:
		if(button_hour_A)
		{
			set_alarm_state = hour_a_change;
		}
		if(button_min_A)
		{
			set_alarm_state = min_a_change;
		}
		if(!button_hour_A|| !button_min_A)
		{
			set_alarm_state = Wait_A;
		}
		break;

		case min_a_change:
		if(button_min_A)
		{
			set_alarm_state = min_a_change;
		}
		else if(button_enter_A)
		{
			set_alarm_state = Enter_AM;
		}
		break;

		case Enter_AH:
		//if(button_enter_A)
		//{
		set_alarm_state = min_a_change;
		//}
		break;

		case Enter_AM:
		if(button_enter_A)
		{
			set_alarm_state = Display_A;
		}

		case Display_A:
		break;

		default:
		break;
	}

	switch(set_alarm_state)
	{
		case init_alarm:
		LCD_Cursor(18);
		break;


		case hour_a_change:
		if(button_hour_A)
		{
			if(hour_A_cnt < 9)
			{
				hour_A_cnt = hour_A_cnt + 1;
				LCD_Cursor(18);
				LCD_WriteData(0 + '0');
				LCD_Cursor(19);
				LCD_WriteData(hour_A_cnt + '0');

			}
			else if(hour_A_cnt < 19)
			{
				hour_A_cnt = hour_A_cnt + 1;
				LCD_Cursor(18);
				LCD_WriteData(1 + '0');
				LCD_Cursor(19);
				LCD_WriteData(hour_A_cnt2 + '0');
				hour_A_cnt2 = hour_A_cnt2 + 1;
			}
			else if(hour_A_cnt < 23)
			{
				hour_A_cnt = hour_A_cnt + 1;
				LCD_Cursor(18);
				LCD_WriteData(2 + '0');
				LCD_Cursor(19);
				LCD_WriteData(hour_A_cnt3 + '0');

				hour_A_cnt3 = hour_A_cnt3 + 1;
			}
			else if(hour_A_cnt >= 23)
			{
				LCD_Cursor(18);
				LCD_WriteData (0 + '0');
				LCD_Cursor(19);
				LCD_WriteData(0 + '0');
				hour_A_cnt = 0; hour_A_cnt2 = 0; hour_A_cnt3 = 0;
			}
		}
		else if(!button_hour_A)
		{
			break;
		}
		else if(button_enter_A)
		{
			LCD_Cursor(21);
		}
		break;

		// case Wait_A:
		// LCD_ClearScreen();
		// LCD_ClearScreen();
		// LCD_Display_AString(1, "Wait Working");
		// break;
		case min_a_change:
		if(button_min_A){
			//LCD_ClearScreen();
			//LCD_ClearScreen();
			if(min_A_cnt < 9)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(0 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt + '0');
			}
			else if(min_A_cnt < 19)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(1 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt2 + '0');
				min_A_cnt2 = min_A_cnt2 + 1;
			}
			else if(min_A_cnt < 29)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(2 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt3 + '0');

				min_A_cnt3 = min_A_cnt3 + 1;
				min_A_cnt2 = 0;
			}
			else if(min_A_cnt < 39)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(3 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt2 + '0');

				min_A_cnt2 = min_A_cnt2 + 1;
				min_A_cnt3 = 0;
			}
			else if(min_A_cnt < 49)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(4 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt3 + '0');

				min_A_cnt3 = min_A_cnt3 + 1;
				min_A_cnt2 = 0;
			}
			else if(min_A_cnt < 59)
			{
				min_A_cnt = min_A_cnt + 1;
				LCD_Cursor(21);
				LCD_WriteData(5 + '0');
				LCD_Cursor(22);
				LCD_WriteData(min_A_cnt2 + '0');

				min_A_cnt2 = min_A_cnt2 + 1;
				min_A_cnt3 = 0;
			}
			else if(min_A_cnt >= 59)
			{
				LCD_Cursor(21);
				LCD_WriteData (0 + '0');
				LCD_Cursor(22);
				LCD_WriteData(0 + '0');
				min_A_cnt = 0; min_A_cnt2 = 0; min_A_cnt3 = 0;
			}


		}
		else if(button_enter_A)
		{
			set_alarm_state = Enter_AM;
		}
		break;

		case Enter_AH:
		LCD_Cursor(21);
		break;

		case Display_A:
		break;
	}
}



enum Clock_States { Int_Clk,  Clock_Tic,
cnt_clock , W_SEC, W_SEC_10, min_tic, min_tic_10, hour_tic, hour_tic_10} Clock_state;

void Clock_tic ()
{
	static unsigned char tick = 0;
	static unsigned char Clock_sec = 0;
	static unsigned char Clock_sec_10 = 0;
	//static unsigned char min_clk = 0;

	switch (Clock_state)
	{
		case Int_Clk :
		if (timer_done)
		{
			Clock_state =  Clock_Tic;
		}
		else if (!timer_done)
		{
			Clock_state = Int_Clk;
		}
		break;

		case Clock_Tic :
		if (tick >= 4)
		{
			Clock_state = cnt_clock;
		}
		else if (tick < 4)
		{
			Clock_state = Clock_Tic;
		}
		break;

		case cnt_clock :
		if (hour_cnt >= 9)
		{
			if(min_cnt_10 >= 5)
			{
				if(min_cnt >= 9)
				{
					if(Clock_sec >= 10)
					{
						if(Clock_sec_10 >= 5)
						{
							Clock_state = hour_tic_10;
						}
					}
				}
			}
		}

		else if (min_cnt_10 >= 5)
		{
			if(min_cnt >= 9)
			{
				if(Clock_sec >= 10)
				{
					if(Clock_sec_10 >= 5)
					{
						Clock_state = hour_tic_10;
					}
				}
			}
		}
		else if (min_cnt >= 9)
		{
			if(Clock_sec >= 10)
			{
				if(Clock_sec_10 >= 5)
				{
					Clock_state = hour_tic_10;
				}
			}
		}

		else if (Clock_sec >= 10)
		{
			if(Clock_sec_10 >= 5)
			{
				Clock_state = hour_tic_10;
			}
		}
		else if (Clock_sec < 10)
		{   Clock_state = W_SEC;}
		break;

		case W_SEC :
		Clock_state =Clock_Tic;
		break;

		case W_SEC_10 :
		Clock_state = Clock_Tic ;
		break;

		case min_tic :
		Clock_state = Clock_Tic ;
		break;

		case min_tic_10 :
		Clock_state = Clock_Tic ;
		break;
		case hour_tic :
		Clock_state = Clock_Tic ;
		break;
		case hour_tic_10 :
		Clock_state = Clock_Tic ;
		break;
	}

	switch (Clock_state)
	{
		case Int_Clk :
		//hour_cnt_10 = hour_cnt_10 + 1;
		LCD_Cursor(1);
		LCD_WriteData(hour_cnt_10 + '0') ;
		LCD_Cursor(2);
		LCD_WriteData(hour_cnt + '0') ;
		LCD_Cursor(4);
		LCD_WriteData(min_cnt_10 + '0') ;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		Clock_sec_10 = 0;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		tick = 0;

		break;

		case Clock_Tic :
		tick = tick +1;
		break;

		case cnt_clock :
		tick = 0;
		Clock_sec = Clock_sec +1;
		break;

		case W_SEC :
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;

		case W_SEC_10 :
		Clock_sec_10 = Clock_sec_10 + 1;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;

		case min_tic :
		min_cnt = min_cnt + 1;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		Clock_sec_10 = 0;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;

		case min_tic_10 :
		min_cnt_10 = min_cnt_10 + 1;
		LCD_Cursor(4);
		LCD_WriteData(min_cnt_10 + '0') ;
		min_cnt = 0;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		Clock_sec_10 = 0;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;

		case hour_tic :
		hour_cnt = hour_cnt + 1;
		LCD_Cursor(2);
		LCD_WriteData(hour_cnt + '0') ;
		min_cnt_10 = 0;
		LCD_Cursor(4);
		LCD_WriteData(min_cnt_10 + '0') ;
		min_cnt = 0;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		Clock_sec_10 = 0;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;
		case hour_tic_10 :
		hour_cnt_10 = hour_cnt_10 + 1;
		LCD_Cursor(1);
		LCD_WriteData(hour_cnt_10 + '0') ;
		hour_cnt = 0;
		LCD_Cursor(2);
		LCD_WriteData(hour_cnt + '0') ;
		min_cnt_10 = 0;
		LCD_Cursor(4);
		LCD_WriteData(min_cnt_10 + '0') ;
		min_cnt = 0;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		Clock_sec_10 = 0;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		Clock_sec = 0;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;
	}
}

enum Set_Alarm_States {INIT_ALM, START_ALM} state_Alm;
void Set_Alarm ()
{
	unsigned char button_alm = ~PINA & 0x02;
	unsigned char semicolon = 10;
	switch (state_Alm)
	{
		case INIT_ALM :
		if (button_alm)
		{
			state_Alm = START_ALM;
		}

		else{
			state_Alm = INIT_ALM;
		}
		break;

		case START_ALM :
		state_Alm = START_ALM ;

		default :
		state_Alm = INIT_ALM;
	}

	switch (state_Alm)
	{
		case INIT_ALM :
		on_alarm = 0;
		break;

		case START_ALM :
		on_alarm = 1;
		LCD_DisplayString(17, " 00:00");
		LCD_Cursor(1);
		LCD_WriteData(hour_cnt_10 + '0') ;
		LCD_Cursor(2);
		LCD_WriteData(hour_cnt + '0') ;
		LCD_Cursor(3);
		LCD_WriteData(semicolon + '0') ;
		LCD_Cursor(4);
		LCD_WriteData(min_cnt_10 + '0') ;
		LCD_Cursor(5);
		LCD_WriteData(min_cnt + '0') ;
		LCD_Cursor(6);
		LCD_WriteData(semicolon + '0') ;
		LCD_Cursor(7);
		LCD_WriteData(Clock_sec_10 + '0') ;
		LCD_Cursor(8);
		LCD_WriteData(Clock_sec + '0') ;
		break;
	}
}


enum Output_States {intial_output, check_output, OUTPUT, dismiss, snooze} output_state ;

void Output_Funct ()
{
	unsigned char tempB = 0x00;
	unsigned char button_off = ~PINA & 0x40;
	unsigned char button_snooze = ~PINA & 0x20;
	static short output_flag = 0;

	switch (output_state)
	{
		case intial_output :
		if (done_alarm)
		{
			output_state = check_output;
		}
		else
		{
			output_state = intial_output;
		}
		break;

		case check_output :
		if (( hour_cnt_10 == hour_A_cnt_10 )  && (hour_cnt == hour_A_cnt) &&
		(min_cnt_10 == min_A_cnt_10) && (min_cnt == min_A_cnt))
		{
			output_state = OUTPUT;
		}
		else{
			output_state = check_output;
		}
		break;

		case OUTPUT :
		if (button_off)
		{
			output_state = dismiss;
		}
		else if (button_snooze)
		{
			output_state = snooze;
		}
		else
		output_state = OUTPUT;
		break;

		case dismiss :
		if (!button_off)
		{
			output_state = check_output ;
		}
		else
		{
			output_state = dismiss;
		}
		break;

		case snooze :
		if (output_flag >= 240)
		{
			output_state = OUTPUT ;
		}
		else if (output_flag < 240)
		{
			output_state= snooze;
		}
		break;

		default :
		output_state = intial_output;
		break;

	}

	switch (output_state) {
		case intial_output :
		tempB = 0x00;
		output_flag = 0;
		break;

		case OUTPUT:
		tempB = 0x01;
		output_flag = 0;
		set_PWM(281);
		break;

		case dismiss :
		tempB = 0x00;
		set_PWM(0);
		break;

		case snooze :
		output_flag = output_flag + 1;
		tempB = 0x00;
		set_PWM(0);
		break;
	}

	PORTB = tempB;
}
int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	setting_time_state = Init;
	set_time_state = init_time;

	LCD_init();

	TimerSet(250);
	TimerOn();

	PWM_on();
	set_PWM(0);

	LCD_init();
	LCD_Cursor(1);
	LCD_DisplayString(1, "00:00");
	LCD_Cursor(17);
	LCD_DisplayString(17," 00:00");

	while(1) {

		SETTING_TIME();
		set_time();
		Clock_tic();
		SETTING_ALARM();
		set_alarm();

		Output_Funct();
		while(!TimerFlag);
		TimerFlag = 0;

	}
}
