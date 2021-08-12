/* USER CODE BEGIN Header */
/**
  @verbatim 
  Pins assignments for Nucleo 32 (stm32l031)
  PortA
  PA0  BUTTON/WKUP1 - Wake UP port
  PA1  BATTERY      - Battery charge ADC port (CH1)
  PA4  LED          - Application LED
  PA6  ESP_RST      - ESP reset control port
  PA7  ESP_PWR_ON   - ESP power control port
  PA2  USART2_TX    - Tx Debug&ESP comunication
  * PA15 USART2_RX    - Rx Debug comunication
  * PA3  USART2_RX    - Rx ESP comunication  
  PA13 SWDIO
  PA14 SWCLK
  
  PortB
  PB3  LD3          - Built in to Nucleo led
  
  Pins assignments for STM32l011(TSOP20)
  
   
  Pins assignments for STM32l031(TSOP20)
  PortA
  PA0  BUTTON/WKUP1 - Wake UP port
  PA1  BATTERY      - Battery charge ADC port (CH1)
  PA4  LED          - Application LED
  PA6  ESP_RST      - ESP reset control port
  PA7  ESP_PWR_ON   - ESP power control port
  PA2  USART2_TX    - Tx Debug&ESP comunication
  PA3  USART2_RX    - Rx ESP comunication  
  PA13 SWDIO
  PA14 SWCLK
   
  PortB
  PB3  LD3          - Built in to Nucleo led  
  
  @endverbatim   
*/
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
#include "globals.h"

#include "main.h"
#include "events.h"
#include "eventq.h"
#include "settings.h"
#include "appfwk.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/** \def    BLINK_LED
 *  \brief  Defines Nucleo32 board preinstalled LED3 ID    

    <Details>.
*/
#define BLINK_LED           0

/** \def    STATE_LED
 *  \brief  Defines Application logic LED ID
*/
#define STATE_LED           1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
/* ADC channel configuration structure declaration */
ADC_ChannelConfTypeDef	sConfig;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  
	/* USER CODE BEGIN 2 */

  /* App load config */
  loadSettings();

	/* Apps Initialization */
	fwkAppInit();
  
	fwkLedBlink( 0, 2, 20 );
	/* USER CODE END 2 */

  /* Infinite loop */
  fwkMain( 0x01 );
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV4;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

// ------------------------- ADC Begin ---------------------------------------
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
  hadc.Instance                   = ADC1;
  hadc.Init.OversamplingMode      = DISABLE;
  hadc.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc.Init.SamplingTime          = ADC_SAMPLETIME_1CYCLE_5;
  hadc.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc.Init.ContinuousConvMode    = DISABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc.Init.DMAContinuousRequests = DISABLE;
  hadc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  hadc.Init.Overrun               = ADC_OVR_DATA_PRESERVED;
  hadc.Init.LowPowerAutoWait      = DISABLE;
  hadc.Init.LowPowerFrequencyMode = DISABLE;
  hadc.Init.LowPowerAutoPowerOff  = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}


void setADCChanel( uint32_t cnl, uint32_t OnOff )
{
  if( OnOff )
  {
    /* On channel */ 
    sConfig.Channel = cnl; 
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }    
  }
  else
  {
    /* Off channel 1 */ 
    sConfig.Channel = cnl; 
    sConfig.Rank = ADC_RANK_NONE;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
      Error_Handler();  
    }
  }
}

void startADCConversion( void )
{
  /* Start the conversion process */  
  if (HAL_ADC_Start_IT(&hadc) != HAL_OK)
  {
    /* Start Conversation Error */
    Error_Handler();
  }  
}

void stopADCConversion( void )
{

}

/* Var for ADC Scan Event */
_tEQ eqAdcScan = { EV_ADC_SCAN, 00 };

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc1)
{

  uint32_t ADC_Data = HAL_ADC_GetValue(hadc1);
    /* Add adc scan event into queue */
  eqAdcScan.reserved =  ADC_Data;
  eqAddEvent( &eqAdcScan );
}
// ------------------------- ADC End ---------------------------------------
// ------------------------- RTC Begin ---------------------------------------
/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */
}

uint32_t blAlarmWasConfigured = 0;

/**
  * @brief  Alarm callback
  * @param  hrtc : RTC handle
  * @retval None
  */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	blAlarmWasConfigured = 0x00;
  AlarmNum = 'A';
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
  AlarmNum = 'B';
}


