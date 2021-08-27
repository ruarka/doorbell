/*! \file      appbtfms.c
    \version   0.1
    \date      2017-07-18 18:25
    \brief     Watering Application 
    \copyright  
    \author    ruarka
		\details  
		

		Bell Button App State sequence

		State     ->  	Possible Transitions
	
		\bug
*/
/* ------------------------------------------------------------------------------------------------
 *                                 Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>
#include "globals.h"

#include "events.h"
#include "eventq.h"
#include "settings.h"
#include "appfwk.h"
#include "main.h"
#include "appbtfms.h"
#include "lkuptrg.h"


/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
 
/* ------------------------------------------------------------------------------------------------
 *                                 Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                 Local Variables
 * ------------------------------------------------------------------------------------------------
 */

uint16_t  uiStopWtSamples;
uint32_t  uiBtFsmAppState     = BT_IDLE;
uint32_t  uiADCSum;
uint8_t   uiADCSamlesNum; 
uint32_t  uiBatteryPercentage;
uint32_t  uiAdcTO;
uint32_t  uiRstTO;
uint32_t  uiBeaconRecvTO;
uint32_t  uiMqttWaitRespTO;
uint32_t  uiErrorTO;
uint32_t  uiModeBeforeEnteringState;
uint32_t  uiBellMode = 0;

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t setBtFsmAppState( uint32_t newState );

static void updateBellModeForBell( void )
{
  uint8_t Pa0PinVal = HAL_GPIO_ReadPin( WU_GPIO_Port, WU_Pin );

  if( HAL_GPIO_ReadPin( WU_GPIO_Port, WU_Pin ))
    if( uiBellMode == modeBeacon ) // Beacon mode
    { 
      uiBellMode = modeBell;      // Bell Mode
    }
}

/* ------------------------------------------------------------------
   IDLE state 
   ------------------------------------------------------------------ */
