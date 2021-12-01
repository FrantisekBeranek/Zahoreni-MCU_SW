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
extern int testNum;	//udává pořadí odesílaných dat

//___Pole pro převod dat převodníku na pole bytů___//
uint8_t data[14];

/* definice funkcí */
void comHandler(void);
static void makeByteArray();
static void fillPaket(Paket* paket, outPaketType type, uint8_t* data, uint8_t dataLength);
static void pushPaket(RING_BUFFER* buffer, Paket* data);
static uint8_t decodePaket(/*Paket* paket, */uint8_t* data, uint8_t dataLenght);

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
	//___Ošetření plného bufferu___//
	if(USB_Rx_Buffer->status == BUFFER_FULL)
	{
		flags.data_received = 0;
		clearBuffer(USB_Rx_Buffer);
	}

	//___Příjem dat___//
	if(flags.data_received)
	{
		int start = 0;	//flag o nalezení počátku paketu

		for(int i = 0; i < USB_Rx_Buffer->filled; i++)	//Projdi celou obsazenou část bufferu
		{
			//Přečti znaky na pozici i a i+1
			char tmp1, tmp2;
			at(USB_Rx_Buffer, i, &tmp1);
			at(USB_Rx_Buffer, i+1, &tmp2);

			if(tmp1 == '>' && tmp1 == '>')	//začátek paketu
			{
				for(int y = 0; y < i; y++)	//vymazání obsahu buuferu před začátkem paketu (neplatná data)
				{
					char tmp;
					pop(USB_Rx_Buffer, &tmp);
				}
				start = 1;	//nastav flag o nalezení počátku
				break;
			}
		}

		if(start)	//počátek byl nalezen
		{
			for(int i = 0; i < USB_Rx_Buffer->filled; i++)	//Projdi celou obsazenou část bufferu
			{
				//Přečti znaky na pozici i a i+1
				char tmp1, tmp2;
				at(USB_Rx_Buffer, i, &tmp1);
				at(USB_Rx_Buffer, i+1, &tmp2);

				if(tmp1 == '<' && tmp1 == '<')	//konec paketu
				{
					uint8_t* tmp = (uint8_t*)malloc((i+2)*sizeof(uint8_t));
					for(int y = 0; y < i+2; y++)	//překopírování zprávy
					{
						pop(USB_Rx_Buffer, &tmp[y]);
					}

					decodePaket(tmp, i+2);
					free(tmp);
					break;
				}
			}
		}

		flags.data_received = 0;
	}

	if(flags.testProgress)	//Pokud test pokročil...
	{
		//...zjisti v jaké je fázi...
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

		//...a upozorni na to PC
		Paket paket;
		fillPaket(&paket, TEST_PHASE_PAKET, &testPhaseChr, 1);
		pushPaket(USB_Tx_Buffer, &paket);
	}

	if(flags.meas.measComplete)	//Jsou připravena data k odeslání
	{
		if(!flags.meas.calibMeas)
		{
			//Nejde o kalibrační data -> pošli číslo dat
			Paket paket;
			fillPaket(&paket, TEST_NUM_PAKET, &testNum, 1);
			pushPaket(USB_Tx_Buffer, &paket);
			testNum++;
		}
		else
		{
			flags.meas.calibMeas = 0;
		}

		//___Připrav a odešli paket___//
		makeByteArray();
		Paket paket;
		outPaketType type = (flags.meas.onlyBattery)? DATA_BAT_PAKET : DATA_PAKET;
		fillPaket(&paket, type, data, 14);
		pushPaket(USB_Tx_Buffer, &paket);
	}

#ifdef __APP_COMPATIBILITY__
	//___Odesílání refresh zprávy___//
	if(flags.time.sec)
	{
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
		clearBuffer(USB_Tx_Buffer);
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
		data[2*i] = MaskByte(ADC_Results[2*i], 1);
		data[2*i+1] = MaskByte(ADC_Results[2*i], 0);
	}
}

//_____Vytvoří strukturu Paket z dat v argumentech_____//
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
static uint8_t decodePaket(/*Paket* paket,*/ uint8_t* data, uint8_t dataLenght)
{
	int sum = 0;
	for(int i = 2; i < dataLenght - 3; i++)
	{
		sum += data[i];
	}
	if(sum == data[dataLenght - 3])	//kontorlní součet odpovídá
	{
		switch(data[2])	//Na třetím místě je instrukce
		{
		case 's': ;
			//___Start testu___//
			if(dataLenght == 7)
			{
				flags.instructions.startRequest = 1;
#ifdef __DEBUG_INST__
				sprintf(txt, "Start\n");
				pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
				//Na pozici data je ukazatel na testovaný zdroj
			}
			break;

		case'c': ;
			//___Ukončení___//
		if(dataLenght == 7)
		{
			flags.instructions.stopRequest = 1;
#ifdef __DEBUG_INST__
			sprintf(txt, "Ukonceni\n");
			pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
			//Na pozici data je ukazatel na testovaný zdroj
		}
			break;

		case'p': ;
			//___Pauza___//
		if(dataLenght == 7)
		{
			flags.instructions.pauseRequest = 1;
#ifdef __DEBUG_INST__
			sprintf(txt, "Pauza\n");
			pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
			//Na pozici data je ukazatel na testovaný zdroj
		}
			break;

		case'k': ;
			//___Kalibrace___//
		if(dataLenght == 7)
		{
			flags.instructions.calibRequest = 1;
#ifdef __DEBUG_INST__
			sprintf(txt, "Kalibrace\n");
			pushStr(USB_Tx_Buffer, txt, strlen(txt));
#endif
			//Na pozici data je ukazatel na testovaný zdroj
		}
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
	return 1;
}
