/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//nějaká změna
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
//extern	RING_BUFFER* dispBuffer;
//extern	RING_BUFFER* regBuffer;
extern	RING_BUFFER* USB_Rx_Buffer;
extern	RING_BUFFER* USB_Tx_Buffer;

volatile uint32_t ADC_Buffer[17][NUMBER_OF_SAMPLES];	//get samples of each channel
volatile uint32_t ADC_Results[17] = {0};

const uint32_t ADC_ChannelConf[] = 	{U15V_CHANNEL, U15V_CURRENT_CHANNEL,
		 	 	 	 	 	 	 	 U12V_CHANNEL, U12V_CURRENT_CHANNEL,
									 U24VO2_CHANNEL, U24VO2_CURRENT_CHANNEL,
									 U24V_CHANNEL, U24V_CURRENT_CHANNEL,
									 U5VK_CHANNEL, U5VK_CURRENT_CHANNEL,
									 U5V_CHANNEL, U5V_CURRENT_CHANNEL,
									 U_BAT_CHANNEL,
									 U48V_CURRENT_CHANNEL,
									 INTERNAL_REF_CHANNEL,
									 PAD9_CHANNEL, PAD15_CHANNEL};

//extern const uint8_t regCount;

//_____Proměnné �?asu_____//
volatile uint32_t sysTime[4] = {0};
/*
 * SYSTIME_TEN_MS	0
 * SYSTIME_SEC		1
 * SYSTIME_MIN		2
 * SYSTIME_HOUR		3
 */

//_____Bitové pole příznaků_____//
volatile Flags flags;

//_____Proměnné pro debouncing_____//
volatile uint8_t button0_Debounce = 0;
volatile uint8_t button1_Debounce = 0;

//_____Číslo zdroje k otestování_____//
volatile uint8_t supplyToTest = 0;

//_____Kalibra�?ní hodnota interní reference____//
uint16_t calibValue;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */
 static void clkHandler();
 static void buttonDebounce();
 static void dispHandler();
 static void UI_Handler();
 static void measHandler();
 static void calibHandler();
 static void ADC_dataProcessing();

//static uint32_t ADC_dataProcessing();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//_____Buttons interrupt callback_____//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == BUTTON_0_Pin)
	{
		flags.buttons.butt0_int = 1;
	}
	if(GPIO_Pin == BUTTON_1_Pin)
	{
		flags.buttons.butt1_int = 1;
	}
}

//_____Timer interrupt callback_____//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim14)	//Timer 14 -> každých 10 ms
	{
		//Krátké pípnutí signalizuje vykonávání hlavní smy�?ky programu
		//delší než deset ms
		if(flags.time.ten_ms == 1)
			flags.ui.shortBeep = 1;
		flags.time.ten_ms = 1;
	}
}

