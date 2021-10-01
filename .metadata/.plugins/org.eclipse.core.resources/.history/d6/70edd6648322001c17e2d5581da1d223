/*
 * ringBuffer.h
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdlib.h>

typedef enum
{
	BUFFER_OK = 0U,
	BUFFER_EMPTY,
	BUFFER_FULL,
	BUFFER_ERR,
}BUFFER_STATE;

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
//Návratová hodnota - ukazatel na vytvořený buffer (NULL = chyba)
//Argument - velikost bufferu
RING_BUFFER* createBuffer(int);

//_____Uvolní paměť a odstraní buffer_____//
void destroyBuffer(RING_BUFFER*);

//_____Vrátí hodnotu status_____//
BUFFER_STATE getStatus(RING_BUFFER*);

//_____Uloží znak do bufferu_____//
BUFFER_STATE push(RING_BUFFER*, char);

//_____Přečte a odstraní poslední znak z bufferu_____//
//Znak bude uložen na adresu v argumentu
BUFFER_STATE pop(RING_BUFFER*, char*);

//_____Přečte a zachová hodnotu na dané pozici od začátku_____//
BUFFER_STATE at(RING_BUFFER*, int, char*);


#endif /* INC_RINGBUFFER_H_ */
