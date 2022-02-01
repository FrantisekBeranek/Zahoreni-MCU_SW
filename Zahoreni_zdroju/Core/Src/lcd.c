#include "lcd.h"

//_____Rozvítí podsvícení dané argumentem_____//
//-> argument: Barva podsvícení
void setColour(BACKLIGHT colour)
{
	switch(colour)
	{
	case BACKLIGHT_WHITE:
		HAL_GPIO_WritePin(BACKLIGHT_WHITE_GPIO_Port, BACKLIGHT_WHITE_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin, GPIO_PIN_RESET);
		break;

	case BACKLIGHT_GREEN:
		HAL_GPIO_WritePin(BACKLIGHT_WHITE_GPIO_Port, BACKLIGHT_WHITE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin, GPIO_PIN_SET);
		break;

	case BACKLIGHT_RED:
		HAL_GPIO_WritePin(BACKLIGHT_WHITE_GPIO_Port, BACKLIGHT_WHITE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin, GPIO_PIN_RESET);
		break;

	default:
		HAL_GPIO_WritePin(BACKLIGHT_WHITE_GPIO_Port, BACKLIGHT_WHITE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin, GPIO_PIN_RESET);
		break;
	}
}

//_____Přečte dostupnost displeje_____//
// !!! Pracuje v blokujícím módu !!!
DISP_STATE readBusy(void)
{
	uint8_t data[2] = {READ_BUSY_FLAG, 0};
	uint8_t data2[2];

	DISP_CS_ON;
	//_____Požadavek na čtení Busy Flag_____//
	//HAL_SPI_TransmitReceive(&hspi1, data, data2, 2, 100);
	//_____Čtení příchozího bytu_____//


	if(HAL_SPI_TransmitReceive(&hspi1, data, data2, 2, 100) == HAL_OK)
	{
		//_____Vrať hodnotu BF_____//
		DISP_CS_OFF;
		return (MaskBit(data2[1], 7))? SPI_BUSY : SPI_OK;
	}
	else
	{
		//_____Chyba čtení BF_____//
		DISP_CS_OFF;
		return SPI_ERR;
	}
}

//_____Pošle byte dat_____//
//-> argumenty: char - posílaný byte, Start_byte definuje zda jde o instrukci nebo data
// !!! Pracuje v blokujícím módu !!!
static DISP_STATE sendByte(char byte, START_BYTE type)
{
	/*while(readBusy() != SPI_OK)
	{
		if(readBusy() == SPI_ERR)
		{
			return SPI_ERR;
		}
	}*/
	uint8_t buffer[3];
	switch(type)
	{
		case INSTRUCTION:
			buffer[0] = 0xF8;
			break;
		case DATA:
			buffer[0] = 0xFA;
			break;
		default:	//neošetřené možnosti
			return SPI_ERR;
			break;
	}
	uint8_t tmp1 = 0U, tmp2 = 0U;
	for(uint8_t i = 0; i < 4; i++)
	{
		if(MaskBit(byte, i))
		{
			SetBit(tmp1, (7-i));
		}
		if(MaskBit(byte, (i+4)))
		{
			SetBit(tmp2, (7-i));
		}
	}
	buffer[1] = tmp1;
	buffer[2] = tmp2;

	DISP_CS_ON;
	HAL_StatusTypeDef ret = HAL_SPI_Transmit(&hspi1, buffer, 3, 100);
	DISP_CS_OFF;
	HAL_Delay(1);
	if(ret == HAL_OK)
		return DISP_OK;
	else
		return SPI_ERR;
}

//_____Provede nastavení proměných funkcí dle konfigurace_____//
//-> argument: ukazatel na proměnou s definováním konfigurace
//Nedokončená funkce
void setDispConfig(uint8_t* config)
{

}

