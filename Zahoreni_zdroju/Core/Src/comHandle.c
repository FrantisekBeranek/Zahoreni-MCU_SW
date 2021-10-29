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
/*
 * 		U15V, U15V_CURRENT,
 *		U12V, U12V_CURRENT,
 *		U24VO2, U24VO2_CURRENT,
 *		U24V, U24V_CURRENT,
 *		U5VK, U5VK_CURRENT,
 *		U5V, U5V_CURRENT,
 *		U_BAT,
 *		PAD9, PAD15,
 *		U48V_CURRENT
 */

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
#ifdef __APP_COMPATIBILITY__
		sprintf(txt, "#%c\n", testPhaseChr);
#else
		sprintf(txt, "Test progress #%c\n", testPhaseChr);
#endif
		pushStr(USB_Tx_Buffer, txt, strlen(txt));
	}

	if(flags.meas.measComplete)
	{
		if(!flags.meas.calibMeas)
		{
			push(USB_Tx_Buffer, '#');
			push(USB_Tx_Buffer, testNum);
			push(USB_Tx_Buffer, '\n');
			testNum++;
		}
		else
		{
			flags.meas.calibMeas = 0;
		}

		if(flags.meas.onlyBattery)
		{
			char res[20] = {0};
			sprintf(res, "%d;\n", ADC_Results[12]);
			pushStr(USB_Tx_Buffer, res, strlen(res));
		}
		else
		{
			for(int i = 0; i < 7; i++)
			{
				char res[20];
				sprintf(res, "%d;", ADC_Results[2*i]);
				pushStr(USB_Tx_Buffer, res, strlen(res));
			}
			push(USB_Tx_Buffer, 0x0A);
		}
	}

#ifdef __APP_COMPATIBILITY__
	if(flags.time.sec)
	{
		char txt[] = {"#Hi\n"};
		pushStr(USB_Tx_Buffer, txt, strlen(txt));
	}
#endif

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
