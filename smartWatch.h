#pragma INTERRUPT timer1IntHndl;
#pragma INTERRUPT timer2IntHndl;
#pragma INTERRUPT timer0IntHndl;
#pragma INTERRUPT Switch1IntHndl;
#pragma INTERRUPT Switch2IntHndl;

#define I_MASK (0x0040)	
#define GET_INT_STATUS(x){\
			_asm("STC FLG, $$[FB]", x);\
			x &= I_MASK;\
		}		

#define SUN 0x40
#define MON 0x20
#define TUE 0x10
#define WED 0x08
#define THU 0x04
#define FRI 0x02
#define SAT 0x01

#define SUN_MASK 0x40
#define MON_MASK 0x20
#define TUE_MASK 0x10
#define WED_MASK 0x08
#define THU_MASK 0x04
#define FRI_MASK 0x02
#define SAT_MASK 0x01

#define TIMESTATE		1
#define MODIFYSTATE		2
#define ALARMSTATE		3
#define STOPWATCHMENU	4
#define MENUSTATE		5
#define STOPWATCHSTATE	6
#define LABRECORD		7

#define STOPWATCHRUN 	1
#define STOPWATCHSTOP 	2

enum time_type {
	DAY, MONTH, DATE, HOUR, MINUTE1, MINUTE2, SECOND1, SECOND2
};

typedef struct timeType{
	unsigned char day;
	unsigned short second;
	unsigned short minute;
	unsigned short hour;
	unsigned char month;
	unsigned char date;	
}TIME;

typedef struct sTimeType{	
	unsigned short s_msecond;
	unsigned short s_second;
	unsigned short s_minute;
	unsigned short s_hour;
}STIME;

typedef struct aTimeType{	
	unsigned short a_hour;
	unsigned short a_minute;
	unsigned char a_day;
}ATIME;

typedef struct lapRecordType{	
	unsigned char count;
	unsigned char forRead;
	STIME sTime[10];
}LAP;


/*
enum day_type {
	SUN, MON, TUE, WED, THU, FRI, SAT	
};
*/

void InitSmartWatch();
void InitVar();
void InitLED();
void InitSwitch();
void InitTimer();
void myInitDisplay(int select);
void displayDay(unsigned char __day);
void displayBasic(unsigned int wr);
void displayMonth(unsigned char __month);
void displayDate(unsigned char __date);
void displayHour(unsigned short __hour);
void displayMinute(unsigned short __minute);
void displaySecond(unsigned short __second);
void displayMsecond(unsigned short __msecond);
void display(int select);
void timer1IntHndl(void);
void timer2IntHndl(void);
void timer0IntHndl(void);
void Switch1IntHndl(void);
void Switch2IntHndl(void);
void calStopWatch();
void pollingTimer();
void pollingSwitch();
void calTime();
void incHour(unsigned short *__hour);
void incMonth(unsigned char *__month);
void incDay(unsigned char *__day);
void incDate(unsigned char *__date);
void incMinAndSec(unsigned short *__minute, int s);
void modify_cursor();
void alarm_cursor();
void itoa(unsigned short num, char *buf);

//void itoaForYear(unsigned short num, char *buf);
