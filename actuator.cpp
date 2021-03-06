#include <wiringPi.h>
#include <softPwm.h>
#include <stdlib.h>
#include "actuator.h"
#include "constants.h"
#include "utils.h"

//////////////////////////////////////////////
// Buzzer
//////////////////////////////////////////////

bool Buzzer::onInit(const struct timespec& time)
{
	pinMode(mPin, OUTPUT);
	digitalWrite(mPin, LOW);
	return true;
}
void Buzzer::onClean()
{
	digitalWrite(mPin, LOW);
}
bool Buzzer::onCommand(const std::vector<std::string>& args)
{
	int period, on_period, off_period, count;
	switch(args.size())
	{
		case 2:
			if(args[1].compare("stop") == 0)		//buzzer stop
			{
				if(mOnPeriod == 0 && mOffPeriod == 0 && mCount == 0)
				{
					Debug::print(LOG_PRINT,"Buzzer is already stopping\r\n");
				}
				else
				{
					Debug::print(LOG_PRINT,"Stop Command Executed!\r\n");
					mOffPeriod = 0;
					mCount = 1;
					stop();
				}
				return true;
			}
			else									//buzzer [period]
			{
				Debug::print(LOG_PRINT,"Start Command Executed!\r\n");
				period = atoi(args[1].c_str());
				start(period);
				return true;	
			}
			break;
			
		case 3:										//buzzer [period] [count]
			Debug::print(LOG_PRINT,"Start Command Executed!\r\n");
			period = atoi(args[1].c_str());
			count  = atoi(args[2].c_str());
			start(period, count);
			return true;
			break;
			
		case 4:									//buzzer [on oeriod] [off period] [count]
			Debug::print(LOG_PRINT,"Start Command Executed!\r\n");
			on_period  = atoi(args[1].c_str());
			off_period = atoi(args[2].c_str());
			count  	   = atoi(args[3].c_str());
			start(on_period, off_period, count);
			return true;
			break;
		default:
			break;
	}
	
	Debug::print(LOG_PRINT,"buzzer [period]                         : wake buzzer while period\r\n\
buzzer [period] [count]                 : wake buzzer several times (COUNT)\r\n\
buzzer [on period] [off period] [count] : wake buzzer several times (COUNT)\r\n\
buzzer stop                             : stop buzzer\r\n");
	return true;
}
void Buzzer::onUpdate(const struct timespec& time)
{
	if(mOffPeriod == 1)		//鳴らさない時間の終了
	{
		restart();
	}
	else if(mOffPeriod > 0)	//鳴らさない時間
	{
		--mOffPeriod;
		return;
	}

	if(mOnPeriod == 1)		//鳴らす時間の終了
	{
		stop();
	}
	else if(mOnPeriod > 0)	//鳴らす時間
	{
		--mOnPeriod;
	}
}
void Buzzer::start(int period)
{
	start(period, 1, 1);
}
void Buzzer::start(int on_period, int count)
{
	start(on_period, DEFAULT_OFF_PERIOD, count);
}
void Buzzer::start(int on_period, int off_period, int count)
{
	if(mOnPeriod == 0 && on_period >= 1 && off_period >= 1 && count >= 1)
	{
		Debug::print(LOG_DETAIL,"Buzzer Start!\r\n");
		mOnPeriodMemory  = on_period;
		mOnPeriod		 = on_period;
		mOffPeriodMemory = off_period;
		mOffPeriod		 = 0;
		mCount			 = count;
		digitalWrite(mPin, HIGH);
	}
}
void Buzzer::restart()
{
	digitalWrite(mPin, HIGH);
	mOnPeriod = mOnPeriodMemory;
	mOffPeriod = 0;
}
void Buzzer::stop()
{
	mOnPeriod = 0;
	digitalWrite(mPin, LOW);

	if(mCount == 1)
	{
		Debug::print(LOG_DETAIL,"Buzzer Stop!\r\n");
		mCount = 0;
	}
	else if(mCount > 0)
	{
		mOffPeriod = mOffPeriodMemory;
		--mCount;
	}
}
Buzzer::Buzzer() : mPin(PIN_BUZZER),mOnPeriodMemory(0),mOnPeriod(0),mOffPeriodMemory(0),mOffPeriod(0),mCount(0)
{
	setName("buzzer");
	setPriority(TASK_PRIORITY_ACTUATOR,TASK_INTERVAL_ACTUATOR);
}
Buzzer::~Buzzer()
{
}

