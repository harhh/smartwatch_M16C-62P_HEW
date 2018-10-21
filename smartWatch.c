#include "smartWatch.h"
#include "qsk_bsp.h"

static int stopwatch_state = STOPWATCHSTOP;
static int volatile state = 0;
static int stopwatch = 0;	//0:mils, 1:(mils)+sec 2:(mils+)sec+min 3: (mils+)sec+min+hr 4:(mils+)sec+min+hour+date..
static int timerwatch = 1;

//unsigned short year = 1000; don't need this form

static unsigned char alarm_light = 0;
static unsigned short light = 0;

//for bottom half
static unsigned char bool_int1 = 0;
static unsigned char bool_int2 = 0;
static unsigned char bool_S3 = 0;

static unsigned short tOffset = DAY;

static TIME time;
static TIME mTime;
static STIME sTime;
static ATIME aTime;
static LAP lab;

/*about Init fuction*/
void InitSmartWatch()
{
	InitLED();			
	InitDisplay();
	InitVar();
	LCD_write(CTRL_WR, LCD_CLEAR);
//	LCD_write(CTRL_WR, LCD_CURSOR_ON);
	myInitDisplay(1);
	myInitDisplay(2);
	InitSwitch();
	InitTimer();
	ENABLE_IRQ;
	state = 1;//
}

void InitVar()//init variables as init value
{
	time.day = mTime.day = SUN;
	time.second = mTime.second = 0;
	time.minute = mTime.minute = 0;
	time.hour = mTime.hour = 0;
	time.month = mTime.month = 1;
	time.date = mTime.date = 1;
	
	sTime.s_msecond = 0;
 	sTime.s_second = 0;
 	sTime.s_minute = 0;
 	sTime.s_hour = 0;
	
	aTime.a_hour = 0;
	aTime.a_minute = 0;
	aTime.a_day = 0x00;
	
	lab.count = 0;
	lab.forRead = 0;	
}

void InitLED()
{
	/*LED Direction Register = output(1)*/
	RED_DDR = 1;
	YLW_DDR = 1;
	GRN_DDR = 1;
	/*LED off*/
	RED_LED = LED_OFF;
	YLW_LED = LED_OFF;
	GRN_LED = LED_OFF;
}

void InitSwitch()
{
	/*Switch Direction Register = input(0)*/
	S1_DDR = 0;	//INT1	
	S2_DDR = 0;	//INT0			
	S3_DDR = 0; 
	//Set Interrupt Register*/
	ilvl1_int0ic = 1;	//int0's priority : 010
	ilvl1_int1ic = 1;	//int1's priority : 010
	pol_int0ic = 1; pol_int1ic = 1;	//int0,1's polarity : falling edge
}	

void InitTimer()
{
	//TA1 
	/*set Timer Interrupt Register*/
	ilvl2_ta1ic = 1;	//priority : 100
	/*set Mode Register*/
	tmod1_ta1mr = 0;	tmod0_ta1mr = 1;	//event counter mode
	/*set Trigger Select Register*/
	ta1tgh = 1;	ta1tgl = 1;	//TA2 overflow count
	/*set Data Register*/
	ta1 = 100-1;
	
	//TA2
	/*set Timer Interrupt Register*/
	ilvl2_ta2ic = 1;	//priority : 100
	/*set Mode Register*/	
	tmod1_ta2mr = 0;	tmod0_ta2mr = 0;	//timer mode
	mr0_ta2mr = 0;	//pulse outputnot
	tck1_ta2mr = 0; tck0_ta2mr = 1;	//8MHz
	/*set Data Register*/
	ta2 = 30000-1;
	
	//TA0
	ilvl2_ta0ic = 1;	//priority : 100
	/*set Mode Register*/	
	tmod1_ta0mr = 0;	tmod0_ta0mr = 0;	//timer mode
	mr0_ta0mr = 0;	//pulse output not**********
	tck1_ta0mr = 0; tck0_ta0mr = 1;	//8MHz
	/*set Data Register*/
	ta0 = 30000-1;
	
	/*Count Start Register*/
	ta1s = 1;
	ta2s = 1;	//start counting
}


