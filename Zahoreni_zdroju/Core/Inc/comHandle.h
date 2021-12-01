/*
 * comHandler.h
 *
 *  Created on: 15. 10. 2021
 *      Author: František Beránek
 */

#ifndef INC_COMHANDLE_H_
#define INC_COMHANDLE_H_

#include "ringBuffer.h"
#include "main.h"
#include "testHandle.h"
#include <stdio.h>

//___Buffer pro USB___//
extern	RING_BUFFER* USB_Tx_Buffer;
extern	RING_BUFFER* USB_Rx_Buffer;

//___Importované proměnné z main.c___//
//extern Flags flags;
//extern uint32_t ADC_Results[16] = {0};

//_____Struktura paketu_____//
typedef struct{
	//uint8_t startSequence[] = ">>";
	uint8_t type;
	uint8_t* data;
	uint8_t dataLength;
	uint8_t CA_value;	//control addiction
	//uint8_t endSequence[] = "<<\n";
}Paket;

//_____Typy paketů posílaných do PC_____//
typedef enum{
	DATA_PAKET = 1,
	DATA_BAT_PAKET,
	TEST_NUM_PAKET,
	TEST_PHASE_PAKET,
	ACK_PAKET,
	REFRESH_PAKET
}outPaketType;

//_____Typy příchozích paketů z PC_____//
typedef enum{
	START_PAKET = 's',
	CANCEL_PAKET = 'c',
	PAUSE_PAKET = 'p',
	CALIB_PAKET = 'k',
	CON_PAKET = 'a'
}inPaketType;

//_____Funkce pro řízení komunikace přes USB_____//
void comHandler(void);


#endif /* INC_COMHANDLE_H_ */
