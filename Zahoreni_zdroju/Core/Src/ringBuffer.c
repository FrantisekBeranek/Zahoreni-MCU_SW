/*
 * ringBuffer.c
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */

#include <stdio.h>
#include "ringBuffer.h"

//_____Vytvoří buffer v dynamické paměti_____//
//Návratová hodnota - ukazatel na vytvořený buffer (NULL = chyba)
//Argument - velikost bufferu
RING_BUFFER* createBuffer(int size)
{
	RING_BUFFER* buffer = (RING_BUFFER*) malloc(sizeof(RING_BUFFER));
	if(buffer == NULL)			//Nepodařilo se alokovat paměť
	{
		return NULL;
	}
	buffer->buffer = (char*) malloc(size * sizeof(char));
	if(buffer->buffer == NULL)	//Nepodařilo se alokovat paměť
	{
		free(buffer);
		return NULL;
	}

	buffer->bufferSize = size;
	buffer->filled = 0;
	buffer->first = 0;
	buffer->last = size-1;
	buffer->status = BUFFER_EMPTY;

	return buffer;
}

//_____Uvolní paměť a odstraní buffer_____//
void destroyBuffer(RING_BUFFER* buffer)
{
	free(buffer->buffer);
	free(buffer);
}

//_____Vrátí hodnotu status_____//
BUFFER_STATE getStatus(RING_BUFFER* buffer)
{
	return buffer->status;
}

//_____Uloží znak do bufferu_____//
BUFFER_STATE push(RING_BUFFER* buffer, char character)
{
	if(buffer->status == BUFFER_FULL)
		return BUFFER_FULL;

	buffer->last = (buffer->last + 1) % (buffer->bufferSize);
	buffer->filled++;
	buffer->buffer[buffer->last] = character;
	buffer->status = (buffer->filled >= buffer->bufferSize)? BUFFER_FULL : BUFFER_OK;

	return BUFFER_OK;
}

//_____Uloží řetězec do bufferu_____//
BUFFER_STATE pushStr(RING_BUFFER* buffer, char* str, int len)
{
	if(buffer->bufferSize < (buffer->filled + len))
		return BUFFER_FULL;

	for(int i = 0; i < len; i++)
	{
		push(buffer, str[i]);
	}

	return BUFFER_OK;
}

//_____Přečte a odstraní poslední znak z bufferu_____//
//Znak bude uložen na adresu v argumentu
BUFFER_STATE pop(RING_BUFFER* buffer, char* character)
{
	if(buffer->status == BUFFER_EMPTY)
		return BUFFER_EMPTY;

	*character = buffer->buffer[buffer->first];
	buffer->first = (buffer->first + 1) % (buffer->bufferSize);
	buffer->filled--;
	buffer->status = (buffer->filled <= 0)? BUFFER_EMPTY : BUFFER_OK;

	return BUFFER_OK;
}

//_____Přečte a zachová hodnotu na dané pozici od prvního uloženého znaku_____//
BUFFER_STATE at(RING_BUFFER* buffer, int index, char* character)
{
	if(buffer->status == BUFFER_EMPTY)
		return BUFFER_EMPTY;

	 if((index + 1) > buffer->filled)
		 return BUFFER_ERR;

	 int tmp = (buffer->first + index) % (buffer->bufferSize);
	 *character = buffer->buffer[tmp];

	 return BUFFER_OK;
}

