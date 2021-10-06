/*
 * testHandle.h
 *
 *  Created on: 6. 10. 2021
 *      Author: František Beránek
 */

#ifndef TESTHANDLE_H_
#define TESTHANDLE_H_

#include "lcd.h"
#include "main.h"


typedef enum{
	WAITING = 0U,
	START, START_DONE,
	MAIN_TEST, MAIN_TEST_DONE,
	BATTERY_TEST, BATTERY_TEST_DONE
}TEST_PHASE;

//_____Funkce pro řízení testu_____//
void testHandler();

//_____Funkce pro zahájení testu_____//
static void startTest(/*doplnit ukazatel na konkrétní zdroj*/);

#endif /* TESTHANDLE_H_ */