void hwSleepingInitiate(void)
{
	if( !blAlarmWasConfigured )
	{
		/* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
			mainly  when using more than one wakeup source this is to not miss any wakeup event.
			- Disable all used wakeup sources,
			- Clear all related wakeup flags, 
			- Re-enable all used wakeup sources,
			- Enter the Standby mode.
		*/
		/* Disable all used wakeup sources*/
		HAL_RTC_DeactivateAlarm( &hrtc, RTC_ALARM_A );
    HAL_RTC_DeactivateAlarm( &hrtc, RTC_ALARM_B );
		
		/* Clear all related STANDBY flags */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);	
	
		/* Set Alarm to almWeekday almHour:almMinute:almSeconds 
			 RTC Alarm Generation: Alarm on Hours, Minutes and Seconds */
	
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
   
		RTC_DateTypeDef 				sDateStructureGet;
    HAL_RTC_GetDate(&hrtc, &sDateStructureGet, RTC_FORMAT_BIN);	
   
    DBGT(LOG_DEBUG, "MN:RTC:%d:%d:%d", sTime.Hours, sTime.Minutes, sTime.Seconds );

    RTC_AlarmTypeDef sAlarm;

    // uint32_t  uiNextTimeInSeconds = sTime.Hours*3600 
    //                                +sTime.Minutes*60 
    //                                +sTime.Seconds
    //                                +uiBeacon;     
   
    uint32_t  uiNextTimeInSeconds = sTime.Hours*3600 
                                   +sTime.Minutes*60 
                                   +sTime.Seconds
                                   +15;      

    sAlarm.AlarmTime.Seconds  = uiNextTimeInSeconds%60;
    uint32_t uiTmp            = uiNextTimeInSeconds/60;
    sAlarm.AlarmTime.Minutes  =( uiTmp )%60;
    sAlarm.AlarmTime.Hours    = uiTmp/60;
    if( sAlarm.AlarmTime.Hours > 24  )
      sAlarm.AlarmTime.Hours = uiTmp%24;

    DBGT(LOG_DEBUG, "MN:RTC:NEWS:%d-%02d-%02d", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds );
   
    sAlarm.AlarmTime.TimeFormat     = RTC_HOURFORMAT12_AM;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;    
    sAlarm.AlarmMask                = RTC_ALARMMASK_DATEWEEKDAY;

    sAlarm.AlarmDateWeekDaySel      = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay         = 1;
    sAlarm.AlarmSubSecondMask       = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmTime.SubSeconds     = 0;

//    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.Alarm = RTC_ALARM_B;
    
    if(HAL_RTC_SetAlarm_IT(&hrtc,&sAlarm,FORMAT_BIN) != HAL_OK)
		{
			/* Initialization Error */
			Error_Handler(); 
		}
   
		blAlarmWasConfigured = 0x01; 	// TODO: It is not really needed - just to debug STANDBY

#if 1    
    DBGT( LOG_DEBUG, "MN:SLEEP");
    
		/* Enter the Standby mode */
		HAL_PWR_EnterSTANDBYMode();

#endif        
  }
}

// ------------------------- RTC End ---------------------------------------
// ------------------------- UART Begin ---------------------------------------
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART2 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance 										= USART2;
  huart2.Init.BaudRate 								= 9600;
  huart2.Init.WordLength 							= UART_WORDLENGTH_8B;
  huart2.Init.StopBits 								= UART_STOPBITS_1;
#ifdef DBG_UART_WEMOS  
  huart2.Init.Parity 									= UART_PARITY_NONE;
#else  
  huart2.Init.Parity 									= UART_PARITY_ODD;
#endif
  huart2.Init.Mode 										= UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl 							= UART_HWCONTROL_NONE;
  huart2.Init.OverSampling 						= UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling 					= UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit 	= UART_ADVFEATURE_NO_INIT;
	
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  /* USER CODE END USART2_Init 2 */
}

/* Var for UartRx Scan Event */
_tEQ eqUartRx = { EV_UART_RX, 00 };
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if(huart == &huart2) 
  {
    eqUartRx.reserved = pUartRxBuff[ 0 ];
    eqAddEvent( &eqUartRx );

    HAL_UART_Receive_IT (&huart2, pUartRxBuff, 1);
  }
}

// ------------------------- UART End ---------------------------------------

// ------------------------- GPIO Begin ---------------------------------------
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|ESP_RST_Pin|ESP_PWR_ON_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin 		= LED_Pin;
  GPIO_InitStruct.Mode 		= GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull 		= GPIO_NOPULL;
  GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
	
	setLedControl( 0 );

  /*Configure GPIO pin : ESP_RST_Pin */
  GPIO_InitStruct.Pin 		= ESP_RST_Pin;
  GPIO_InitStruct.Mode 		= GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull 		= GPIO_NOPULL;
  GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ESP_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ESP_PWR_ON_Pin */
  GPIO_InitStruct.Pin 		= ESP_PWR_ON_Pin;
  GPIO_InitStruct.Mode 		= GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull 		= GPIO_NOPULL;
  GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ESP_PWR_ON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD3_Pin */
  GPIO_InitStruct.Pin 		= LD3_Pin;
  GPIO_InitStruct.Mode 		= GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull 		= GPIO_NOPULL;
  GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WU_Pin */
  GPIO_InitStruct.Pin 		= WU_Pin;
  GPIO_InitStruct.Mode 		= GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull 		= GPIO_NOPULL;
  GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WU_GPIO_Port, &GPIO_InitStruct);  
}
/**
  * @brief  Provides a low level abstraction for ESP8266 reset GPIO.  
  *
  * @note   Used by service logic.
  *
  * @param  OnOff  0 ESP8266 resset is OFF. Others ESP8266 reset is ON.   
  * 
  * @retval None
  */
