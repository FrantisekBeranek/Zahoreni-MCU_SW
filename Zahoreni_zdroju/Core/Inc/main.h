/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdlib.h"
#include "string.h"
#include "ringBuffer.h"
#include "lcd.h"
#include "shiftReg.h"
#include "testHandle.h"
#include "comHandle.h"
#include "ADC_defines.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* Bitové pole flagů */
typedef struct{
	//___TIME___//
	struct{
		unsigned int ten_ms			: 1;
		unsigned int sec			: 1;
		unsigned int min			: 1;
		unsigned int hour			: 1;
	}time;

	//___BUTTONS___//
	struct{
		unsigned int butt0_int		: 1;	//butt0 interrupt
		unsigned int butt0_ver		: 1;	//butt0 verified
		unsigned int butt1_int		: 1;	//butt1 interrupt
		unsigned int butt1_ver		: 1;	//butt1 verified
	}buttons;

	//___COMUNICATION___//
	unsigned int data_received		: 1;	//byla přijata data

	//___INSTRUCTIONS___//
	struct{
		unsigned int startRequest	: 1;	//požadavek na start testu
		unsigned int stopRequest	: 1;	//požadavek na zrušení testu
		unsigned int pauseRequest	: 1;	//požadavek na pozastavení testu
		unsigned int calibRequest	: 1;	//požadavek na zaslání kalibračních dat
		unsigned int unknownInst	: 1;	//Neznámá instrukce
	}instructions;

	//___UI___//
	struct{
		unsigned int shortBeep		: 1;
		unsigned int longBeep		: 1;
		unsigned int error			: 1;
		unsigned int notice			: 1;
		unsigned int done			: 1;
	}ui;

	//___VOLTAGE MEASUREMENT___//
	struct{
		unsigned int measRequest	: 1;	//Požadavek naprovedení měření
		unsigned int measComplete	: 1;	//Měření dokončeno
		unsigned int measDataReady	: 1;	//Měření dokončeno a data připravena k odeslání
		unsigned int measRunning	: 1;	//Měření probíhá
		unsigned int measConflict	: 1;	//Dva požadavky na měření najednou
		unsigned int onlyBattery	: 1;	//Měřena pouze baterie
		unsigned int calibMeas		: 1;	//Kalibrační měření
	}meas;

	//___TEST CONTROL___//
	unsigned int startConflict		: 1;	//Dva požadavky na start najednou
	unsigned int testProgress		: 1;	//Fáze testu se změnila

	//___SHIFT REGISTERS___//
	unsigned int conErr				: 1;	//Chyba připojení shift registrů

} Flags;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

//_____Funkce pro řízení časování_____//
/* Nastavuje flagy ve struktuře time */
void clkHandler(void);

//_____Funkce pro debouncing tlačítek_____//
/* Nastavuje flagy ve struktuře buttons */
void buttonDebounce(void);

//_____Funkce pro obsluhu uživatelského rozhraní (buzzer a podsvícení displeje)_____//
/* �?ídí se pomocí nastavení flagů struktury ui */
void UI_Handler(void);