/*about display function*/
void myInitDisplay(int select)
{
	if(select == 1){	//SUN01/01	00:00:00
		displayDay(time.day);
		displayMonth(time.month);		
		displayDate(time.date);
		LCD_write(CTRL_WR, LCD_HOME_L2);
		displayHour(time.hour);
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
		displayMinute(time.minute);			
		displaySecond(time.second);
	}
	else if (select == 2){	//'/'':'':'
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 5) );
		LCD_write(DATA_WR, '/');		
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 2) );
		LCD_write(DATA_WR, ':');		
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 5) );
		LCD_write(DATA_WR, ':');		
	}
}

void displayDay(unsigned char __day)
{
	LCD_write(CTRL_WR, LCD_HOME_L1);
	if(__day == SUN){
		LCD_write(DATA_WR, 'S');		
		LCD_write(DATA_WR, 'U');		
		LCD_write(DATA_WR, 'N');		
	}
	else if(__day == MON){
		LCD_write(DATA_WR, 'M');		
		LCD_write(DATA_WR, 'O');		
		LCD_write(DATA_WR, 'N');		
	}
	else if(__day == TUE){
		LCD_write(DATA_WR, 'T');		
		LCD_write(DATA_WR, 'U');		
		LCD_write(DATA_WR, 'E');		
	}
	else if(__day == WED){
		LCD_write(DATA_WR, 'W');		
		LCD_write(DATA_WR, 'E');		
		LCD_write(DATA_WR, 'D');		
	}
	else if(__day == THU){
		LCD_write(DATA_WR, 'T');		
		LCD_write(DATA_WR, 'H');		
		LCD_write(DATA_WR, 'U');		
	}
	else if(__day == FRI){
		LCD_write(DATA_WR, 'F');		
		LCD_write(DATA_WR, 'R');		
		LCD_write(DATA_WR, 'I');		
	}
	else if(__day == SAT){
		LCD_write(DATA_WR, 'S');		
		LCD_write(DATA_WR, 'A');		
		LCD_write(DATA_WR, 'T');		
	}
}

void displayBasic(unsigned int wr)
{
	char buf[3];
	itoa(wr, buf);
	LCD_write(DATA_WR, buf[1]);		
	LCD_write(DATA_WR, buf[0]);	
}

void displayMonth(unsigned char __month)
{
	LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 3) );
	displayBasic(__month);
}

void displayDate(unsigned char __date)
{
	LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 6) );
	displayBasic(__date);
}

void displayHour(unsigned short __hour)
{
	displayBasic(__hour);
}

void displayMinute(unsigned short __minute)
{
	displayBasic(__minute);
}

void displaySecond(unsigned short __second)
{
	LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 6) );	
	displayBasic(__second);	
}

void displayMsecond(unsigned short __msecond)
{
	LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 6) );	
	displayBasic(__msecond);
}