//_____Provede reset displeje a defaultní nastavení_____//
// !!! Pracuje v blokujícím módu !!!
void dispInit(void)
{
	//_____Reset displeje po startu_____//
	HAL_Delay(20);
	HAL_GPIO_WritePin(DISP_RST_GPIO_Port,DISP_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(30);
	HAL_GPIO_WritePin(DISP_RST_GPIO_Port,DISP_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(5);

	//_____Nastavit parametry_____//
	sendByte(0x31, INSTRUCTION);	//Function set
	sendByte(0x01, INSTRUCTION);	//Clear display
	sendByte(0x13, INSTRUCTION);	//Oscilator
	sendByte(0x70, INSTRUCTION);	//Contrast
	sendByte(0x5C, INSTRUCTION);	//Power/Icon/Contrast
	sendByte(0x6B, INSTRUCTION);	//Follower control
	sendByte(0x0F, INSTRUCTION);	//Display on

	//_____Zapnout podsvícení_____//
	setColour(BACKLIGHT_WHITE);
}

//_____Nastaví kurzor_____//
//pozice počítána od nuly//
//-> argumenty: řádek, sloupec
// !!! Pracuje v blokujícím módu !!!
DISP_STATE setCursor(uint8_t row, uint8_t col)
{
	uint8_t addres = 0x80;	//DDRAM adresa
	if(row > 3 || col > 15)	//displej 4x16
		return DISP_ERR;
	addres += row*0x20;
	addres += col;

	sendByte(0x38, INSTRUCTION);	//Function set RE = 0
	HAL_Delay(5);
	DISP_STATE ret = sendByte(addres, INSTRUCTION);
	HAL_Delay(5);
	return ret;
}

//_____Zapiš znak na dané souřadnice_____//
//-> argumenty: znak k zobrazení, řádek, sloupec
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeChar(char character, uint8_t row, uint8_t col)
{
	DISP_STATE ret = setCursor(row, col);
	HAL_Delay(5);
	if(ret == DISP_OK)
	{
		//doplnit úpravu dat podle převodní tabulky displeje
		ret = sendByte(character, DATA);
		HAL_Delay(5);
		return ret;
	}
	else
		return ret;
}

//_____Zapiš řetězec na daný řádek_____//
//znaky přečnívající znaky budou smazány//
//-> argumenty: pole znaků, délka řetězce, řádek, zarovnání
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeRow(char* string, uint8_t lenght, uint8_t row, ALIGN align)
{
	if(lenght > 16)	//neplatná délka řetězce
		return DISP_ERR;
	uint8_t col;
	char newString[16];
	for(uint8_t i = 0; i < 16; i++)
	{
		newString[i] = ' ';
	}
	switch(align)
	{
		case LEFT:	//zarovnání doleva
			col = 0;
			break;
		case RIGHT:
			col = 15 - lenght;
			break;
		case CENTER:
			col = (15 - lenght)/2 + 1;
			break;
		default:
			break;
	}
	for(uint8_t i = 0; i < lenght; i++)
	{
		newString[col + i] = string[i];
	}
	for(uint8_t i = 0; i < 16; i++)
	{
		if(writeChar(newString[i], row, i) != DISP_OK)
					return SPI_ERR;
	}
	return DISP_OK;
}

//_____Zapiš řetězec na dané souřadnice_____//
//znaky přečnívající znaky budou zachovány//
//-> argumenty: pole znaků, délka, řádek, sloupec začátku
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeString(char* string, uint8_t lenght, uint8_t row, uint8_t col)
{
	if((lenght + col) > 16)
	{
		return DISP_ERR;
	}
	for(uint8_t i = 0; i < lenght; i++)
	{
		DISP_STATE ret = writeChar(string[i], row, (col + i));
		if(ret != DISP_OK)
			return ret;
	}
	return DISP_OK;
}

DISP_STATE clearRow(uint8_t row)
{
	char newString[16];
	for(uint8_t i = 0; i < 16; i++)
	{
		newString[i] = ' ';
	}
	return writeRow(newString, 16, row, LEFT);
}

//_____Řídí obsluhu displeje v neblokujícím režimu_____//
//Nedokončená funkce//
void dispHandler(void)
{

}
