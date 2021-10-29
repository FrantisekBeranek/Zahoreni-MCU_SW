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


void comHandler(void);


#endif /* INC_COMHANDLE_H_ */