void display(int select)
{
	if(select == TIMESTATE)		//displayLCD(state == TIMESTATE)
	{
		if(timerwatch > 3)//display all
		{
			displayDay(time.day);
			displayMonth(time.month);		
			displayDate(time.date);
			LCD_write(CTRL_WR, LCD_HOME_L2);
			displayHour(time.hour);
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
			displayMinute(time.minute);		
			timerwatch = 1;
		}else if(timerwatch > 2){	//display hour, minute, second
			LCD_write(CTRL_WR, LCD_HOME_L2);
			displayHour(time.hour);
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
			displayMinute(time.minute);		
			timerwatch = 1;
		}else if(timerwatch > 1){	//display minute, second
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
			displayMinute(time.minute);		
			timerwatch = 1;		
		}	
		displaySecond(time.second);
	}
	else if(select == MENUSTATE)	//displayLCD(state == MENUSTATE)
	{	
		LCD_write(CTRL_WR, LCD_CLEAR);
		DisplayString(LCD_LINE1,"MENU");
		//DisplayString(LCD_LINE1,"s1 s2 s3");
		DisplayString(LCD_LINE2,"<- AL ST");
	}
	else if(select == ALARMSTATE)	//displayLCD(state == ALARMSTATE)
	{	
		LCD_write(CTRL_WR, LCD_CLEAR);
		DisplayString(LCD_LINE1,"AL");
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 3) );	
		displayHour(aTime.a_hour);	
		LCD_write(DATA_WR, ':');
		displayMinute(aTime.a_minute);
		DisplayString(LCD_LINE2,"SMTWTFS");	
		if(aTime.a_day & SUN_MASK){
			LCD_write(CTRL_WR,LCD_HOME_L2);
			LCD_write(DATA_WR, '*');		
		}
		if(aTime.a_day & MON_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 1) );
			LCD_write(DATA_WR, '*');		
		}
		if(aTime.a_day & TUE_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 2) );
			LCD_write(DATA_WR, '*');		
		}
		if(aTime.a_day & WED_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );
			LCD_write(DATA_WR, '*');		
		}
		if(aTime.a_day & THU_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 4) );
			LCD_write(DATA_WR, '*');		
		}
		if(aTime.a_day & FRI_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 5) );
			LCD_write(DATA_WR, '*');
		}	
		if(aTime.a_day & SAT_MASK){
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 6) );
			LCD_write(DATA_WR, '*');
		}	
	}
	else if(select == STOPWATCHMENU){	//displayLCD(state == STOPWATCHMENU)
		LCD_write(CTRL_WR, LCD_CLEAR);
		DisplayString(LCD_LINE1,"STOPWAT");		
		DisplayString(LCD_LINE2,"<- St re");
	}
	else if(select == STOPWATCHSTATE){	//displayLCD(state == STOPWATCHSTATE)
		LCD_write(CTRL_WR, LCD_CLEAR);
		DisplayString(LCD_LINE1,"ST ");
		displayBasic(lab.count);
		LCD_write(DATA_WR, '.');
		displayMsecond(sTime.s_msecond);
		LCD_write(CTRL_WR, LCD_HOME_L2);
		displayHour(sTime.s_hour);
		LCD_write(DATA_WR, ':');
		displayMinute(sTime.s_minute);
		LCD_write(DATA_WR, ':');
		displaySecond(sTime.s_second);
	}
	else if(select == LABRECORD){	//displayLCD(state == LABRECORD)
		LCD_write(CTRL_WR, LCD_CLEAR);
		DisplayString(LCD_LINE1,"re ");
		if(lab.count == 0){
			DisplayString(LCD_LINE1,"re  NO");
			DisplayString(LCD_LINE2,"<- DATA");
		}
		else{
			if(state == LABRECORD)
				displayBasic(lab.forRead +1);
			else displayBasic(lab.forRead);
			LCD_write(DATA_WR, '.');
			displayMsecond(lab.sTime[lab.forRead].s_msecond);
			LCD_write(CTRL_WR, LCD_HOME_L2);
			displayHour(lab.sTime[lab.forRead].s_hour);
			LCD_write(DATA_WR, ':');
			displayMinute(lab.sTime[lab.forRead].s_minute);
			LCD_write(DATA_WR, ':');
			displaySecond(lab.sTime[lab.forRead].s_second);	
		}			
	}
	//...
}

/*about ISR*/
void timer1IntHndl(void)
{	
	calTime();	//increase time
	
	if(light > 0){	//light function
		if(--light <= 0)	//for 3~4 second light ON
		{
			YLW_LED = 1;
			GRN_LED = 1;
			RED_LED = 1;
			light = 0;
		}		
	}
	if(alarm_light > 0){	//alarm function
		if(--alarm_light > 0)	//for 5~6second light on/off trig
		{
			if(alarm_light%2 == 1){
				YLW_LED = 1;
				GRN_LED = 1;
				RED_LED = 1;
			}
			else{
				YLW_LED = 0;
				GRN_LED = 0;
				RED_LED = 0;
			}
		}
		else alarm_light = 0;
	}
}

