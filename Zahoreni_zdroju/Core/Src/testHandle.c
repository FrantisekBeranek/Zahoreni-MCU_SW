/*
 * testHandle.c
 *
 *  Created on: 6. 10. 2021
 *      Author: František Beránek
 */

#include "testHandle.h"

static TEST_PHASE testPhase = WAITING;

//___Proměnné z main.c___//
extern Flags flags;
extern int sysTime[4];

//___Proměnné z shiftReg.c___//
extern uint8_t* regValues;
extern uint8_t regCount;

uint8_t* sourceInTesting;
uint8_t tmp;

static void startTest();
static void stopTest();

//_____Dotaz na aktuální fázi testu_____//
TEST_PHASE currentPhase()
{
	return testPhase;
}

//_____Funkce pro řízení testu_____//
void testHandler()
{
	flags.testProgress = 0;

	if(flags.instructions.startRequest)
	{
		if(testPhase == WAITING)
		{
			if(flags.conErr)
			{
				//Pošli zprávu do PC
			}
			else
			{
				startTest();
			}
		}
		else
		{
			flags.startConflict = 1;
		}
	}
	if(flags.instructions.stopRequest)
	{
		stopTest();
	}

	switch(testPhase)
	{
	case WAITING:
		flags.testProgress = 0;
		flags.meas.measRequest = 0;
		break;
	case START:

		//___Pokud je dokončeno měření napětí naprázdno...____//
		if(flags.meas.measComplete)
		{
			testPhase++;
		}
		break;
	case START_DONE:
		//___Připojení zátěže___//
		LOAD_MIN_ON;
		LOAD_MAX_ON;

		testPhase++;
		flags.testProgress = 1;
		flags.ui.shortBeep = 1;

		//___Nulování času___//
		for(int i = 1; i < 4; i++)
		{
			sysTime[i] = 0;
		}
		break;
	case MAIN_TEST:
		if(flags.time.sec)	//___Změna času___//
		{
			char time[9] = {0};
			sprintf(time, "%d:%d:%d", 60-sysTime[SYSTIME_SEC], 60-sysTime[SYSTIME_MIN], 3-sysTime[SYSTIME_HOUR]);
			//writeRow(time, strlen(time), 0, LEFT);

			PROGRESS_RUNNING(*sourceInTesting, PROGRESS_LED2);	//blikání druhé progress led
			sendData();
		}
		if(!(sysTime[SYSTIME_MIN] % 10) && sysTime[SYSTIME_MIN] != 0 && flags.time.min)	//___Měření napětí každých deset minut___//
		{
			flags.meas.measRequest = 1;
		}
		if(sysTime[SYSTIME_HOUR] >= 3)	//___Po třech hodinách je měření u konce___//
		{
			testPhase++;
		}
		break;
	case MAIN_TEST_DONE:
		if(flags.meas.measComplete)
		{
			flags.ui.notice = 1;
			flags.testProgress = 1;

			testPhase++;

			PROGRESS_ON(*sourceInTesting, PROGRESS_LED2);
			PWR_OFF(*sourceInTesting);
			sendData();

			//___Nulování času___//
			for(int i = 1; i < 4; i++)
			{
				sysTime[i] = 0;
			}
		}
		break;
	case BATTERY_TEST:
		if(flags.time.sec)	//___Změna času___//
		{
			char time[9] = {0};
			sprintf(time, "%d:%d:%d", 60-sysTime[SYSTIME_SEC], 60-sysTime[SYSTIME_MIN], 3-sysTime[SYSTIME_HOUR]);
			//writeRow(time, strlen(time), 0, LEFT);

			PROGRESS_RUNNING(*sourceInTesting, PROGRESS_LED3);	//blikání třetí progress led
			sendData();
		}
		if(!(sysTime[SYSTIME_MIN] % 5) && sysTime[SYSTIME_MIN] != 0 && flags.time.min)	//___Měření napětí každých pět minut___//
		{
			flags.meas.measRequest = 1;
		}
		if(sysTime[SYSTIME_MIN] >= 15)	//___Po třech hodinách je měření u konce___//
		{
			testPhase++;
		}
		break;
	case BATTERY_TEST_DONE:
		if(flags.meas.measComplete)
		{
			flags.ui.done = 1;
			flags.testProgress = 1;

			//Zobrazit text na displej

			LOAD_MIN_OFF;
			LOAD_MAX_OFF;

			PROGRESS_ON(*sourceInTesting, PROGRESS_LED3);
			PWR_ON(*sourceInTesting);
			sendData();

			testPhase = WAITING;
		}
		break;

	}
}

//_____Funkce pro zahájení testu_____//
static void startTest(/*ukazatel na zdroj*/)
{
	flags.ui.shortBeep = 1;
	testPhase = START;
	flags.testProgress = 1;

	sourceInTesting = &regValues[0/*ukazatel na zdroj*/];
	/*
	 * for(int i = 0; i < regCount; i++)
	 * {
	 * 		regValues[i] = 0;
	 * }
	 * PROGRESS_ON(*sourceInTesting, PROGRESS_LED1);	//rozsvítit první ledku progress
	 * RELAY_ON(*sourceInTesting);	//připojit relé
	 *
	 * sendData();	//poslat konfiguraci shift registrům
	 */
	//Zobrazit text na displej
	flags.meas.measRequest = 1;	//spustit měření

	flags.instructions.startRequest = 0;
}

//_____Funkce pro ukončení testu_____//
static void stopTest()
{
	flags.ui.longBeep = 1;
	testPhase = WAITING;

	//Poslat data shift registrům
	//Zobrazit text na displej
	LOAD_MIN_OFF;
	LOAD_MAX_OFF;

	flags.instructions.stopRequest = 0;
}
