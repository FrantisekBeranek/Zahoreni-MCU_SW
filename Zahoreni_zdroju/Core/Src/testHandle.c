/*
 * testHandle.c
 *
 *  Created on: 6. 10. 2021
 *      Author: František Beránek
 */

#include "testHandle.h"

static	TEST_PHASE testPhase = WAITING;
		int testNum = 0;

//___Proměnné z main.c___//
extern Flags flags;
extern int sysTime[4];
extern volatile uint8_t supplyToTest;

extern RING_BUFFER* USB_Tx_Buffer;

//___Proměnné z shiftReg.c___//
extern uint8_t* regValues;
extern uint8_t regCount;

uint8_t* sourceInTesting;

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
	flags.testCanceled = 0;

	if(flags.instructions.startRequest)
	{
		if(testPhase == WAITING)
		{
			if(flags.conErr)
			{
				char txt[] = {"Relay PCB connection error\n"};
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
				flags.instructions.startRequest = 0;
			}
			else
			{
				startTest();
			}
		}
		else
		{
			flags.startConflict = 1;
			flags.instructions.startRequest = 0;
		}
	}
	if(flags.instructions.stopRequest)
	{
		stopTest();
	}

	if(testPhase != WAITING)
	{
		if(flags.buttons.butt0_ver)
		{
			stopTest();
			flags.testCanceled = 1;
		}
	}

	switch(testPhase)
	{
	case WAITING:
		flags.testProgress = 0;
		//flags.meas.measRequest = 0;
		break;
	case START:

		//___Pokud je dokončeno měření napětí naprázdno...____//
		if(flags.meas.measComplete)
		{
			testPhase++;
		}

		if(flags.time.sec)
		{
			PROGRESS_RUNNING(*sourceInTesting, PROGRESS_LED1);	//blikání prvni progress led
			sendData();

			switch(sysTime[SYSTIME_SEC])
			{
			case 1:	//Po jedne sekunde zapnout topeni
				HAL_Delay(2);	//pro oddaleni sepnuti rele od spi komunikace
				HTR_ON;
				EM_HTR_ON;
				break;
			case 2:
				break;
			case 3:	//V treti sekunde overit funkcnost topeni
				if(HAL_GPIO_ReadPin(HEATER_STATE_GPIO_Port, HEATER_STATE_Pin) != GPIO_PIN_RESET)	//Topení neni v poradku
				{
					flags.heaterState = HEATER_ERR;
				}
				//vypnout topeni optotriak
				HTR_OFF;
				break;
			case 4:
				if(HAL_GPIO_ReadPin(HEATER_STATE_GPIO_Port, HEATER_STATE_Pin) != GPIO_PIN_SET)	//Topení neni v poradku
				{
					flags.heaterState = HEATER_TRIAC_ERR;
				}
				//vypnout topeni
				HAL_Delay(2);	//pro oddaleni sepnuti rele od spi komunikace
				EM_HTR_OFF;
				break;
			case 5:	//v pate sekunde zmerit napeti naprazdno
				flags.meas.measRequest = 1;
				break;
			default:
				break;
			}
		}
		break;
	case START_DONE:
		//___Připojení zátěže___//
		LOAD_MIN_ON;
		LOAD_MAX_ON;

		testPhase++;
		flags.testProgress = 1;
		//flags.ui.shortBeep = 1;

		PROGRESS_ON(*sourceInTesting, PROGRESS_LED1);	//rozsvítit první led

		//___Nulování času___//
		for(int i = 1; i < 4; i++)
		{
			sysTime[i] = 0;
		}
		break;
	case MAIN_TEST:
		if(flags.time.sec)	//___Změna času___//
		{
			PROGRESS_RUNNING(*sourceInTesting, PROGRESS_LED2);	//blikání druhé progress led
			sendData();
		}
#ifdef __DEBUG_TEST__
		if(sysTime[SYSTIME_MIN] != 0 && flags.time.min)	//___Měření napětí každou minutu___//
#else
		if(!(sysTime[SYSTIME_MIN] % 10) && !(sysTime[SYSTIME_MIN] == 0 && sysTime[SYSTIME_HOUR] == 0) && flags.time.min)	//___Měření napětí každých deset minut___//
#endif
		{
			flags.meas.measRequest = 1;
		}
#ifdef __DEBUG_TEST__
		if(sysTime[SYSTIME_MIN] >= 3)	//___Po deseti minutách je měření u konce___//
#else
		if(sysTime[SYSTIME_HOUR] >= 3)	//___Po třech hodinách je měření u konce___//
#endif
		{
			testPhase++;
		}
		break;
	case MAIN_TEST_DONE:
		if(!flags.meas.measRunning)
		{
			flags.ui.notice = 1;
			flags.testProgress = 1;

			testPhase++;

			LOAD_MIN_OFF;
			LOAD_MAX_OFF;

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
			PROGRESS_RUNNING(*sourceInTesting, PROGRESS_LED3);	//blikání třetí progress led
			sendData();
		}
#ifdef __DEBUG_TEST__
		if(sysTime[SYSTIME_MIN] != 0 && flags.time.min)	//___Měření napětí každou minutu___//
#else
		if(!(sysTime[SYSTIME_MIN] % 5) && sysTime[SYSTIME_MIN] != 0 && flags.time.min)	//___Měření napětí každých pět minut___//
#endif
		{
			flags.meas.onlyBattery = 1;
			flags.meas.measRequest = 1;
		}
#ifdef __DEBUG_TEST__
		if(sysTime[SYSTIME_MIN] >= 2)	//___Po třech minutách je měření u konce___//
#else
		if(sysTime[SYSTIME_MIN] >= 15)	//___Po patnácti minutách je měření u konce___//
#endif
		{
			testPhase++;
		}
		break;
	case BATTERY_TEST_DONE:
		if(!flags.meas.measRunning)
		{
			flags.ui.done = 1;
			flags.testProgress = 1;

			//Zobrazit text na displej

			PROGRESS_ON(*sourceInTesting, PROGRESS_LED3);
			RELAY_OFF(*sourceInTesting);
			PWR_ON(*sourceInTesting);
			sendData();

			flags.meas.onlyBattery = 0;

			testPhase = WAITING;
		}
		break;

	}
}