//_____Funkce pro řízení ADC převodníků_____//
/* �?ídí se pomocí flagů measRequest a onlyBat struktury meas */
/* Zbylé flagy struktury meas nastavuje */
void measHandler(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define _5V_BAT_OFF_Pin GPIO_PIN_2
#define _5V_BAT_OFF_GPIO_Port GPIOB
#define DEBUG_Pin GPIO_PIN_12
#define DEBUG_GPIO_Port GPIOB
#define SR_CLR_Pin GPIO_PIN_13
#define SR_CLR_GPIO_Port GPIOB
#define SR_RCLK_Pin GPIO_PIN_14
#define SR_RCLK_GPIO_Port GPIOB
#define SR_OE_Pin GPIO_PIN_15
#define SR_OE_GPIO_Port GPIOB
#define LOAD_MAX_Pin GPIO_PIN_6
#define LOAD_MAX_GPIO_Port GPIOC
#define LOAD_MIN_Pin GPIO_PIN_7
#define LOAD_MIN_GPIO_Port GPIOC
#define EM_HEATER_CTRL_Pin GPIO_PIN_8
#define EM_HEATER_CTRL_GPIO_Port GPIOC
#define HEATER_CTRL_Pin GPIO_PIN_9
#define HEATER_CTRL_GPIO_Port GPIOC
#define BUTTON_1_Pin GPIO_PIN_8
#define BUTTON_1_GPIO_Port GPIOA
#define BUTTON_1_EXTI_IRQn EXTI4_15_IRQn
#define BUTTON_0_Pin GPIO_PIN_9
#define BUTTON_0_GPIO_Port GPIOA
#define BUTTON_0_EXTI_IRQn EXTI4_15_IRQn
#define USB_VBUS_Pin GPIO_PIN_10
#define USB_VBUS_GPIO_Port GPIOA
#define BACKLIGHT_RED_Pin GPIO_PIN_15
#define BACKLIGHT_RED_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_12
#define BUZZER_GPIO_Port GPIOC
#define CONNECTION_ERR_Pin GPIO_PIN_2
#define CONNECTION_ERR_GPIO_Port GPIOD
#define DISP_CS_Pin GPIO_PIN_6
#define DISP_CS_GPIO_Port GPIOB
#define DISP_RST_Pin GPIO_PIN_7
#define DISP_RST_GPIO_Port GPIOB
#define BACKLIGHT_GREEN_Pin GPIO_PIN_8
#define BACKLIGHT_GREEN_GPIO_Port GPIOB
#define BACKLIGHT_WHITE_Pin GPIO_PIN_9
#define BACKLIGHT_WHITE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* Makra pro snazší debug */
//#define __DEBUG_TIME__			//Posílání zpravy s časem od zapnutí
//#define __DEBUG_BUTT__			//Tlačítka mění podsvícení displeje
//#define __DEBUG_INST__			//Po přijetí instrukce posílá řetězec zprávu o vyhodnocení
//#define __SILENT__				//Zakazuje pípání
#define __DEBUG_TEST__				//Test běží v zkáceném režimu
//#define __DEBUG_FAST__			//Čas je desetkrát zrychlen
#define __APP_COMPATIBILITY__		//Spouští posílání pravidelné zprávy

/* Prace s bitovými proměnnými */
#define SetBit(x,y) x|=(1<<y)			//nastav bit y bytu x
#define ClearBit(x,y) x&=~(1 << y)		//vynuluj bit y bytu x
#define NegBit(x,y) x^=(1 << y)			//neguj bit y bytu x
#define MaskBit(x,y) x&(1 << y)			//vymaskuj but y bytu x

#define MaskByte(x,y) (x >> y*8) & 0xFF	//vymaskuj byte y proměnné x

/* �?ízení zátěží */
#define LOAD_MIN_ON HAL_GPIO_WritePin(LOAD_MIN_GPIO_Port, LOAD_MIN_Pin, GPIO_PIN_SET)
#define LOAD_MIN_OFF HAL_GPIO_WritePin(LOAD_MIN_GPIO_Port, LOAD_MIN_Pin, GPIO_PIN_RESET)
#define LOAD_MAX_ON HAL_GPIO_WritePin(LOAD_MAX_GPIO_Port, LOAD_MAX_Pin, GPIO_PIN_SET)
#define LOAD_MAX_OFF HAL_GPIO_WritePin(LOAD_MAX_GPIO_Port, LOAD_MAX_Pin, GPIO_PIN_RESET)

/* Piezo */
#define BUZZER_ON HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET)
#define BUZZER_OFF HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET)
#define BUZZER_Toggle HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin)

/* Blikání podsvícení displeje */
#define BACKLIGHT_RED_Toggle HAL_GPIO_TogglePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin)
#define BACKLIGHT_GREEN_Toggle HAL_GPIO_TogglePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin)

/* Hodnoty pro práci s polem sysTime */
#define SYSTIME_TEN_MS	0
#define SYSTIME_SEC		1
#define SYSTIME_MIN		2
#define SYSTIME_HOUR	3

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