/* State IDLE Event Handlers */
static uint8_t btFsmIdleStateEh(_tEQ* p)
{
  switch( p->eId )
  {
    case EV_SYS_TICK:
      // Move->BT_CHECK_MODE
      setBtFsmAppState( BT_CHECK_MODE ); 
      return 0;

    case EV_APP_IDLE:
    case EV_APP_INIT:
    case EV_ADC_SCAN:
    case EV_UART_RX:
      break;

    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Check Mode state 
   ------------------------------------------------------------------ */
/* State IDLE Event Handlers */
static uint8_t btFsmCheckModeStateEH(_tEQ* p)
{
  switch( p->eId )
  {
    case EV_SYS_TICK:
      updateBellModeForBell();
      setBtFsmAppState( BT_ESP_PWRON ); 
      break;

    case EV_APP_IDLE:
    case EV_APP_INIT:
    case EV_ADC_SCAN:
    case EV_UART_RX:
      break;

    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   POWER ON state 
   ------------------------------------------------------------------ */
uint8_t   uiBtWaitPwrOnSeconds  = 0;

static uint8_t btfsmESPPwrOnStateEh(_tEQ* p)
{
  switch (p->eId)
  {
    case EV_APP_IDLE:
    case EV_APP_INIT:
      setBtFsmAppState( BT_IDLE );
      break;

    case EV_UART_RX:
      break;

    case EV_ADC_SCAN:
      return 0;

    case EV_SYS_TICK:
      updateBellModeForBell();
      if( uiBtWaitPwrOnSeconds >= ESP_PWRON_STAB_TIME )
        setBtFsmAppState( BT_ADC_BATTERY_CONTROL );
      else{
        uiBtWaitPwrOnSeconds += MILISECONDS_PER_SYSTICK;
        return 0;
      } 
      break;

    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Check Battery Power state 
   ------------------------------------------------------------------ */
uint8_t uiMappedADCSumWt  = 255;

// 
static uint8_t btfsmCheckBatteryStateEh(_tEQ* p)
{
  switch (p->eId)
  {
    case EV_APP_IDLE:
    case EV_APP_INIT:
      setBtFsmAppState( BT_IDLE );
      break;

    case EV_ADC_SCAN:
      if( uiADCSamlesNum++ >= BT_ADC_SAMPLES ){
        // TODO: Calculate a Real percentage value
        uiADCSum /= BT_ADC_SAMPLES;

        DBGT(LOG_DEBUG, "\nd[ADC:%X]", uiADCSum);

        uiBatteryPercentage = 75;

        setBtFsmAppState( BT_ESP_RESET_LEAVE );
      }else{
        uiADCSum += p->reserved;
        startADCConversion();
      }
      break;

    case EV_SYS_TICK:
      updateBellModeForBell();

      if( uiAdcTO > BT_ADC_TO ){
        setBtFsmAppState( BT_ESP_RESET_LEAVE );
        uiBatteryPercentage = 0;
      }else{
        uiAdcTO += MILISECONDS_PER_SYSTICK;
      }
      break;

    case EV_UART_RX:
    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Wait TO after Reset release state 
   ------------------------------------------------------------------ */
static uint8_t btfsmESPResetOffStateEH(_tEQ* p)
{
  switch (p->eId)
  {
    case EV_APP_IDLE:
    case EV_APP_INIT:
      setBtFsmAppState( BT_IDLE );
      break;

    case EV_SYS_TICK:
      updateBellModeForBell();

      if( uiRstTO > BT_RST_LEAVE_TO  ){
        setBtFsmAppState( BT_BEACON_CFG_RECV );
      }else{
        uiRstTO += MILISECONDS_PER_SYSTICK;
      }
      break;

    case EV_ADC_SCAN:
    case EV_UART_RX:

    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Wait Beacon Value state 
   ------------------------------------------------------------------ */
token_trigger_t trg;
char pRespPattern[]={ 'b', '[', (char)0xff, ']' };
//char pRespPattern[]={ "Bla" };
char pResultBuff[ 15 ];

static uint8_t  btfsmBeaconWaitStateEH(_tEQ* p )
{
  switch (p->eId)
  {
    case EV_SYS_TICK:
      updateBellModeForBell();

      if( uiBeaconRecvTO > BT_BEACON_CFG_TO ){
        setBtFsmAppState( BT_ERR );
      }else{
        uiBeaconRecvTO += MILISECONDS_PER_SYSTICK;
      }
      break;

    case EV_APP_IDLE:
    case EV_APP_INIT:
      setBtFsmAppState( BT_IDLE );
      break;

    case EV_UART_RX:
		{
      char ch =  p->reserved;
      if( sniffByTemplate(( uint8_t )ch, &trg ))
      {
        trg.pResult[ trg.resultLen ]= 0;
				DBGT(LOG_DEBUG, "\nFSM:beacon[%s]", trg.pResult);
				
        uiBeacon = my_atoi(( char* )trg.pResult, trg.resultLen );
        if(( uiBeacon < 5 )&&( uiBeacon > 86400 ))
          uiBeacon = 20; 

				setBtFsmAppState( BT_MODE_SEND_WAIT_MQTT_RES );
       }else{
        /* code */
        HAL_UART_Receive_IT (&huart2, pUartRxBuff, 1);
      }
		}   
    case EV_ADC_SCAN:
    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Send Mode state 
   ------------------------------------------------------------------ */
char pOkPattern[]={ 'b', '[', 'O','k',']' };
char pErrPattern[]={ 'b', '[', 'E','r','r',']' };
token_trigger_t trgE;

static uint8_t btfsmSendModeWaitMqttRespStateEh(_tEQ* p)
{
  switch (p->eId)
  {
    case EV_SYS_TICK:
      updateBellModeForBell();

      switch (uiBellMode)
      {
        case BT_BEACON_MODE:
        case BT_BELL_MODE:
          if( uiMqttWaitRespTO > BT_MQTT_SEND_TO ){
            setBtFsmAppState( BT_ERR );
          }else{
            uiMqttWaitRespTO += MILISECONDS_PER_SYSTICK;
          }
          break;
        
        case BT_CFG_MODE:
          if( HAL_GPIO_ReadPin( WU_GPIO_Port, WU_Pin ))
          {
            /* Disable all used wakeup sources: PWR_WAKEUP_PIN3 */
            HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

            /* Clear all related wakeup flags*/
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    
            /* Enable WakeUp Pin PWR_WAKEUP_PIN3 connected to PA.02 (Arduino A7) */
            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

            hwSleepingInitiate();
          }
          break;
      }
      break;

    case EV_APP_IDLE:
    case EV_APP_INIT:
      setBtFsmAppState( BT_IDLE );
      break;

    case EV_UART_RX:
		{
      char ch =  p->reserved;

      if( ch == 0x0d )
      {
        DBGT(LOG_DEBUG, "]\nW[");
      }else
      {
        LOG(LOG_DEBUG, LOG_DEBUG, "%c", ch);
      }

      if( sniffByTemplate(( uint8_t )ch, &trg ))
      {
				DBGT(LOG_DEBUG, "\nFSM:Ok");
				
        setEspRstControl( OFF );  // Turn ON Reset on ESP
        setEspPwrControl( OFF );  // Power OFF for ESP
        setLedControl( OFF );     // Turn Off LED 

        if(( uiModeBeforeEnteringState != uiBellMode )
          &&( uiModeBeforeEnteringState == BT_BEACON_MODE ))
          {
            setBtFsmAppState( BT_IDLE );
            return 0;
          }

        // Go to sleep
        DBGT(LOG_DEBUG, "\nFSM:SLEEP");
        /* Disable all used wakeup sources: PWR_WAKEUP_PIN3 */
        HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
        /* Clear all related wakeup flags*/
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
        /* Enable WakeUp Pin PWR_WAKEUP_PIN3 connected to PA.02 (Arduino A7) */
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

        hwSleepingInitiate();

      }else if( sniffByTemplate(( uint8_t )ch, &trgE ))
      {
				DBGT(LOG_DEBUG, "\nFSM:ERR");
        setEspRstControl( OFF );  // Turn ON Reset on ESP
        setEspPwrControl( OFF );  // Power OFF for ESP

        setBtFsmAppState( BT_ERR );
      }else{
        /* code */
        HAL_UART_Receive_IT (&huart2, pUartRxBuff, 1);
      }
      break;
		}

    case EV_ADC_SCAN:
    default:
      return 0;
  }
  return 0;
}

/* ------------------------------------------------------------------
   Err Mode state 
   ------------------------------------------------------------------ */
static uint8_t btfsmErrStateEh(_tEQ* p)
{
  switch (p->eId)
  {
    case EV_APP_IDLE:
    case EV_APP_INIT:

    case EV_SYS_TICK:
      updateBellModeForBell();

      if( uiErrorTO > BT_ERR_STATE_TO )
      {
        setLedControl( OFF );     // Turn Off LED         
        
        if(( uiModeBeforeEnteringState != uiBellMode )
          &&( uiModeBeforeEnteringState == BT_BEACON_MODE )){
            setBtFsmAppState( BT_IDLE );
            return 0;
          }

        // Go to sleep
        /* Disable all used wakeup sources: PWR_WAKEUP_PIN3 */
        HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
        /* Clear all related wakeup flags*/
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
        /* Enable WakeUp Pin PWR_WAKEUP_PIN3 connected to PA.02 (Arduino A7) */
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

        hwSleepingInitiate();

      }else{
        uiErrorTO += MILISECONDS_PER_SYSTICK;
      }
      break;

    case EV_APP_SEC_TICK:
    case EV_ADC_SCAN:
    case EV_UART_RX:
    default:
      return 0;
  }
  return 0;
}

/*

*/
static uint8_t setBtFsmAppState( uint32_t newState )
{
	char pBuff[ 7 ];
	
  DBGT(LOG_DEBUG, "\nFSM:%d->%d", uiBtFsmAppState, newState );

	switch (newState) 
  {
    case BT_IDLE:
      // Init state vars
      setEspRstControl( ON );  // Turn ON Reset on ESP
      setEspPwrControl( OFF );  // Power OFF for ESP
      setLedControl( OFF );     // Turn Off LED 
      break;

    case BT_CHECK_MODE:
      /* Check and handle if the system was resumed from StandBy mode */ 
      {
				setEspRstControl( ON );  // Turn ON Reset on ESP
				
        blPwrWuFlag = 0;
        if(__HAL_PWR_GET_FLAG(PWR_FLAG_WU) != RESET )
        {
          /* Clear WakeUp flag */
          __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); 
          
          blPwrWuFlag = 1;
        }

        Pa0PinVal = HAL_GPIO_ReadPin( WU_GPIO_Port, WU_Pin );

//        DBGT( LOG_DEBUG, "\nPWRWU[%d] Alarm[%c] WUP[%d]", blPwrWuFlag, AlarmNum, Pa0PinVal );
        DBGT( LOG_DEBUG, "\nPWRWU[%d] WUP[%d]", blPwrWuFlag, Pa0PinVal );

        if( uiBellMode == 0 )
        {
          if( !blPwrWuFlag )
            uiBellMode = modeCfg;    // Cfg mode
          else{ 
            if( Pa0PinVal ) 
              uiBellMode = modeBell;  // Bell Mode
            else
              uiBellMode = modeBeacon; // Beacon mode 
          }
        }
      }	
      break;
      
    case BT_ESP_PWRON:
      // Init state vars
      setEspPwrControl( ON );   // Power On for ESP
      setLedControl( ON );      // Turn On LED 
      uiBtWaitPwrOnSeconds  = 0;
      break;

    case BT_ADC_BATTERY_CONTROL:
      // Init state vars
      uiADCSum            = 0;
      uiADCSamlesNum      = 0; 
      uiAdcTO             = 0;
      uiBatteryPercentage = 0;
			startADCConversion();
      break;

    case BT_ESP_RESET_LEAVE:
      // Init state vars
      uiRstTO             = 0;
      break;

    case BT_BEACON_CFG_RECV:
      // Init state vars
      uiBeaconRecvTO      = 0;
      triggerSet( &trg, pRespPattern, sizeof(pRespPattern));
      trg.pResult =( uint8_t* )pResultBuff;
      trg.resultBuffSize = sizeof( pResultBuff );
			HAL_UART_Receive_IT( &huart2, pUartRxBuff, 1 );
      
      setEspRstControl( OFF );  // Turn off Reset on ESP
      break; 

    case BT_MODE_SEND_WAIT_MQTT_RES:
      // Init state vars
			DBGT( LOG_DEBUG, "\n" );
      uiModeBeforeEnteringState = uiBellMode;
      uiMqttWaitRespTO    = 0;
      triggerSet( &trg, pOkPattern, sizeof(pOkPattern));            /* Set Ok trigger for resp  */
      triggerSet( &trgE, pErrPattern, sizeof(pErrPattern));         /* Set Err trigger for resp */
      sprintf(pBuff, "b[%c%02d]",( char )uiBellMode, uiBatteryPercentage );
      HAL_UART_Transmit(&huart2, (uint8_t *)pBuff, 6, 0xFFFF);      /* Send mode to ESP         */
	  	HAL_UART_Receive_IT( &huart2, pUartRxBuff, 1 );               /* Initiate UART receive    */
      break;

    case BT_ERR:
      // Init state vars
      uiModeBeforeEnteringState = uiBellMode;
      setEspRstControl( OFF );  // Turn ON Reset on ESP
      setEspPwrControl( OFF );  // Power OFF for ESP
      fwkLedBlink( 1, 2, 20 );
      uiErrorTO = 0;
      break;

		default:
			break;
	}

  uiBtFsmAppState = newState;
	return 0;
}


/*!
 \fn			uint8_t appBtFsmEvHandler(_tEQ* p)
 \brief 	Framework events handler
 \param 	_tEQ* p Pointer to event object from events queue
 \retval	0 - handled event should be suppressed
 \retval	1 - handled event should be delivered to other apps
 */
uint8_t appBtFsmEvHandler(_tEQ* p)
{
   /* Application control block */
   switch (uiBtFsmAppState) {
    case BT_IDLE:
       return btFsmIdleStateEh( p );
    case BT_ESP_PWRON:
       return btfsmESPPwrOnStateEh( p );
    case 	BT_ADC_BATTERY_CONTROL:
       return btfsmCheckBatteryStateEh( p );
    case BT_ESP_RESET_LEAVE:
       return btfsmESPResetOffStateEH( p );
    case BT_BEACON_CFG_RECV:
        return btfsmBeaconWaitStateEH( p );
    case BT_MODE_SEND_WAIT_MQTT_RES:
       return btfsmSendModeWaitMqttRespStateEh( p );
    case BT_CHECK_MODE:
       return btFsmCheckModeStateEH( p );
		case BT_ERR:
       return btfsmErrStateEh( p ); 

     default:
       return btfsmErrStateEh( p );
   }
}