void timer2IntHndl(void)
{
//No
}

void timer0IntHndl(void)
{
	calStopWatch();	//stopwatch time increase
}

void Switch1IntHndl(void)
{
	bool_int1 =	1;
}

void Switch2IntHndl(void)
{
	bool_int2 = 1;
}


/*about polling*/
void pollingTimer()
{
	if(state == TIMESTATE)	//time mode display
		display(TIMESTATE);
	
	else if(state == STOPWATCHSTATE)	//stopwatch mode display
		if(stopwatch_state == STOPWATCHRUN)
		{
			if(stopwatch == 0){		//0~1second
				displayMsecond(sTime.s_msecond);	
			}else if(stopwatch == 1){	//per second
				displaySecond(sTime.s_second);
				displayMsecond(sTime.s_msecond);
				stopwatch = 0;
			}else if(stopwatch == 2){	//per minute
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
				displayMinute(sTime.s_minute);		
				displaySecond(sTime.s_second);
				displayMsecond(sTime.s_msecond);	
				stopwatch = 0;
			}else if(stopwatch == 3){	//per hour
				LCD_write(CTRL_WR, LCD_HOME_L2);
				displayHour(sTime.s_hour);	
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
				displayMinute(sTime.s_minute);		
				displaySecond(sTime.s_second);
				displayMsecond(sTime.s_msecond);	
				stopwatch = 0;
			}
		}
}