void setEspRstControl( uint8_t OnOff )
{
  if( OnOff == 0 )
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(ESP_RST_GPIO_Port, ESP_RST_Pin, GPIO_PIN_RESET);	
}
/**
  * @brief  Provides a low level abstraction for ESP8266 Power On GPIO.  
  *
  * @note   Used by service logic.
  *
  * @param  OnOff  0 LED not light. Others lidhts the LRD.      
  * 
  * @retval None
  */
void setEspPwrControl( uint8_t OnOff )
{
  if( OnOff == 0 )
    HAL_GPIO_WritePin(ESP_PWR_ON_GPIO_Port, ESP_PWR_ON_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(ESP_PWR_ON_GPIO_Port, ESP_PWR_ON_Pin, GPIO_PIN_RESET);	
}

/**
  * @brief  Provides a possibility to control application LED separately from app framework.  
  *
  * @note   Mostly used by application framework.
  *
  * @param  ledId  Led number used by application framework. 
  *                Could have BLINK_LED and STATE_LED values.
  * 
  * @param  state specifies the values 0 and 1 to ON ro OFF led device. 
  *  
  * @retval None
  */
void setLedControl( uint8_t OnOff )
{
  if( !OnOff )
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);	
}

/**
  * @brief  Provides a low level abstraction for LED devices used by framework layer 
  *         and application service logic layer.  
  *
  * @note   Mostly used by application framework.
  *
  * @param  ledId  Led number used by application framework. 
  *                Could have BLINK_LED and STATE_LED values.
  * 
  * @param  state specifies the values 0 and 1 to ON ro OFF led device. 
  *  
  * @retval None
  */
void setLedPinState( uint8_t ledId, uint8_t state )
{
  switch ( ledId )
  {
    case BLINK_LED:
      if( state == 1  )
        HAL_GPIO_WritePin( LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET );
      
      if( state == 0 )
        HAL_GPIO_WritePin( LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET );
			
			break;
			
		case STATE_LED:
      if( state == 0  )
        HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_SET );
      
      if( state == 1 )
        HAL_GPIO_WritePin( LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET );		
			
			break;
			
    default:
        break;
  }
}
// ------------------------- GPIO End --------------------------------------

/* USER CODE BEGIN 4 */

// ------- Eeprom -------------------------------------
#define EEPROM_START_ADDR 	((uint32_t)0x08080000)    /* Data EEPROM base address */
#define EEPROM_END_ADDR			((uint32_t)0x080803FF)    /* Data EEPROM end address */

#ifdef STM32L031xx
void hwkWriteBufferToEeprom( uint16_t eeprom_offset, uint8_t* pBuff, uint32_t size )
{
	uint32_t adr = EEPROM_START_ADDR +eeprom_offset;
	uint32_t eepromByte = 0;
  uint32_t idx = 0;

  /* Erasing EEPROM - could erase additional bytes 
    if size is not packed to word(4) */
  uint32_t uiEndErasedAdr = EEPROM_START_ADDR +eeprom_offset +size;
    
  if( size % sizeof(uint32_t))
    uiEndErasedAdr += 4;

  HAL_FLASHEx_DATAEEPROM_Unlock();
  
  while( adr < uiEndErasedAdr )
  {
    if( HAL_FLASHEx_DATAEEPROM_Erase( adr ) != HAL_OK )
      Error_Handler();   
    
    adr += 4;
  }
  
  /* EEPROM Programming */
  // assign to start
  adr = EEPROM_START_ADDR +eeprom_offset;
  
  for( idx = 0; idx<size; idx++ )
  {
    eepromByte = pBuff[ idx ];

    HAL_FLASHEx_DATAEEPROM_Program( FLASH_TYPEPROGRAMDATA_BYTE, adr++, eepromByte );  
  }
  HAL_FLASHEx_DATAEEPROM_Lock();   
}

void hwkReadBufferFromEeprom( uint16_t eeprom_offset, uint8_t* pBuf, uint32_t size) 
{
	uint32_t addr = EEPROM_START_ADDR +eeprom_offset;

	for( uint32_t i=0; i<size; i++ )
		pBuf[ i ]= (*(__IO uint8_t*)( addr +i ));
}
#endif 

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
