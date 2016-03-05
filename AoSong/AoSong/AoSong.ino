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
#include <math.h>
#define TOTAL_BYTES 4
#define EA 12
#define EB 11
#define I1 22
#define I2 23
#define I3 24
#define I4 25
#define ECHO 7
#define TRIG 8
char first = 1;
const char HEADER = 'H';
short int last_order_second = 0;
unsigned int pwm_A = 5000;
unsigned int pwm_B = 5000;
// MPU6050
unsigned char Re_buf[11], counter = 0;
unsigned char sign = 0;
unsigned char angle_buf = 0;
float a[3], w[3], angle[3], T;

//Obstacle Detector
bool forward_state = false;
#define MIN_SAFE_DIST 10*58  //10 cm


void setup() {
	Serial.begin(9600);
	Serial1.begin(9600);
	InitTimersSafe(); //initialize all timers except for 0, to save time keeping functions
	InitSensor();
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
	static char legal_order[] = { 's', 'f', 'b', 'l', 'r', 'A', 'B', 'k','\0' };//'k' = "keep alive"
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
		Serial1.println("A_ok");
		//Serial1.println("pwm changed");
		break;
	case 'B':
		//Serial1.println("waiting");
		pwm_B = rcv_ch[1] * 256;
		pwm_B += (unsigned char)rcv_ch[2];
		Serial1.print("rcv:pwm B=");
		Serial1.print(pwm_B);
		analogWrite(EB, pwm_B);
		Serial1.println("B_ok");
		Serial1.println("pwm changed");
		break;
	case 's':
		Serial1.println("s_ok");
		Serial1.println("try to stop");
		digitalWrite(I1, 0);
		digitalWrite(I2, 0);
		digitalWrite(I3, 0);
		digitalWrite(I4, 0);
		break;
	case 'f':
		Serial1.println("f_ok");
		Serial1.println("try to go forward");
		if (DetectObstacle())
		{
			digitalWrite(I1, 0);
			digitalWrite(I2, 0);
			digitalWrite(I3, 0);
			digitalWrite(I4, 0);
			Serial1.println("WARNING_O");
			Serial1.println("Obstacle in front, Car stoped.");
		}
		else
		{
			Serial1.println("car forwarding");
			forward_state = true;
			digitalWrite(I1, 1);
			digitalWrite(I2, 0);
			digitalWrite(I3, 1);
			digitalWrite(I4, 0);
			analogWrite(EA, pwm_A);
			analogWrite(EB, pwm_B);
		}
		break;
	case 'l':
		Serial1.println("l_ok");
		Serial1.println("try to turn left");
		turn('l', (unsigned int)rcv_ch[1]);
		break;
	case 'r':
		Serial1.println("r_ok");
		Serial1.println("try to turn right");
		turn('r', (unsigned int)rcv_ch[1]);
		break;
	case 'b':
		Serial1.println("b_ok");
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
	CheckFrontSafety();
	Alarm.delay(0);
}