//_____ADC data ready callback_____//
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	HAL_ADC_Stop_IT(hadc);
	flags.meas.measDataReady = 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //__Buffery___//
  //dispBuffer = createBuffer(100);
  //regBuffer = createBuffer(100);
  USB_Rx_Buffer = createBuffer(500);
  USB_Tx_Buffer = createBuffer(500);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_ADC_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
  //___Nacteni kalibracni konstanty___//
  calibValue = *((uint16_t*)CALIB_VALUE_PTR);

  //___Inicializace displeje___//
  dispInit();
  char line1[] = "Zahoreni";
  char line2[] = "zdroju";
  writeRow(line1, 8, 1, CENTER);
  writeRow(line2, 6, 2, CENTER);

  LOAD_MIN_OFF;
  LOAD_MAX_OFF;

  if(regInit() != REG_OK)	//inicializace shift registrů
  {
	  flags.conErr = 1;
	  //Odešli zprávu do PC
  }

  // Start timer
  HAL_TIM_Base_Start_IT(&htim14);

  flags.ui.longBeep = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if(flags.time.ten_ms)	// 10 ms
	  {
		  clkHandler();
		  buttonDebounce();
		  comHandler();
		  calibHandler();
		  dispHandler();
		  UI_Handler();
		  testHandler();
		  measHandler();

	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_15;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */
  HAL_ADCEx_Calibration_Start(&hadc);
  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */
  DISP_CS_OFF;
  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 16-1;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 10000-1;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 38400;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, _5V_BAT_OFF_Pin|SR_CLR_Pin|SR_RCLK_Pin|SR_OE_Pin
                          |DISP_CS_Pin|DISP_RST_Pin|BACKLIGHT_GREEN_Pin|BACKLIGHT_WHITE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LOAD_MAX_Pin|LOAD_MIN_Pin|EM_HEATER_CTRL_Pin|HEATER_CTRL_Pin
                          |BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : _5V_BAT_OFF_Pin SR_CLR_Pin SR_RCLK_Pin SR_OE_Pin
                           DISP_CS_Pin DISP_RST_Pin BACKLIGHT_GREEN_Pin BACKLIGHT_WHITE_Pin */
  GPIO_InitStruct.Pin = _5V_BAT_OFF_Pin|SR_CLR_Pin|SR_RCLK_Pin|SR_OE_Pin
                          |DISP_CS_Pin|DISP_RST_Pin|BACKLIGHT_GREEN_Pin|BACKLIGHT_WHITE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : HEATER_STATE_Pin */
  GPIO_InitStruct.Pin = HEATER_STATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HEATER_STATE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LOAD_MAX_Pin LOAD_MIN_Pin EM_HEATER_CTRL_Pin HEATER_CTRL_Pin
                           BUZZER_Pin */
  GPIO_InitStruct.Pin = LOAD_MAX_Pin|LOAD_MIN_Pin|EM_HEATER_CTRL_Pin|HEATER_CTRL_Pin
                          |BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BUTTON_1_Pin BUTTON_0_Pin */
  GPIO_InitStruct.Pin = BUTTON_1_Pin|BUTTON_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BACKLIGHT_RED_Pin */
  GPIO_InitStruct.Pin = BACKLIGHT_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BACKLIGHT_RED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CONNECTION_ERR_Pin */
  GPIO_InitStruct.Pin = CONNECTION_ERR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CONNECTION_ERR_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

//_____Hodiny_____//
static void clkHandler(void)
{
	//___nulování všech flagů___//
	flags.time.ten_ms = 0;
	flags.time.sec	= 0;
	flags.time.min	= 0;
	flags.time.hour	= 0;

	sysTime[SYSTIME_TEN_MS]++;	//uplynulo dalších 10 ms

#ifdef __DEBUG_FAST__
	if((sysTime[SYSTIME_TEN_MS] % 10) == 0)	//0,1 s
#else
	if((sysTime[SYSTIME_TEN_MS] % 100) == 0)	//1 s
#endif
	{
		sysTime[SYSTIME_SEC]++;
		flags.time.sec = 1;
	}

	if(flags.time.sec)	//Uplynula 1 s
	{
		if((sysTime[SYSTIME_SEC] % 60) == 0 && sysTime[SYSTIME_TEN_MS] != 0)	//1 min
		{
			sysTime[SYSTIME_SEC] = 0;
			sysTime[SYSTIME_MIN]++;
			flags.time.min = 1;
		}

		if(flags.time.min)	//Uplynula 1 min
		{
			if((sysTime[SYSTIME_MIN] % 60) == 0 && sysTime[SYSTIME_TEN_MS] != 0)	//1 min
			{

				sysTime[SYSTIME_SEC] = 0;
				sysTime[SYSTIME_MIN] = 0;
				sysTime[SYSTIME_HOUR]++;
				flags.time.hour = 1;
				if(sysTime[SYSTIME_HOUR] >= 23)
					sysTime[SYSTIME_HOUR] = 0;
			}
		}
	}

#ifdef __DEBUG_TIME__
	if(flags.time.sec)
	{
		char timeStamp[30];
		sprintf(timeStamp, "%d : %d : %d\n", sysTime[SYSTIME_HOUR], sysTime[SYSTIME_MIN], sysTime[SYSTIME_SEC]);
		pushStr(USB_Tx_Buffer, timeStamp, strlen(timeStamp));	//odešli �?as
	}
#endif
}

//_____Debounce tla�?ítek_____//
static void buttonDebounce(void)
{
	//___nulování flagů___//
	flags.buttons.butt0_ver = 0;
	flags.buttons.butt1_ver = 0;

	if(flags.buttons.butt0_int)	//interrupt tla�?ítka 0
	{
		if(HAL_GPIO_ReadPin(BUTTON_0_GPIO_Port,BUTTON_0_Pin) == GPIO_PIN_SET)
		{
			button0_Debounce++;
		}
		else	//pin tla�?ítka na Low -> šlo o zákmit
		{
			button0_Debounce = 0;
			flags.buttons.butt0_int = 0;
		}
		if(button0_Debounce >= 5)	//pin tla�?ítka na High 5*10 ms -> ustálený stisk
		{
			flags.buttons.butt0_ver = 1;
			flags.buttons.butt0_int = 0;
			button0_Debounce = 0;

			flags.ui.active = 1;
#ifdef __DEBUG_BUTT__
			HAL_GPIO_TogglePin(BACKLIGHT_GREEN_GPIO_Port, BACKLIGHT_GREEN_Pin);
			//writeChar('a', 1, 5);
#endif
		}
	}

	if(flags.buttons.butt1_int)	//interrupt tla�?ítka 1
	{
		if(HAL_GPIO_ReadPin(BUTTON_1_GPIO_Port,BUTTON_1_Pin) == GPIO_PIN_SET)
		{
			button1_Debounce++;
		}
		else	//pin tla�?ítka na Low -> šlo o zákmit
		{
			button1_Debounce = 0;
			flags.buttons.butt1_int = 0;
		}
		if(button1_Debounce >= 5)	//pin tla�?ítka na High 5*10 ms -> ustálený stisk
		{
			flags.buttons.butt1_ver = 1;
			flags.buttons.butt1_int = 0;
			button1_Debounce = 0;

			flags.ui.active = 1;
#ifdef __DEBUG_BUTT__
			HAL_GPIO_TogglePin(BACKLIGHT_RED_GPIO_Port, BACKLIGHT_RED_Pin);
#endif
		}
	}
}

//_____Obsluha výtisků textu na displej_____//
static void dispHandler()
{
	char emptyString[] = "                ";
	char* strings[4] = {emptyString};
	ALIGN align[4] = {CENTER};

#ifdef __ADC_DEBUG__
	if(flags.meas.measComplete)
	{
		char ADC_value[10] = {0};
		sprintf(ADC_value, "ADC: %d", ADC_Results[U24VO2-1]);
		strings[3] = ADC_value;
		writeRow(strings[3], strlen(strings[3]), 3, align[3]);
	}
#endif

	if(flags.testProgress && !flags.instructions.stopRequest)
	{
		if(currentPhase() != WAITING)
		{
			char supplyInTestingNum[6];
			sprintf(supplyInTestingNum, "%d/%d", supplyToTest+1, regCount);

			strings[0] = supplyInTestingNum;
			align[0] = LEFT;
		}

		switch(currentPhase())
		{
		case START:
		{
			char start1[] = "Spousteni";
			strings[1] = start1;
			align[1] = CENTER;

			break;
		}
		case START_DONE:
		{
			char start1[] = "Spousteni";
			char start2[] = "dokonceno";
			strings[1] = start1;
			align[1] = CENTER;
			strings[2] = start2;
			align[2] = CENTER;
			break;
		}
		case MAIN_TEST:
		{
			char main1[] = "Hlavni test";
			strings[1] = main1;
			align[1] = CENTER;
			break;
		}
		case MAIN_TEST_DONE:
		{
			char main1[] = "Hlavni test";
			char main2[] = "dokoncen";
			strings[1] = main1;
			align[1] = CENTER;
			strings[2] = main2;
			align[2] = CENTER;
			break;
		}
		case BATTERY_TEST:
		{
			char bat1[] = "Test baterie";
			strings[1] = bat1;
			align[1] = CENTER;
			break;
		}
		case BATTERY_TEST_DONE:
		{
			char bat1[] = "Test baterie";
			char bat2[] = "dokoncen";
			strings[1] = bat1;
			align[1] = CENTER;
			strings[2] = bat2;
			align[2] = CENTER;
			break;
		}
		default:
		{
			char default1[] = "Zahoreni";
			char default2[] = "zdroju";
			strings[1] = default1;
			align[1] = CENTER;
			strings[2] = default2;
			align[2] = CENTER;

			break;
		}
		}

		for(int i = 0; i < 4; i++)
		{
			writeRow(strings[i], strlen(strings[i]), i, align[i]);
		}
	}

	//_____Zobrazení �?asu u hlavních testů_____//
	if(flags.time.sec)
	{
		switch(currentPhase())
		{
		case MAIN_TEST:
		{
			char time[9] = {0};
			sprintf(time, "%lu:%lu:%lu", 2-sysTime[SYSTIME_HOUR], 59-sysTime[SYSTIME_MIN], 59-sysTime[SYSTIME_SEC]);
			writeRow(time, strlen(time), 2, CENTER);
			break;
		}
		case BATTERY_TEST:
		{
			char time[9] = {0};
			sprintf(time, "%lu:%lu", 14-sysTime[SYSTIME_MIN], 59-sysTime[SYSTIME_SEC]);
			writeRow(time, strlen(time), 2, CENTER);
			break;
		}
		default:
			break;
		}
	}

	if(flags.instructions.stopRequest || flags.testCanceled)
	{
		char err[] = "Preruseni";
		strings[1] = err;

		for(int i = 0; i < 4; i++)
		{
			writeRow(strings[i], strlen(strings[i]), i, CENTER);
		}
	}
}

//_____Obsluha piezo + podsvícení displeje_____//
static void UI_Handler(void)
{
	//_____Vypínání podsvětlení displeje při ne�?innosti_____//
	/*static uint32_t startTime_LCD = 0;

	if(flags.testProgress)
		flags.ui.active = 1;
	if(flags.instructions.calibRequest || flags.instructions.startRequest || flags.instructions.stopRequest || flags.instructions.pauseRequest)
		flags.ui.active = 1;

	if(flags.ui.active)
	{
		startTime_LCD = sysTime[SYSTIME_TEN_MS];
		setColour(BACKLIGHT_WHITE);
	}

	if((sysTime[SYSTIME_TEN_MS] - startTime_LCD) >= 6000)	//1min
	{
		setColour(BACKLIGHT_OFF);
	}*/

	if(flags.conErr)
		flags.ui.error = 1;

	flags.ui.active = 0;

	static enum
	{
		OFF = 0U,
		SHORT_BEEP,
		LONG_BEEP,
		ERROR,
		NOTICE,
		DONE,
	}UI_State;

	static uint32_t startTime;	//proměnná pro �?asování dějů

	//___Nastavení stavu podle požadavků___//
	//___Stavy výše mají vyšší prioritu (error nejvyšší)___//
	if(flags.ui.error && (UI_State != ERROR))
	{
		UI_State = ERROR;
		startTime = sysTime[SYSTIME_TEN_MS];
	}
	else if(flags.ui.notice && (UI_State == OFF))
	{
		UI_State = NOTICE;
		startTime = sysTime[SYSTIME_TEN_MS];
		flags.ui.notice = 0;
	}
	else if(flags.ui.done && (UI_State == OFF))
	{
		UI_State = DONE;
		startTime = sysTime[SYSTIME_TEN_MS];
		flags.ui.done = 0;
	}
	else if(flags.ui.longBeep && (UI_State == OFF))
	{
		UI_State = LONG_BEEP;
		startTime = sysTime[SYSTIME_TEN_MS];
		flags.ui.longBeep = 0;
	}
	else if(flags.ui.shortBeep && (UI_State == OFF))
	{
		UI_State = SHORT_BEEP;
		startTime = sysTime[SYSTIME_TEN_MS];
		flags.ui.shortBeep = 0;
	}

	switch(UI_State)
	{
	case SHORT_BEEP:
#ifndef __SILENT__
			BUZZER_ON;
#endif
		if((sysTime[SYSTIME_TEN_MS] - startTime) >= 50)		//0,5s
		{
			UI_State = OFF;
			BUZZER_OFF;
		}
		break;

	case LONG_BEEP:
#ifndef __SILENT__
			BUZZER_ON;
#endif
		if((sysTime[SYSTIME_TEN_MS] - startTime) >= 100)	//1s
		{
			UI_State = OFF;
			BUZZER_OFF;
		}
		break;

	case ERROR:
		if(!flags.ui.error)	//dokud není požadavek zrušen provádí se error
			UI_State = OFF;
		if(!((sysTime[SYSTIME_TEN_MS] - startTime) % 50))	//každých 0,5s
		{
#ifndef __SILENT__
			BUZZER_Toggle;
#endif
			BACKLIGHT_RED_Toggle;
		}
		break;

	case NOTICE:
		if(!((sysTime[SYSTIME_TEN_MS] - startTime) % 35))	//každých 0,35s
		{
#ifndef __SILENT__
			BUZZER_Toggle;
#endif
		}
		if((sysTime[SYSTIME_TEN_MS] - startTime) >= 209)	//Po 2,1s ukon�?i
			UI_State = OFF;
		break;

	case DONE:
		if(!((sysTime[SYSTIME_TEN_MS] - startTime) % 50))	//každých 0,5s
		{
#ifndef __SILENT__
			BUZZER_Toggle;
#endif
			BACKLIGHT_GREEN_Toggle;
		}
		if((sysTime[SYSTIME_TEN_MS] - startTime) >= 299)	//Po 3s ukon�?i
			UI_State = OFF;
		break;

	default:	//Ošetřuje i UI_State == OFF
		BUZZER_OFF;
#ifndef __DEBUG_BUTT__
		//setColour(BACKLIGHT_OFF);
#endif
		break;

	}
}

//_____Osluha AD převodníků_____//
static void measHandler(void)
{
	static ADC_State_Type ADC_State;
	static uint8_t numOfSamples = 0;

	//___Nulování flagů___//
	flags.meas.measComplete = 0;
	flags.meas.measConflict = 0;

	if(flags.meas.measRequest)
	{
		if(!flags.meas.measRunning)
		{
			//hadc->Instance->CR = 0;	//Will disable ADC so calibrtion can start
			HAL_ADCEx_Calibration_Start(&hadc);	//Calibration process
			//hadc->Instance->CR = 1	//Enable ADC
			flags.meas.measRunning = 1;
			if(currentPhase() == BATTERY_TEST || currentPhase() == BATTERY_TEST_DONE)	//probíhá battery test
			{
				flags.meas.onlyBattery = 1;
				ADC_State = U_BAT;
			}
			else
			{
				flags.meas.onlyBattery = 0;
				ADC_State = U15V;
			}
			ADC1->CHSELR = ADC_ChannelConf[ADC_State-1];
			HAL_ADC_Start_IT(&hadc);
		}
		else
		{
			flags.meas.measConflict = 1;
		}
		flags.meas.measRequest = 0;
	}

	if(ADC_State != ADC_WAITING && flags.meas.measDataReady)
	{

		flags.meas.measDataReady = 0;

		ADC_Buffer[ADC_State-1][numOfSamples] = HAL_ADC_GetValue(&hadc);
		if(ADC_State == INTERNAL_REF)	//interni reference je vždy měřena jako poslední
		{
			numOfSamples++;
			if(numOfSamples == NUMBER_OF_SAMPLES)	//Naměřen dostatečný počet vzorků
			{
				ADC_dataProcessing();
				flags.meas.measComplete = 1;
				flags.meas.measRunning = 0;
				ADC_State = ADC_WAITING;
				numOfSamples = 0;
			}
			else
			{
				ADC_State = flags.meas.onlyBattery? U_BAT : U15V;
				ADC1->CHSELR = ADC_ChannelConf[ADC_State-1];
				HAL_ADC_Start_IT(&hadc);
			}
		}
		else
		{
			ADC_State += 2;	//Měř další kanál (měření proudů se přeskakuje)

			ADC1->CHSELR = ADC_ChannelConf[ADC_State-1];	//Nastav měřený kanál

			HAL_ADC_Start_IT(&hadc);
		}
	}
}

static void calibHandler()
{
	static uint32_t savedSec;

	if(flags.instructions.calibRequest)
	{
		sourceInTesting = &regValues[regCount - 1];

		for(int i = 0; i < regCount; i++)
		{
			regValues[i] = 0;
		}
		RELAY_ON(*sourceInTesting);	//připojit relé

		sendData();	//poslat konfiguraci shift registrům

		savedSec = sysTime[SYSTIME_SEC];

		flags.instructions.calibRequest = 0;
		flags.calibRunning = 1;


	}
	if(flags.calibRunning)
	{
		static uint8_t lock = 0;
		if((sysTime[SYSTIME_SEC] >= savedSec + 3) & !lock)
		{
			flags.meas.measRequest = 1;
			flags.meas.calibMeas = 1;
			lock = 1;
		}
		if(flags.instructions.calibDone)
		{
			for(int i = 0; i < regCount; i++)
			{
				regValues[i] = 0;
			}

			sendData();	//poslat konfiguraci shift registrům
			flags.calibRunning = 0;
			flags.instructions.calibDone = 0;
		}
	}
}

//_____Zpracování naměřených dat_____//
static void ADC_dataProcessing()
{
	uint32_t max[17] = {0};
	uint32_t min[17] = {ADC_MAX};

	uint8_t max_pos[17];
	uint8_t min_pos[17];
	//Nalezni max a min naměřenou hodnotu a jejich pozice
	for(uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		for(uint8_t j =0; j < 17; j++)
		{
			if(ADC_Buffer[j][i] > max[j])
			{
				max[j] = ADC_Buffer[j][i];
				max_pos[j] = i;
			}
			if(ADC_Buffer[j][i] < min[j])
			{
				min[j] = ADC_Buffer[j][i];
				min_pos[j] = i;
			}
		}
	}
	//Udělej průměr z naměřených hodnot kromě max a min
	for(uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++)
	{
		for(uint8_t j =0; j < 17; j++)
		{
			if(i != max_pos[j] && i != min_pos[j])
				ADC_Results[j] += ADC_Buffer[j][i];
		}
	}
	for(uint8_t j =0; j < 17; j++)
	{
		ADC_Results[j] /= 17;
	}

	return;
	//return ADC_Buffer[10];
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
