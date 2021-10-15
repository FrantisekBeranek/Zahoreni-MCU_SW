/*
 * comHandler.c
 *
 *  Created on: 15. 10. 2021
 *      Author: František Beránek
 */

#include "comHandle.h"

//___Buffery pro USB___//
		RING_BUFFER* USB_Tx_Buffer;
extern	RING_BUFFER* USB_Rx_Buffer;

//___Importované proměnné z main.c___//
extern Flags flags;
extern uint32_t ADC_Results[16];

//___Importované proměnné z testHandle.c___//
extern int testNum;

//_____Obsluha komunikace s PC přes USB_____//
/*
 * Funkce obsluhuje příjem i odesílání dat.
 * V jakékoliv jiné části kódu by nemělo dojít k volání funkce CDC_Transmit_FS, která
 * se stará o samotné odesílání. Data by se pouze měla ukládat do bufferu USB_Tx_Buffer
 * metodou push nebo pushStr.
 *
 * Při přijmutí instrukce nedochází k zpracování, ale pouze k nastavení adekvátního flagu.
 * Vykonání instrukce musí být zařízeno v jiné části hlavního programu.
 */
void comHandler(void)
{
	//___Příjem dat___//
	if(flags.data_received)
	{
		char instruction;
		while(pop(USB_Rx_Buffer, &instruction) != BUFFER_EMPTY)
		{
#ifdef __DEBUG_INST__
			uint8_t txt[30];
#endif

			switch(instruction)
			{
			case 's': ;
				//___Start testu___//
				flags.instructions.startRequest = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Start\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				break;

			case'c': ;
				//___Ukončení___//
				flags.instructions.stopRequest = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Ukonceni\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				break;

			case'p': ;
				//___Pauza___//
				flags.instructions.pauseRequest = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Pauza\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				break;

			case'k': ;
				//___Kalibrace___//
				flags.instructions.calibRequest = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Kalibrace\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				break;

			default: ;
				//___Neplatný příkaz___//
				flags.instructions.unknownInst = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Neplatna instrukce\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				break;
			}
		}
		flags.data_received = 0;
	}

	if(flags.testProgress)
	{
		char testPhaseChr;
		switch(currentPhase())
		{
		case START:
			testPhaseChr = 's';
			break;
		case START_DONE:
			testPhaseChr = 's';
			break;
		case MAIN_TEST:
			testPhaseChr = 'm';
			break;
		case MAIN_TEST_DONE:
			testPhaseChr = 'm';
			break;
		case BATTERY_TEST:
			testPhaseChr = 'b';
			break;
		case BATTERY_TEST_DONE:
			testPhaseChr = 'M';
			break;
		default:
			testPhaseChr = 'e';
			break;
		}
		char txt[30];
		sprintf(txt, "Test progress #%c\n", testPhaseChr);
		pushStr(USB_Tx_Buffer, txt, strlen(txt));
	}

	if(flags.meas.measComplete)
	{
		char txt[10];
		sprintf(txt, "#%d\n", testNum);
		pushStr(USB_Tx_Buffer, txt, strlen(txt));

		uint8_t measResult[32];
		for(int i = 0; i < 16; i++)
		{
			measResult[2*i] = ADC_Results[i] & 0x00FF;
			measResult[2*i + 1] = (ADC_Results[i] & 0xFF00) >> 8;
		}
		pushStr(USB_Tx_Buffer, measResult, 32);

		testNum++;
	}

	//___Odesílání dat___//
	//_Ošetření plného bufferu_//
	if(USB_Tx_Buffer->status == BUFFER_FULL)
	{
		char msg[] = {"Buffer full\n"};
		CDC_Transmit_FS(msg, strlen(msg));
	}
	//_Samotné odesílání_//
	if(USB_Tx_Buffer->filled)
	{
		int size = USB_Tx_Buffer->filled;
		char tmpStr[size];
		for(int i = 0; i < size; i++)
		{
			pop(USB_Tx_Buffer, &tmpStr[i]);
		}
		CDC_Transmit_FS(tmpStr, size);
	}

}
