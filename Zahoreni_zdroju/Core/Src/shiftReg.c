/*
 * shiftReg.c
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */
#include "shiftReg.h"



//_____Zjistí počet registrů_____//
uint8_t getCount(void)
{
	uint8_t question = 42;
	uint8_t answer = 0;
	regCount = 0;

	do
	{
		if(HAL_SPI_TransmitReceive(&hspi1, &question, &answer, 1, 100) != HAL_OK)
			return 0;
		regCount++;
	}
	while(answer != question);

	regCount--;

	return regCount;
}

//_____Inicializuje registry_____//
REG_STATE regInit(void)
{
	REG_CLR_ACTIVE;
	HAL_Delay(5);
	REG_CLR_INACTIVE;

	REG_DISABLE;

	getCount();
	regValues = (uint8_t*) malloc(regCount * sizeof(uint8_t));
	if(regValues == NULL)
	{
		regState = REG_ERR;
		return REG_ERR;
	}

	for(int i = 0; i < regCount; i++)
	{
		regValues[i] = 0;
	}

	REG_ENABLE;

	regState = (HAL_SPI_Transmit(&hspi1, &regValues[0], regCount, 100) == HAL_OK)? REG_OK : REG_ERR;
	return regState;
}

//_____Pošle data z regValues do registrů_____//
REG_STATE sendData(void)
{
	if((HAL_SPI_Transmit(&hspi1, &regValues[0], regCount, 100) == HAL_OK))
	{
		//vytvoř pulz na RCLK¨
		REG_RCLK_HIGH;
		HAL_Delay(5);
		REG_RCLK_LOW;

		regState = REG_OK;
	}
	else
	{
		regState = REG_ERR;
	}

	return regState;
}

//_____Řídí obsluhu registrů při neblokujícím módu_____//
//Zavolat v sysCLK přerušení
void regHandler(void)
{
	switch(regState)
	{
	case REG_BUSY:
		REG_RCLK_HIGH;
		regState = REG_READY;
		break;

	case REG_READY:
		REG_RCLK_LOW;
		regState = REG_OK;
		break;

	case REG_RESET:
		REG_CLR_ACTIVE;
		regState = REG_SET;
		break;

	case REG_SET:
		REG_CLR_INACTIVE;
		regState = REG_OK;
		break;

	default:
		break;
	}
}
