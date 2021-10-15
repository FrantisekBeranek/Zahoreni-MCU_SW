/*
 * shiftReg.c
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */
#include "shiftReg.h"


REG_STATE regState;

//___BUFFER___//
RING_BUFFER* regBuffer;

//___Pole hodnot k poslaní___//
//velikost pole odpovídá počtu registrů v sérii
uint8_t* regValues;
uint8_t regCount;

//___Řídící struktura SPI___//
extern SPI_HandleTypeDef hspi1;

//___Importované proměnné z main.c___//
extern Flags flags;



//_____Zjistí počet registrů_____//
static uint8_t getCount(void)
{
	uint8_t question;
	uint8_t answer;
	regCount = 0;

	do
	{
		question = 42;
		answer = 0;
		if(HAL_SPI_TransmitReceive(&hspi1, &question, &answer, 1, 100) != HAL_OK)
			return 0;
		regCount++;

		HAL_Delay(1);

		if(regCount >= 100)	//Ošetření nepřipojených relé desek
		{
			flags.conErr = 1;
			return 0;
		}
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

	if(getCount() == 0)
	{
		return REG_CON_ERR;	//Connection error
	}

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

	sendData();

	REG_ENABLE;

	regState = (HAL_SPI_Transmit(&hspi1, &regValues[0], regCount, 100) == HAL_OK)? REG_OK : REG_ERR;
	return regState;
}

//_____Pošle data z regValues do registrů_____//
REG_STATE sendData(void)
{
	if(HAL_SPI_Transmit(&hspi1, &regValues[0], regCount, 100) == HAL_OK)
	{
		//vytvoř pulz na RCLK¨
		REG_RCLK_HIGH;
		HAL_Delay(1);
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
//Zavolat v main uvnitř časované smyčky
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
