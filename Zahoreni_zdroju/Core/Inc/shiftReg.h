/*
 * shiftReg.h
 *
 *  Created on: 2. 9. 2021
 *      Author: beranekf
 */

#ifndef INC_SHIFTREG_H_
#define INC_SHIFTREG_H_


#include "stm32f0xx_hal.h"
#include "ringBuffer.h"
#include "main.h"
#include <stdlib.h>

/* Řízení pinu Output Enable */
#define REG_ENABLE HAL_GPIO_WritePin(SR_OE_GPIO_Port, SR_OE_Pin, GPIO_PIN_RESET)
#define REG_DISABLE HAL_GPIO_WritePin(SR_OE_GPIO_Port, SR_OE_Pin, GPIO_PIN_SET)

/* Řízení pinu RCLK */
#define REG_RCLK_HIGH HAL_GPIO_WritePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_SET)
#define REG_RCLK_LOW HAL_GPIO_WritePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_RESET)
#define REG_RCLK_TOGGLE HAL_GPIO_TogglePin(SR_RCLK_GPIO_Port, SR_RCLK_Pin, GPIO_PIN_RESET)

/* Řízení pinu CLR */
#define REG_CLR_INACTIVE HAL_GPIO_WritePin(SR_CLR_GPIO_Port, SR_CLR_Pin, GPIO_PIN_SET)
#define REG_CLR_ACTIVE HAL_GPIO_WritePin(SR_CLR_GPIO_Port, SR_CLR_Pin, GPIO_PIN_RESET)

//___Stav registrů___//
typedef enum
{
	REG_OK = 0U,
	REG_CON_ERR,	//Chyba připojení registrů
	REG_ERR,
	REG_RESET,
	REG_SET,
	REG_SENDING,
	REG_BUSY,		//Hodnoty jsou nastaveny a čeká se na vyslání na výstup
	REG_READY
}REG_STATE;

extern REG_STATE regState;

//___BUFFER___//
extern RING_BUFFER* regBuffer;

//___Pole hodnot k poslaní___//
//velikost pole odpovídá počtu registrů v sérii (regCount)
extern uint8_t* regValues;
extern uint8_t regCount;

//___Řídící struktura SPI___//
extern SPI_HandleTypeDef hspi1;

//___Importované proměnné z main.c___//
//extern Flags flags;

//_____Zjistí počet registrů_____//
//static uint8_t getCount(void);

//_____Inicializuje registry_____//
REG_STATE regInit(void);

//_____Pošle data z values do registrů_____//
REG_STATE sendData(void);

//_____Řídí obsluhu registrů při neblokujícím módu_____//
// -- Nedokončená funkce -- //
void regHandler(void);



#endif /* INC_SHIFTREG_H_ */
