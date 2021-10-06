/*
 * testHandle.c
 *
 *  Created on: 6. 10. 2021
 *      Author: František Beránek
 */

#include "testHandle.h"

TEST_PHASE testPhase = WAITING;

extern Flags flags;

//_____Funkce pro řízení testu_____//
void testHandler()
{
	if(flags.startRequest)
	{
		if(testPhase == WAITING)
		{
			startTest();
		}
		else
		{
			flags.startConflict = 1;
		}
	}
}

//_____Funkce pro zahájení testu_____//
static void startTest(/*ukazatel na zdroj*/)
{
	flags.shortBeep = 1;
	testPhase = START;

	//poslat konfiguraci shift registrům
	//Zobrazit text na displej
	//spustit měření
}