//_____Funkce pro zahájení testu_____//
static void startTest(/*ukazatel na zdroj*/)
{
	if(supplyToTest > regCount)
	{
		return;
	}

	flags.ui.shortBeep = 1;
	testPhase = START;
	testNum = 0;
	flags.testProgress = 1;

	sourceInTesting = &regValues[regCount - (supplyToTest+1)];	//První deska (spodní) je řízena posledním bytem

	for(int i = 0; i < regCount; i++)
	{
		//Power up and disconnect all supplies
		RELAY_OFF(regValues[i]);
		PWR_ON(regValues[i]);
	}
	PROGRESS_ON(*sourceInTesting, PROGRESS_LED1);	//rozsvítit první ledku progress
	PROGRESS_OFF(*sourceInTesting, PROGRESS_LED2);	//Zhasnout zbyle led
	PROGRESS_OFF(*sourceInTesting, PROGRESS_LED3);
	ERROR_OFF(*sourceInTesting);
	RELAY_ON(*sourceInTesting);	//připojit relé

	sendData();	//poslat konfiguraci shift registrům

	//___Nulování času___//
	for(int i = 1; i < 4; i++)
	{
		sysTime[i] = 0;
	}

	flags.instructions.startRequest = 0;
}

//_____Funkce pro ukončení testu_____//
static void stopTest()
{
	flags.ui.longBeep = 1;
	testPhase = WAITING;

	//Zobrazit text na displej
	LOAD_MIN_OFF;
	LOAD_MAX_OFF;

	if(sourceInTesting != NULL)
	{
		*sourceInTesting = 0;
		ERROR_ON(*sourceInTesting);
	}
	sendData();

	flags.instructions.stopRequest = 0;
}
