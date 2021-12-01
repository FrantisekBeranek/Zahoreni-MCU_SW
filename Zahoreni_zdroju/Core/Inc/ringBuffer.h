/*
 * ringBuffer.h
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdlib.h>

/* Definované stavy bufferu */
typedef enum
{
	BUFFER_OK = 0U,
	BUFFER_EMPTY,
	BUFFER_FULL,
	BUFFER_ERR,
}BUFFER_STATE;

/* Struktura bufferu */
typedef struct
{
	char* buffer;
	int bufferSize;
	int filled;
	int first;
	int last;
	BUFFER_STATE status;
}RING_BUFFER;

//_____Vytvoří buffer v dynamické paměti_____//
/* @ret		ukazatel na vytvořený buffer (NULL = chyba) */
/* @param	int Velikost bufferu */
RING_BUFFER* createBuffer(int);

//_____Uvolní paměť a odstraní buffer_____//
/* @param	ukazatel na buffer */
void destroyBuffer(RING_BUFFER*);

//_____Uvede buffer do výchozího stavu_____//
/* @param	ukazatel na buffer */
void clearBuffer(RING_BUFFER*);

//_____Vrátí hodnotu status_____//
/* @param	ukazatel na buffer */
BUFFER_STATE getStatus(RING_BUFFER*);

//_____Uloží znak do bufferu_____//
/* @param	ukazatel na buffer */
/* @param	znak k uložení */
BUFFER_STATE push(RING_BUFFER*, char);

//_____Uloží řetězec do bufferu_____//
/* @param	ukazatel na buffer */
/* @param	ukazatel na počátek řetězce k uložení */
/* @param	délka řetězce */
BUFFER_STATE pushStr(RING_BUFFER*, char*, int);

//_____Přečte a odstraní poslední znak z bufferu_____//
/* @param	ukazatel na buffer */
/* @param	adresa, na kterou se načtený znak uloží */
BUFFER_STATE pop(RING_BUFFER*, char*);

//_____Přečte a zachová hodnotu na dané pozici od začátku_____//
/* @param	ukazatel na buffer */
/* @param	index od počátku */
/* @param	adresa, na kterou se načtený znak uloží */
BUFFER_STATE at(RING_BUFFER*, int, char*);


#endif /* INC_RINGBUFFER_H_ */