//////////////////////////////////////////////
// ParaServo(Software PWM)
//////////////////////////////////////////////

bool ParaServo::onInit(const struct timespec& time)
{
	if (wiringPiSetup() == -1)//Software PWMを使う前にwiringPiSetupを呼ぶ必要があるらしい
	{
		Debug::print(LOG_PRINT,"ParaServoError: wiringPi setup failed...\n");
	}

	softPwmCreate(mPin, 0, SERVO_RANGE);	//int softPwmCreate (int pin, int initialValue, int pwmRange);
	return true;
}
void ParaServo::onClean()
{
	stop();
}
bool ParaServo::onCommand(const std::vector<std::string>& args)
{
	if(args.size() >= 2)
	{
		if(args[1].compare("stop") == 0)
		{
			stop();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
		else
		{
			//角度指定
			int period = 0;
			if(args.size() == 2)
			{
				period = atof(args[1].c_str());
			}
			start(period);
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
	}
	else
	{
		Debug::print(LOG_PRINT,"paraservo [0 or %d-%d]\t: set servo position\r\n\
paraservo stop\t\t: stop servo\r\n", SERVO_MIN_RANGE ,SERVO_MAX_RANGE);
	}
	return true;
}
void ParaServo::start(int angle)
{
	//範囲のチェック
	if(angle >= SERVO_MAX_RANGE)
	{
		angle = SERVO_MAX_RANGE;
	}
	else if(angle <= 0)
	{
		angle = 0;
	}
	else if(angle < SERVO_MIN_RANGE)
	{
		angle = SERVO_MIN_RANGE;
	}

	softPwmWrite(mPin, angle);			//void softPwmWrite (int pin, int value);
	Debug::print(LOG_PRINT,"ParaServo Start (%d)!\r\n",angle);
}
void ParaServo::start(POSITION p)
{
	start((int)p);
}
void ParaServo::stop()
{
	softPwmWrite (mPin, 0);
}
void ParaServo::moveRelease()
{
	start(POSITION_RELEASE);
}
void ParaServo::moveHold()
{
	start(POSITION_HOLD);
}
ParaServo::ParaServo() : mPin(PIN_PARA_SERVO)
{
	setName("paraservo");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
ParaServo::~ParaServo()
{
}

//////////////////////////////////////////////
// StabiServo
//////////////////////////////////////////////

bool StabiServo::onInit(const struct timespec& time)
{
	pinMode(mPin, PWM_OUTPUT);

	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(9000);
	pwmSetClock(32);

	return true;
}
void StabiServo::onClean()
{
	stop();
}
bool StabiServo::onCommand(const std::vector<std::string>& args)
{
	if(args.size() >= 2)
	{
		if(args[1].compare("stop") == 0)
		{
			stop();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}else if(args[1].compare("close") == 0)
		{
			close();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}else
		{
			//角度指定
			float period = 0;
			if(args.size() == 2)
			{
				period = atof(args[1].c_str());
			}
			start(period);
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
	}else
	{
		Debug::print(LOG_PRINT,"stabiservo [0-1]          : set servo angle\r\n\
stabiservo stop           : stop servo\r\n");
	}
	return true;
}
void StabiServo::start(double angle)
{
	if(angle > 0.88)angle = 0.88;
	else if(angle < 0)angle = 0;

	pwmWrite (mPin, SERVO_BASE_VALUE + angle * SERVO_MOVABLE_RANGE);
	Debug::print(LOG_DETAIL,"StabiServo Start (%f)!\r\n",angle);
}
void StabiServo::stop()
{
	pwmWrite (mPin, 0);
	Debug::print(LOG_DETAIL,"StabiServo Stop!\r\n");
}
void StabiServo::close()
{
	pwmWrite(mPin, SERVO_BASE_VALUE);
	Debug::print(LOG_DETAIL,"StabiServo Close!\r\n");
}
StabiServo::StabiServo() : mPin(PIN_STABI_SERVO)
{
	setName("stabiservo");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
StabiServo::~StabiServo()
{
}

//////////////////////////////////////////////
// CameraServo (hard pwm)
// 24ピンの問題でうごかねーんだ
//////////////////////////////////////////////
/*
bool CameraServo::onInit(const struct timespec& time)
{
	pinMode(mPin, PWM_OUTPUT);

	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(9000);
	pwmSetClock(32);

	return true;
}
void CameraServo::onClean()
{
	stop();
}
bool CameraServo::onCommand(const std::vector<std::string>& args)
{
	if(args.size() >= 2)
	{
		if(args[1].compare("stop") == 0)
		{
			stop();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}else if(args[1].compare("close") == 0)
		{
			close();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}else
		{
			//角度指定
			float period = 0;
			if(args.size() == 2)
			{
				period = atof(args[1].c_str());
			}
			start(period);
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
	}else
	{
		Debug::print(LOG_PRINT,"cameraservo [0-1]          : set servo angle\r\n\
cameraservo stop           : stop servo\r\n");
	}
	return true;
}
void CameraServo::start(double angle)
{
	//カメラサーボの可動域わからんので予防として[0.4,0.6]としておく　仲田
	if(angle > 0.6)angle = 0.6;
	else if(angle < 0.4)angle = 0.4;

	pwmWrite (mPin, SERVO_BASE_VALUE + angle * SERVO_MOVABLE_RANGE);
	Debug::print(LOG_DETAIL,"CameraServo Start (%f)!\r\n",angle);
}
void CameraServo::stop()
{
	pwmWrite (mPin, 0);
	Debug::print(LOG_DETAIL,"CameraServo Stop!\r\n");
}
void CameraServo::close()
{
	pwmWrite(mPin, SERVO_BASE_VALUE);
	Debug::print(LOG_DETAIL,"CameraServo Close!\r\n");
}
CameraServo::CameraServo() : mPin(PIN_CAMERA_SERVO)
{
	setName("cameraservo");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
CameraServo::~CameraServo()
{
}
*/

//////////////////////////////////////////////
// SoftCameraServo(Software PWM)
//////////////////////////////////////////////

bool SoftCameraServo::onInit(const struct timespec& time)
{
	if (wiringPiSetup() == -1)//Software PWMを使う前にwiringPiSetupを呼ぶ必要があるらしい
	{
		Debug::print(LOG_PRINT,"SoftCameraServoError: wiringPi setup failed...\n");
	}

	//pinMode(mPin, OUTPUT); //ピンを出力モードにするっぽい
	softPwmCreate(mPin, 0, SERVO_RANGE);	//int softPwmCreate (int pin, int initialValue, int pwmRange);
	return true;
}
void SoftCameraServo::onClean()
{
	stop();
}
bool SoftCameraServo::onCommand(const std::vector<std::string>& args)
{
	if(args.size() >= 2)
	{
		if(args[1].compare("stop") == 0)
		{
			stop();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
		else
		{
			//角度指定
			int period = 0;
			if(args.size() == 2)
			{
				period = atof(args[1].c_str());
			}
			start(period);
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
	}
	else
	{
		Debug::print(LOG_PRINT,"cameraservo [0 or %d-%d]\t: set servo position\r\n\
cameraservo stop\t\t: stop servo\r\n", SERVO_MIN_RANGE ,SERVO_MAX_RANGE);
	}
	return true;
}
void SoftCameraServo::start(int angle)
{
	//範囲のチェック
	if(angle >= SERVO_MAX_RANGE)
	{
		angle = SERVO_MAX_RANGE;
	}
	else if(angle <= 0)
	{
		angle = 0;
	}
	else if(angle < SERVO_MIN_RANGE)
	{
		angle = SERVO_MIN_RANGE;
	}

	softPwmWrite(mPin, angle);			//void softPwmWrite (int pin, int value);
	Debug::print(LOG_PRINT,"SoftCameraServo Start (%d)!\r\n",angle);
}
void SoftCameraServo::start(POSITION p)
{
	start((int)p);
}
void SoftCameraServo::stop()
{
	softPwmWrite (mPin, 0);
}
void SoftCameraServo::moveRelease()
{
	start(POSITION_RELEASE);
}
void SoftCameraServo::moveHold()
{
	start(POSITION_HOLD);
}
SoftCameraServo::SoftCameraServo() : mPin(PIN_CAMERA_SERVO_SOFT)
{
	setName("softcameraservo");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
SoftCameraServo::~SoftCameraServo()
{
}

//////////////////////////////////////////////
// BackStabiServo(Software PWM)
//////////////////////////////////////////////

bool BackStabiServo::onInit(const struct timespec& time)
{
	if (wiringPiSetup() == -1)//Software PWMを使う前にwiringPiSetupを呼ぶ必要があるらしい
	{
		Debug::print(LOG_PRINT,"BackStabiServoError: wiringPi setup failed...\n");
	}

	softPwmCreate(mPin, 0, SERVO_RANGE);	//int softPwmCreate (int pin, int initialValue, int pwmRange);
	//SERVO_RANGE　各種の定数はactuator.h 中
	return true;
}
void BackStabiServo::onClean()
{
	stop();
}
bool BackStabiServo::onCommand(const std::vector<std::string>& args)
{
	if(args.size() >= 2)
	{
		if(args[1].compare("stop") == 0)
		{
			stop();
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
		else
		{
			//角度指定
			int period = 0;
			if(args.size() == 2)
			{
				period = atof(args[1].c_str());
			}
			start(period);
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			return true;
		}
	}
	else
	{
		Debug::print(LOG_PRINT,"frontstabiservo [0 or %d-%d]\t: set servo position\r\n\
frontstabiservo stop\t\t: stop servo\r\n", SERVO_MIN_RANGE ,SERVO_MAX_RANGE);
	}
	return true;
}
void BackStabiServo::start(int angle)
{
	//範囲のチェック
	if(angle >= SERVO_MAX_RANGE)
	{
		angle = SERVO_MAX_RANGE;
	}
	else if(angle <= 0)
	{
		angle = 0;
	}
	else if(angle < SERVO_MIN_RANGE)
	{
		angle = SERVO_MIN_RANGE;
	}

	softPwmWrite(mPin, angle);			//void softPwmWrite (int pin, int value);
	Debug::print(LOG_PRINT,"BackStabiServo Start (%d)!\r\n",angle);
}
void BackStabiServo::start(POSITION p)
{
	start((int)p);
}
void BackStabiServo::stop()
{
	softPwmWrite (mPin, 0);
}
void BackStabiServo::moveRelease()
{
	start(POSITION_RELEASE);
}
void BackStabiServo::moveHold()
{
	start(POSITION_HOLD);
}
void BackStabiServo::moveGo()
{
	start(POSITION_GO);
}
BackStabiServo::BackStabiServo() : mPin(PIN_FRONT_STABI_SERVO)
{
	setName("backstabiservo");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
BackStabiServo::~BackStabiServo()
{
}


//////////////////////////////////////////////
// XBee Sleep
//////////////////////////////////////////////

/*bool XBeeSleep::onInit(const struct timespec& time)
{
	mPin = PIN_XBEE_SLEEP;
	pinMode(mPin, OUTPUT);
	digitalWrite(mPin, LOW);
	return true;
}
void XBeeSleep::onClean()
{
	digitalWrite(mPin, LOW);
}
bool XBeeSleep::onCommand(const std::vector<std::string>& args)
{
	if(args.size() == 2)
	{
		if(args[1].compare("sleep") == 0)
		{
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			setState(true);
			return true;
		}else if(args[1].compare("wake") == 0)
		{
			Debug::print(LOG_PRINT,"Command Executed!\r\n");
			setState(false);
			return true;
		}
	}else
	{
		Debug::print(LOG_PRINT,"xbee [sleep/wake] : set sleep mode\r\n");
	}
	return true;
}
void XBeeSleep::setState(bool sleep)
{
	digitalWrite(mPin, sleep ? HIGH : LOW);
}
XBeeSleep::XBeeSleep() : mPin(PIN_XBEE_SLEEP)
{
	setName("xbee");
	setPriority(TASK_PRIORITY_ACTUATOR,UINT_MAX);
}
XBeeSleep::~XBeeSleep()
{
}*/

Buzzer gBuzzer;
ParaServo gParaServo;
StabiServo gStabiServo;
BackStabiServo gBackStabiServo;
//CameraServo gCameraServo;
SoftCameraServo gSoftCameraServo;

//XBeeSleep gXbeeSleep;