void pollingSwitch()	//switch event handling
{
	int i = 0;
	
	if(S3 == 0)	//S3 event check
	{
		bool_S3 = 1;
	}
	
	//-------------------------S1
	if(bool_int1 > 0)
	{
		if(state == TIMESTATE)
		{
			YLW_LED = 0;
			GRN_LED = 0;
			RED_LED = 0;
			light = 3;	//Light ON
		}
		else if(state == MODIFYSTATE)
		{	//modify time
			DISABLE_IRQ;	//Sync
			time.second = mTime.second;
			time.minute = mTime.minute;
			time.hour = mTime.hour;
			time.day = mTime.day;
			time.date = mTime.date;
			time.month = mTime.month;
			ENABLE_IRQ;	//Sync
			
			tOffset = 0;
			LCD_write(CTRL_WR, LCD_CURSOR_OFF);
			state = TIMESTATE;	//MODIFYSTATE -> TIMESTATE
			myInitDisplay(1);
			myInitDisplay(2);		
			//display(TIMESTAE);
		} 
		else if(state == MENUSTATE)
		{
			state = TIMESTATE;	//MENUSTATE -> TIMESTATE
			myInitDisplay(1);
			myInitDisplay(2);		
			//display(TIMESTATE);	
		}
		else if(state == ALARMSTATE)	//ALARMSTATE -> MENUSTATE
		{
			state = MENUSTATE;	
			display(MENUSTATE);			
			tOffset = 0;
			LCD_write(CTRL_WR, LCD_CURSOR_OFF);

		}
		else if(state == STOPWATCHMENU)	
		{
			state = MENUSTATE;	//STOPWATCHMENU -> MENUSTATE		
			display(MENUSTATE);	
		}
		else if(state == STOPWATCHSTATE)
		{
			state = STOPWATCHMENU;	//STOPWATCHSTATE -> STOPWATCHMENU
			display(STOPWATCHMENU);	
		}
		else if(state == LABRECORD)
		{
			state = STOPWATCHMENU;	//LABRECORD -> STOPWATCHMENU
			lab.forRead = 0;
			display(STOPWATCHMENU);			
		}
		DISABLE_IRQ;
		bool_int1 = 0;	//TODO Sync
		ENABLE_IRQ;
	}	
	
	if(S3 == 0)	//priority up as soft
	{
		bool_S3 = 1;
	}
	
	//-------------------------S2
	if(bool_int2 > 0)
	{	//TO MODIFYSTATE
		if(state == TIMESTATE)	
		{
			state = MODIFYSTATE;
			//mov time var -> modify var
			mTime.second = time.second;
			mTime.minute = time.minute;
			mTime.hour = time.hour;
			mTime.day = time.day;
			mTime.date = time.date;
			mTime.month = time.month;
			
			tOffset = DAY;
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 2) );	
			LCD_write(CTRL_WR, LCD_CURSOR_BLINK);
		}
		//modify time
		else if(state == MODIFYSTATE)
		{
			//day
			if(tOffset == DAY)
			{
				incDay(&mTime.day);
				displayDay(mTime.day);
			}else if(tOffset == MONTH)	//month
			{
				incMonth(&mTime.month);
				displayMonth(mTime.month);	
			}else if(tOffset == DATE){	//date
				incDate(&mTime.date);
				displayDate(mTime.date);
			}else if(tOffset == HOUR){	//hour
				incHour(&mTime.hour);
				LCD_write(CTRL_WR, LCD_HOME_L2);		
				displayHour(mTime.hour);
			}else if(tOffset == MINUTE1){	//minute ten's place
				incMinAndSec(&mTime.minute, 1);
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
				displayMinute(mTime.minute);
			}else if(tOffset == MINUTE2){	//minute unit's place
				incMinAndSec(&mTime.minute, 2);
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
				displayMinute(mTime.minute);	
			}else if(tOffset == SECOND1){	//second ten's place	
				incMinAndSec(&mTime.second, 1);
				displaySecond(mTime.second);
			}else if(tOffset == SECOND2){	//second unit's place
				incMinAndSec(&mTime.second, 2);
				displaySecond(mTime.second);
			}
			modify_cursor();
		}	
		else if(state == MENUSTATE)
		{
			state = ALARMSTATE;	//MENUSTATE -> ALARMSTATE
			display(ALARMSTATE);
			LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 4) );	
			LCD_write(CTRL_WR, LCD_CURSOR_BLINK);	
		}
		else if(state == ALARMSTATE)	
		{
			//alarm
			if(tOffset == 0){	//hour modify and display
				incHour(&aTime.a_hour);
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 3) );	
				displayHour(aTime.a_hour);
			}
			else if(tOffset == 1){	//minute ten's place modify and display
				incMinAndSec(&aTime.a_minute, 1);
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 6) );	
				displayMinute(aTime.a_minute);
			}
			else if(tOffset == 2){ 	//minute unit's place modify and display
				incMinAndSec(&aTime.a_minute, 2);
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 6) );	
				displayMinute(aTime.a_minute);
			}		
			else if(tOffset == 3){	//sunday setting
				aTime.a_day = aTime.a_day ^ SUN;
				display(ALARMSTATE);
			}
			else if(tOffset == 4){	//monday setting
				aTime.a_day = aTime.a_day ^ MON;
				display(ALARMSTATE);
			}				
			else if(tOffset == 5){	//tuesday setting
				aTime.a_day = aTime.a_day ^ TUE;
				display(ALARMSTATE);
			}
			else if(tOffset == 6){	//wednesday setting
				aTime.a_day = aTime.a_day ^ WED;
				display(ALARMSTATE);
			}
			else if(tOffset == 7){	//thursday setting
				aTime.a_day = aTime.a_day ^ THU;
				display(ALARMSTATE);
			}
			else if(tOffset == 8){	//friday setting
				aTime.a_day = aTime.a_day ^ FRI;
				display(ALARMSTATE);
			}
			else if(tOffset == 9){	//saturday setting
				aTime.a_day = aTime.a_day ^ SAT;
				display(ALARMSTATE);
			}	
			alarm_cursor();	
		}
		else if(state == STOPWATCHMENU)
		{
			state = STOPWATCHSTATE;	//STOPWATCHMENU -> STOPWATCHSTATE
			display(STOPWATCHSTATE);
		}
		else if(state == STOPWATCHSTATE)
		{
			if(stopwatch_state == STOPWATCHSTOP){
				ta0s = 1;	//stopwatch run 
				stopwatch_state = STOPWATCHRUN;	
			}
			else if(stopwatch_state == STOPWATCHRUN){
				ta0s = 0;	//stopwatch stop
				stopwatch_state = STOPWATCHSTOP;
			}
		}
		else if(state == LABRECORD)
		{	//prev lab record data display			
			if(lab.count > 0 && lab.forRead > 0){
				lab.forRead --;//prev
				display(LABRECORD);
			}
		}
		//...
		DISABLE_IRQ;	
		bool_int2 = 0;
		ENABLE_IRQ;
	}
	
	//-------------------------S3
	if(S3 == 0)
	{
		bool_S3 = 1;
	}
	else if(bool_S3 == 1){	//falling edge as soft
		if(state == TIMESTATE){
			display(MENUSTATE);
			state = MENUSTATE;	//TIMESTATE -> MENUSTATE
		}
		else if(state == MODIFYSTATE)
		{
			if(tOffset < SECOND2)
			{
				tOffset++;
				modify_cursor(); //move cursor to next time modify target
			}
		}
		
		else if(state == MENUSTATE)
		{
			state = STOPWATCHMENU;	//MENUSTATE -> STOPWATCHMENU
			display(STOPWATCHMENU);
		}
		
		else if(state == ALARMSTATE)
		{
			if(tOffset < 9)
			{
				tOffset++;
				alarm_cursor();	//move cursor to next alarm modify target
			}
		}
		else if(state == STOPWATCHMENU)	///->recond
		{
			state = LABRECORD;	//STOPWATCHMENU -> LABRECORD
			display(LABRECORD);		
		}
		else if(state == STOPWATCHSTATE)
		{
			if(stopwatch_state == STOPWATCHSTOP){
				//do not need to Sync already ta0s = 0; 
				//when stopwatch state is stop, init stopwatch
				sTime.s_msecond = 0;
				sTime.s_second = 0;
				sTime.s_minute = 0;
				sTime.s_hour = 0;
				lab.count = 0;	//init lab
				display(STOPWATCHSTATE);
			}
			else if(stopwatch_state == STOPWATCHRUN){
				if(lab.count < 10)
				{
					//during stopwatch is running, lab
					lab.sTime[lab.count].s_msecond = sTime.s_msecond;
					lab.sTime[lab.count].s_second = sTime.s_second;
					lab.sTime[lab.count].s_minute = sTime.s_minute;
					lab.sTime[lab.count].s_hour = sTime.s_hour;
					lab.count ++;	
					LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 3) );	
					displayBasic(lab.count);
				}	
			}
		}
		else if(state == LABRECORD)
		{
			//view next labrecord data
			if(lab.count > 0 && lab.forRead < 9 && lab.forRead < lab.count-1) {
				lab.forRead ++;
				display(LABRECORD);
			}
		}
		//...
					
		bool_S3 = 0;
	}
}


