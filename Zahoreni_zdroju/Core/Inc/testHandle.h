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
#include "shiftReg.h"
#include <stdio.h>
#include <string.h>

//___Nastavení bitu pro řízení napájení___//
#define PWR_ON(x) x&=~(1<<0)
#define PWR_OFF(x) x|=(1<<0)
//___Nastavení bitu pro řízení relé___//
#define RELAY_OFF(x) x&=~(1<<1)
#define RELAY_ON(x) x|=(1<<1)
//___Nastavení bitu pro řízení error led___//
#define ERROR_OFF(x) x&=~(1<<2)
#define ERROR_ON(x) x|=(1<<2)
//___Nastavení bitů pro řízení progress led___//
#define PROGRESS_ON(x,y) x&=~(1<<y)
#define PROGRESS_OFF(x,y) x|=(1<<y)
#define PROGRESS_RUNNING(x,y) x^=(1<<y)

#define PROGRESS_LED1 5
#define PROGRESS_LED2 4
#define PROGRESS_LED3 3

typedef enum{
	WAITING = 0U,
	START, START_DONE,
	MAIN_TEST, MAIN_TEST_DONE,
	BATTERY_TEST, BATTERY_TEST_DONE
}TEST_PHASE;

//___Proměnné z main.c___//
//extern Flags flags;
//extern int sysTime[4];

//___Proměnné z shiftReg.c___//
//extern uint8_t* regValues;
//extern uint8_t regCount;

extern uint8_t* sourceInTesting;

//_____Dotaz na aktuální fázi testu_____//
TEST_PHASE currentPhase();

//_____Funkce pro řízení testu_____//
void testHandler();

//_____Funkce pro zahájení testu_____//
//static void startTest(/*doplnit ukazatel na konkrétní zdroj*/);

//_____Funkce pro ukončení testu_____//
//static void stopTest(/*doplnit ukazatel na konkrétní zdroj*/);

#endif /* TESTHANDLE_H_ */
