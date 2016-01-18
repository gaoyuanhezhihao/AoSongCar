/*
 Name:		AoSong.ino
 Created:	2016/1/17 9:50:35
 Author:	Anna Sherlock
*/

// the setup function runs once when you press reset or power the board
#include <PWM.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#define TOTAL_BYTES 4
#define EA 12
#define EB 11
#define I1 22
#define I2 23
#define I3 24
#define I4 25

char first = 1;
const char HEADER = 'H';
short int last_order_second = 0;
unsigned int pwm_A = 300;
unsigned int pwm_B = 300;

void setup() {
	Serial.begin(9600);
	Serial1.begin(9600);
	InitTimersSafe(); //initialize all timers except for 0, to save time keeping functions
	setTime(0, 0, 0, 1, 17, 2016);
	Alarm.timerRepeat(1, Guard);
	pinMode(EA, OUTPUT);
	pinMode(EB, OUTPUT);
	pinMode(I1, OUTPUT);
	pinMode(I2, OUTPUT);
	pinMode(I3, OUTPUT);
	pinMode(I4, OUTPUT);
	//settingHighResolutionDuty();
}
void Guard()
{
	++last_order_second;
	if (last_order_second >= 2)
	{
		digitalWrite(I1, 0);
		digitalWrite(I2, 0);
		digitalWrite(I3, 0);
		digitalWrite(I4, 0);
	}

}
void process_msg(char rcv_ch[3])
{
	unsigned int pwm = 0;
	static char legal_order[] = { 's', 'f', 'b', 'l', 'r', 'A', 'B' ,'\0' };
	char *p_legal_order = legal_order;
	while (*p_legal_order != '\0')
	{
		if (*p_legal_order == rcv_ch[0])
		{
			last_order_second = 0;
			break;
		}
		++ p_legal_order;
		
	}
	Serial1.println("msg:");
	switch (rcv_ch[0])
	{
	case 'A':
		//Serial1.println("waiting");
		pwm_A = rcv_ch[1] * 256;
		pwm_A += (unsigned char)rcv_ch[2];
		Serial1.print("rcv:pwm A=");
		Serial1.print(pwm_A);
		analogWrite(EA, pwm_A);
		Serial1.println("ok");
		//Serial1.println("pwm changed");
		break;
	case 'B':
		//Serial1.println("waiting");
		pwm_B = rcv_ch[1] * 256;
		pwm_B += (unsigned char)rcv_ch[2];
		Serial1.print("rcv:pwm B=");
		Serial1.print(pwm_B);
		analogWrite(EB, pwm_B);
		Serial1.println("ok");
		Serial1.println("pwm changed");
		break;
	case 's':
		Serial1.println("ok");
		Serial1.println("try to stop");
		digitalWrite(I1, 0);
		digitalWrite(I2, 0);
		digitalWrite(I3, 0);
		digitalWrite(I4, 0);
		break;
	case 'f':
		Serial1.println("ok");
		Serial1.println("try to go forward");
		digitalWrite(I1, 1);
		digitalWrite(I2, 0);
		digitalWrite(I3, 1);
		digitalWrite(I4, 0);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
		break;
	case 'l':
		Serial1.println("ok");
		Serial1.println("try to turn left");
		digitalWrite(I1, 1);
		digitalWrite(I2, 0);
		digitalWrite(I3, 0);
		digitalWrite(I4, 1);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
		break;
	case 'r':
		Serial1.println("ok");
		Serial1.println("try to turn right");
		digitalWrite(I1, 0);
		digitalWrite(I2, 1);
		digitalWrite(I3, 1);
		digitalWrite(I4, 0);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
		break;
	case 'b':
		Serial1.println("ok");
		Serial1.println("try to run back");
		digitalWrite(I1, 0);
		digitalWrite(I2, 1);
		digitalWrite(I3, 0);
		digitalWrite(I4, 1);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
		break;
	default:
		break;
	}
}
// the loop function runs over and over again until power down or reset
void loop() 
{
	char rcv_ch[3] = { 0 };
	if (Serial1.available() >= TOTAL_BYTES)
	{
		//Serial1.println("available");
		char tag = Serial1.read();
		//Serial1.print(tag);
		if (tag == HEADER)
		{
			rcv_ch[0] = Serial1.read();
			rcv_ch[1] = Serial1.read();
			rcv_ch[2] = Serial1.read();
			process_msg(rcv_ch);
		}
	}
	Alarm.delay(0);
}
