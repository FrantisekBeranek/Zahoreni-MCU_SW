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

//___Pole pro převod dat převodníku na pole bytů___//
uint8_t data[14];

/* definice funkcí */
void comHandler(void);
static void makeByteArray();
static void fillPaket(Paket* paket, outPaketType type, uint8_t* data, uint8_t dataLength);
static void pushPaket(RING_BUFFER* buffer, Paket* data);
static uint8_t decodePaket(Paket* paket, uint8_t* data, uint8_t dataLenght);

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
		//sprintf(txt, "#%c\n", testPhaseChr);
#else
		//sprintf(txt, "Test progress #%c\n", testPhaseChr);
#endif
		//pushStr(USB_Tx_Buffer, txt, strlen(txt));
		Paket paket;
		fillPaket(&paket, TEST_PHASE_PAKET, &testPhaseChr, 1);
		pushPaket(USB_Tx_Buffer, &paket);
	}

	if(flags.meas.measComplete)
	{
		if(!flags.meas.calibMeas)
		{
			/*push(USB_Tx_Buffer, '#');
			if(testNum == 10)
				push(USB_Tx_Buffer, 126);
			else
				push(USB_Tx_Buffer, testNum);
			push(USB_Tx_Buffer, '\n');*/

			Paket paket;
			fillPaket(&paket, TEST_NUM_PAKET, &testNum, 1);
			pushPaket(USB_Tx_Buffer, &paket);
			testNum++;
		}
		else
		{
			flags.meas.calibMeas = 0;
		}

		if(flags.meas.onlyBattery)
		{
			/*char res[20] = {0};
			sprintf(res, "%d;\n", ADC_Results[12]);
			pushStr(USB_Tx_Buffer, res, strlen(res));*/

			makeByteArray();
			Paket paket;
			fillPaket(&paket, DATA_BAT_PAKET, &data[12], 2);
			pushPaket(USB_Tx_Buffer, &paket);
		}
		else
		{
			/*for(int i = 0; i < 7; i++)
			{
				char res[20];
				sprintf(res, "%d;", ADC_Results[2*i]);
				pushStr(USB_Tx_Buffer, res, strlen(res));
			}
			push(USB_Tx_Buffer, 0x0A);*/

			makeByteArray();
			Paket paket;
			fillPaket(&paket, DATA_PAKET, data, 14);
			pushPaket(USB_Tx_Buffer, &paket);
		}
	}

#ifdef __APP_COMPATIBILITY__
	if(flags.time.sec)
	{
		//char txt[] = {"#Hi\n"};
		//pushStr(USB_Tx_Buffer, txt, strlen(txt));

		Paket paket;
		fillPaket(&paket, REFRESH_PAKET, NULL, 0);
		pushPaket(USB_Tx_Buffer, &paket);
	}
#endif

	//___Odesílání dat___//
	//_Ošetření plného bufferu_//
	if(USB_Tx_Buffer->status == BUFFER_FULL)
	{
		flags.ui.shortBeep = 1;
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

//_____Zpracuje ADC_Results do pole data_____//
static void makeByteArray()
{
	for(int i = 0; i < 7; i++)
	{
		data[2*i] = MaskByte(ADC_Results[2*i], 0);
		data[2*i+1] = MaskByte(ADC_Results[2*i], 1);
	}
}

//_____Vytvoří strukturu Paket z dat v arcumentech_____//
static void fillPaket(Paket* paket, outPaketType type, uint8_t* data, uint8_t dataLength)
{
	paket->type = type;
	paket->data = data;
	paket->dataLength = dataLength;
	uint8_t CA = type;
	for(int i = 0; i < dataLength; i++)
		CA += data[i];
	paket->CA_value = CA;
}

//_____Vloží do bufferu řetězec odpovídající sestavenému paketu_____//
static void pushPaket(RING_BUFFER* buffer, Paket* paket)
{
	uint8_t msg[paket->dataLength + 2];
	msg[0] = paket->type;
	memcpy(msg+1, paket->data, paket->dataLength);
	msg[paket->dataLength+1] = paket->CA_value;
	//msg[paket->dataLength+2] = 0U;

	uint8_t str[] = {">>"};
	uint8_t end[] = {"<<\n"};

	uint8_t toSend[6+paket->dataLength + 2];
	//sprintf(toSend, "%s%s%s\n", str, msg, end);	//pro testNum = 0 se vytiskne jen paket->type, jelikož sprintf pak narazí na nulu
	memcpy(toSend, str, 2);
	memcpy(toSend+2, msg, paket->dataLength + 2);
	memcpy(toSend+2+paket->dataLength + 2, end, 4);
	pushStr(buffer, toSend, sizeof(toSend)-1);
}

//_____Příchozí řetězec přepracuje do struktury typu paket (pokud to lze)_____//
static uint8_t decodePaket(Paket* paket, uint8_t* data, uint8_t dataLenght)
{

}