void InitSensor() 
{
	Serial2.begin(9600);

	pinMode(ECHO, INPUT);
	pinMode(TRIG, OUTPUT);
}
void getAngle()
{
	counter = 0;
	while (1)
	{
		if (Serial2.available())
		{
			//char inChar = (char)Serial.read(); Serial.print(inChar); //Output Original Data, use this code 

			Re_buf[counter] = (unsigned char)Serial2.read();
			++counter;
			if (Re_buf[0] != 0x55)
			{
				counter = 0;
				continue;
			}
			if (counter >= 2 && Re_buf[1] != 0x53)
			{
				counter = 0;
				continue;
			}
			if (counter == 11)
			{
				break;
			}
		}
	}

	if (Re_buf[0] == 0x55 && Re_buf[1] == 0x53)      
	{
		angle[0] = (short(Re_buf[3] << 8 | Re_buf[2])) / 32768.0 * 180;
		angle[1] = (short(Re_buf[5] << 8 | Re_buf[4])) / 32768.0 * 180;
		angle[2] = (short(Re_buf[7] << 8 | Re_buf[6])) / 32768.0 * 180;
		angle_buf = 1;
	}
}
bool turn(char flag, int turnAngle)
{
	float currentAngle = 0.0;
	//Serial.print("currentAngle = ");//
	//Serial.println(currentAngle);//
	char symbol;
	float expectAngle;
	currentAngle = GetInitAngle();
	if (currentAngle > 0) symbol = '+';
	else symbol = '-';

	if (flag == 'l')
	{
		expectAngle = currentAngle + turnAngle;
		StartTurning(flag);
		while (currentAngle < expectAngle)
		{
			getAngle();
			last_order_second = 0;
			if (symbol == '+'&&angle[2] < 0) currentAngle = angle[2] + 360;
			else currentAngle = angle[2];
			Serial.print("current angle:");
			Serial.print(currentAngle);
			Serial.print("expectAngle:");
			Serial.println(expectAngle);
		}
		Serial.println("turning completed");
		StopCar();
		return true;
	}
	else if (flag == 'r')
	{
		expectAngle = currentAngle - turnAngle;
		StartTurning(flag);
		while (currentAngle>expectAngle)
		{
			getAngle();
			last_order_second = 0;
			if (symbol == '-' && angle[2] > 0) currentAngle = angle[2] - 360;
			else currentAngle = angle[2];
			Serial.print("current angle:");
			Serial.print(currentAngle);
			Serial.print("expectAngle:");
			Serial.println(expectAngle);
		}
		Serial.println("turning completed");
		StopCar();
		return true;
	}
	else return false;
} 
float GetInitAngle()
{

	float angle0 = 0;
	float angle1 = 0;
	float angle2 = 0;
	float ave_angle = 0;
	while (1)
	{
		getAngle();
		angle0 = angle[2];
		getAngle();
		angle1 = angle[2];
		getAngle();
		angle2 = angle[2];
		ave_angle = (angle0 + angle1 + angle2) / 3;
		if (abs(ave_angle - angle0) < 1 && abs(ave_angle - angle1) < 1 && abs(ave_angle - angle2) < 1)
		{
			return ave_angle;
		}
		Serial.println("get bad init angle");
	}
}
void StopCar()
{
	digitalWrite(I1, 0);
	digitalWrite(I2, 0);
	digitalWrite(I3, 0);
	digitalWrite(I4, 0);
}
void StartTurning(char flag)
{
	if (flag == 'l')
	{
		Serial.println("Start Turning Left");
		Serial.print("pwm_A");
		Serial.print(pwm_A);
		Serial.print(", pwm_B");
		Serial.println(pwm_B);
		digitalWrite(I1, 1);
		digitalWrite(I2, 0);
		digitalWrite(I3, 0);
		digitalWrite(I4, 1);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
	}
	else if (flag == 'r')
	{
		digitalWrite(I1, 0);
		digitalWrite(I2, 1);
		digitalWrite(I3, 1);
		digitalWrite(I4, 0);
		analogWrite(EA, pwm_A);
		analogWrite(EB, pwm_B);
	}
}

bool CheckFrontSafety()
{
	if (forward_state)
	{
		
		if (DetectObstacle())
		{
			digitalWrite(I1, 0);
			digitalWrite(I2, 0);
			digitalWrite(I3, 0);
			digitalWrite(I4, 0);
			Serial1.println("WARNING_O");
			Serial1.println("Obstacle in front, Car stoped.");
			forward_state = false;
		}
	}

}

bool DetectObstacle()
{
	int distance;
	digitalWrite(TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG, LOW);
	distance = pulseIn(ECHO, HIGH, 1000);
	if (distance == 0 || distance > MIN_SAFE_DIST)
	{
		return false;
	}
	else
	{
		return true;
	}
}