/*about Increase time variables*/
void calStopWatch()	//stopwatch increase
{
	sTime.s_msecond ++;
	if(sTime.s_msecond >=100){
		sTime.s_msecond = 0;
		sTime.s_second++;
		stopwatch = 1;		//to 
		if(sTime.s_second >= 60)
		{
			sTime.s_second = 0;
			sTime.s_minute++;
			stopwatch = 2;
			if(sTime.s_minute >= 60)	
			{
				sTime.s_minute = 0;	
				sTime.s_hour ++;
				stopwatch = 3;
				if(sTime.s_hour>23){
					sTime.s_hour = 0;
				}
			}
		}
	}
}

void calTime()
{
	time.second++;
	if(time.second >= 60)
	{
		time.second = 0;
		time.minute++;
		timerwatch = 2;
		if(time.minute >= 60)	
		{
			time.minute = 0;	
			time.hour++;
			timerwatch = 3;
			if(time.hour>23)
			{
				time.hour = 0;
				time.date++;
				timerwatch = 4;
				if(time.date > 31)
				{
					time.date = 1;
					time.month++;
					if(time.month > 12)
					{
						time.month = 1;
						//year++;
					}	
				}
				time.day = time.day>>1;
				if(time.day == 0x00)
				{
					time.day = SUN;	
				}
			}
		}
		if(aTime.a_day & time.day)///alarm !!!
		{
			if(time.hour == aTime.a_hour && time.minute == aTime.a_minute)
			{
				alarm_light = 7;//for 6second light on/off trig
			}	
		}
	}
}

