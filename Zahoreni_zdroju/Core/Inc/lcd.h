/*
 * lcd.h
 *
 *  Created on: 31. 8. 2021
 *      Author: beranekf
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f0xx_hal.h"
#include "ringBuffer.h"
#include <main.h>

//_____Chip Select_____//
#define DISP_CS_ON HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_RESET)
#define DISP_CS_OFF HAL_GPIO_WritePin(DISP_CS_GPIO_Port, DISP_CS_Pin, GPIO_PIN_SET)


//_____Instrukce displeje_____//
#define READ_BUSY_FLAG 0b11111100
#define WRITE_INSTRUCTION 0b11111000
#define WRITE_DATA 0b11111010

//_____Enum pro volbu prvního bytu_____//
typedef enum
{
	READ_BUSY = 0U,
	INSTRUCTION,
	DATA,
	DATA_WRITE
}START_BYTE;

//_____Stav displeje_____//
typedef enum
{
	DISP_OK = 0U,
	SPI_OK,
	SPI_ERR,
	SPI_BUSY,
	DISP_ERR,
	DISP_SENDING
}DISP_STATE;

DISP_STATE dispState;

//_____Volba zarovnání řetězce na displeji_____//
typedef enum
{
	LEFT = 0U,
	RIGHT,
	CENTER
}ALIGN;

//_____BUFFER_____//
RING_BUFFER* dispBuffer;

/*
#define CLEAR_DISP 0b00000001


uint8_t FUNCTION_SET = 0b00100000;
	//---argumenty funkce Function_set---//
	#define DL 4	//Data lenght -> H = 8-bit, L = 4bit
	#define N 3		//Number of rows H = 1/3 řádky, L = 2/4 řádky podle NW
	#define DH 2	//Double height font !Pro RE = 0
	#define BE 2	//Blink enable !Pro RE = 1
	#define RE 1	//Extender register enable
	#define IS 0	//Special register enable !Pro RE = 0
	#define REV 0	//Reverse enable !Pro RE = 1

uint8_t EXTENDED_FUNCTION_SET = 0b00001000;
//---argumenty funkce pro RE = 0---//
	#define D 2		//Displej ON/OFF
	#define C 1		//Cursor ON/OFF
	#define B 0		//Cursor blink ON/OFF
//---argumenty funkce pro RE = 1---//
	#define FW 2	//Font width -> H = 6 dots, L = 5 dots
	#define B/W 1	//Black/White inversion at cursor
	#define NW 0	//4 line mode enable -> H = 3/4 řádky, L = 1/2 řádky

uint8_t ENTRY_MODE_SET = 0b00000100;
//---argumenty funkce pro RE = 0---//
	#define I/D 1	//Increment/Decrement kursoru a DDRAM adresy -> H = inc, L = dec
	#define S 0		//Kdo ví co to dělá
//---argumenty funkce pro RE = 1---//
	#define BDC 1	//BOTTOM/TOP view
	#define BDS 0	//Zrdcadlení

uint8_t DOUBLE_HEIGHT = 0b00010000;
//---argumenty funkce pro IS = 0, RE = 1---//
	#define UD2 3	//Pro DH = 1 nastaví double height daného řádku
	#define UD1 2	//=00 -> poslední řádek zdvojený, = 01 -> prostřední řádek
					//=10 -> dva zdvojené řádky, =11 ->první řádek zdvojený
*/


extern SPI_HandleTypeDef hspi1;

//_____Podsvícení_____//
typedef enum
{
	BACKLIGHT_WHITE = 0U,
	BACKLIGHT_GREEN,
	BACKLIGHT_RED,
	BACKLIGHT_OFF
}BACKLIGHT;

//_____Konfigurace displeje_____//
uint32_t dispConfig;	//buď přes bitíky nebo struktura

//_____Přečte dostupnost displeje_____//
// !!! Pracuje v blokujícím módu !!!
DISP_STATE readBusy(void);

//_____Pošle byte dat_____//
//-> argumenty: char - posílaný byte, Start_byte definuje zda jde o instrukci nebo data
// !!! Pracuje v blokujícím módu !!!
//static DISP_STATE sendByte(char, START_BYTE);

//_____Rozvítí podsvícení dané argumentem_____//
//-> argument: Barva podsvícení
void setColour(BACKLIGHT);

//_____Vrátí barvu displeje nastavenou ponocí setColour()_____//
BACKLIGHT getColour();

//_____Provede nastavení proměných funkcí dle konfigurace_____//
//-> argument: ukazatel na proměnou s definováním konfigurace
void setDispConfig(uint8_t*);

//_____Provede reset displeje a defaultní nastavení_____//
// !!! Pracuje v blokujícím módu !!!
void dispInit(void);

//_____Nastaví kurzor_____//
//pozice počítána od nuly//
//-> argumenty: řádek, sloupec
// !!! Pracuje v blokujícím módu !!!
DISP_STATE setCursor(uint8_t, uint8_t);

//_____Zapiš znak na dané souřadnice_____//
//-> argumenty: znak k zobrazení, řádek, sloupec
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeChar(char, uint8_t, uint8_t);

//_____Zapiš řetězec na daný řádek_____//
//znaky přečnívající znaky budou smazány//
//-> argumenty: pole znaků, délka řetězce, řádek, zarovnání
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeRow(char*, uint8_t, uint8_t, ALIGN);

//_____Zapiš řetězec na dané souřadnice_____//
//znaky přečnívající znaky budou zachovány//
//-> argumenty: pole znaků, délka, řádek, sloupec začátku
// !!! Pracuje v blokujícím módu !!!
DISP_STATE writeString(char*, uint8_t, uint8_t, uint8_t);

//_____Smaže řádek daný argumentem_____//
DISP_STATE clearRow(uint8_t);

//_____Řídí obsluhu displeje v neblokujícím režimu_____//
//Nedokončená funkce//
void lcdHandler(void);

#endif /* INC_LCD_H_ */