void incHour(unsigned short *__hour)
{
	(*__hour)++;
	if(*__hour > 23)
	{
		*__hour = 0;			
	}
}
void incMonth(unsigned char *__month)
{
	(*__month)++;
	if(*__month > 12)
	{
		*__month = 1;
	}	
}
 
void incDay(unsigned char *__day)
{
	(*__day) = (*__day)>>1;
	if(*__day == 0x00)
	{
		*__day = SUN;		
	}
}

void incDate(unsigned char *__date)
{
	(*__date)++;
	if(*__date > 31)
	{
		*__date = 1;		
	}
}

void incMinAndSec(unsigned short *__minute, int s)
{
	//modify minute ten's place
	if(s == 1){
		*__minute = *__minute + 10;
		if(*__minute >= 60){
			*__minute = *__minute %10;
		}	 
	}
	//modify minute unit's place
	else if(s == 2){
		(*__minute)++;
		if(*__minute %10 == 0){
			*__minute = *__minute - 10;			
		}
	}
}

/*about cursor operation*/
void modify_cursor()
{
	if(tOffset == DAY){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 2) );	
	}else if(tOffset == MONTH){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 4) );	
	}else if(tOffset == DATE){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 7) );	
	}else if(tOffset == HOUR){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 1) );	
	}else if(tOffset == MINUTE1){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );	
	}else if(tOffset == MINUTE2){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 4) );	
	}else if(tOffset == SECOND1){		
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 6) );	
	}else if(tOffset == SECOND2){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 7) );	
	}
}

void alarm_cursor()
{	
	if(tOffset == 0){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 4) );	
	}
	else if(tOffset == 1){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 6) );	
	}
	else if(tOffset == 2){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 7) );
	}
	else if(tOffset == 3){
		LCD_write(CTRL_WR, LCD_HOME_L2);
	}
	else if(tOffset == 4){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 1) );
	}
	else if(tOffset == 5){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 2) );	
	}
	else if(tOffset == 6){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 3) );		
	}
	else if(tOffset == 7){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 4) );		
	}
	else if(tOffset == 8){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 5) );		
	}
	else if(tOffset == 9){
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + 6) );		
	}			
}

/*itoa*/
void itoa(unsigned short num, char *buf)
{
	unsigned short num_a = num%10;
	unsigned short num_b = num/10;
	char a = num_a +'0';
	char b = num_b +'0';

	buf[0] = a;	
	buf[1] = b;
	buf[2] = 0;
}

/*not to need this form
void itoaForYear(unsigned short num, char *buf)
{
	unsigned _num = num;
	unsigned short num_d;
	unsigned short num_c;
	unsigned short num_b;
	unsigned short num_a;
	
	char a, b, c, d;

	num_d = _num/1000;
	_num -= num_d*1000;
	num_c = _num/100;
	_num -= num_c*100;
	num_b = _num/10;
	num_b = _num%10;

	a = num_a +'0';
	b = num_b +'0';	
	c = num_c +'0';	
	d = num_d +'0';	
	
	buf[0] = a;
	buf[1] = b;
	buf[2] = c;
	buf[3] = d;
	buf[4] = 0;
}
*/
