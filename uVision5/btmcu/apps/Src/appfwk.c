/*! \file       appfwk.c
    \version    0.1
    \date       2017-05-30 22:28
    \author     ruarka
    
    \brief      File contains application layer framework functions.
    
    @verbatim 
    @endverbatim
    
    \bug
 */
/* ------------------------------------------------------------------------------------------------
*                                 Includes
* ------------------------------------------------------------------------------------------------
*/
#include <stdint.h>
#include "globals.h"

#include "hal.h"
#include "events.h"
#include "eventq.h"
#include "appbtfms.h"
#include "settings.h"
#include "appfwk.h"
#include "main.h"


/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
// uint8_t fwkLedsProcessing(_tEQ* p);

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
/*! For each event the list of app handlers should be defined. */
/*! End of the list should be marked by 0 */

/*! Init application handlers list */
pfnAppEventHandler const pEvAppInitHandlersList[]       ={ appBtFsmEvHandler, 0 };


/*! Adc scan event to detect key pressed application handlers list */
pfnAppEventHandler const pEvAppAdcKbPresedHandlersList[]= { appBtFsmEvHandler, 0 };

/*! System Ticks application handlers list */
pfnAppEventHandler const pEvSysTickHandlersList[]       = { appBtFsmEvHandler, 0 };

/*! UART Rx application handlers list */
pfnAppEventHandler const pEvUartRxHandlersList[]       = { appBtFsmEvHandler, 0 };


/*! List of application handlers list */
_tAppEventLink const eventHandlersList[] = {
    { EV_SYS_TICK,        pEvSysTickHandlersList },
    { EV_ADC_SCAN,   		  pEvAppAdcKbPresedHandlersList },
    { EV_APP_INIT,        pEvAppInitHandlersList },
    { EV_UART_RX,         pEvUartRxHandlersList },
};

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */

// =========================================================================
// LED Blinking support
// =========================================================================

  /** @brief LED description struct  
   */
typedef struct _LED_Entity
{
  uint8_t  ledId;       /**< LED Id used by application       */
  uint8_t  OnNum;       /**< number of ticks to in ON state   */
  uint8_t  OffNum;      /**< number of ticks to in OFF state  */
  uint8_t  blPeriodic;  /**< is it periodic or 1 time blink   */
  uint8_t  OnTicks;     /**< number of ticks in On period - internally used  */
  uint8_t  OffTicks;    /**< number of ticks in Off period - internally used */
  uint8_t  OnOffState;  /**<  On or Off blink state - internally used        */
} _tLed;

enum {
  OnTime = 0,
  OffTime,
  Block
};

_tLed LEDS[ APP_LEDS_NUMBER ];    /** LEDs representation objects  */

/**
 * @brief  Sets the duty cycle for Application LED 
 * 
 * @param  uiLedId LED id defined by application
 * @param  onTime  Number of 1/40 parts of second to LED is ON
 * @param  offTime Number of 1/40 parts of second to LED is OFF
 * @param  blPeriodic 0 means 1 time blink duration defined by ontime & offTime
 *                    others is permanent blinking 
 * @return None
 * 
 * @note Hang on System ticks handler (1000 per sec) divided onto 25. 
 *       So minimal setting for ontime and offTime is 1/40 second 
 *       To 0.5 second blinking period ontime(20) and offTime(20).
 */
void fwkLedSet( uint8_t uiLedId, uint8_t onTime, uint8_t offTime, uint8_t blPeriodic )
{
  LEDS[uiLedId].ledId       = uiLedId;
  LEDS[uiLedId].OnNum       = onTime;
  LEDS[uiLedId].OffNum      = offTime;
  LEDS[uiLedId].blPeriodic  = blPeriodic;
  LEDS[uiLedId].OnTicks     = 1;
  LEDS[uiLedId].OffTicks    = 1;
  

  if( LEDS[ uiLedId ].OnNum && LEDS[ uiLedId ].OffNum )
      setLedPinState( uiLedId, 1 );

  if( !LEDS[ uiLedId ].OnNum && LEDS[ uiLedId ].OffNum )
      setLedPinState( uiLedId, 0 );

  if( LEDS[ uiLedId ].OnNum && !LEDS[ uiLedId ].OffNum )
      setLedPinState( uiLedId, 1 );
}

/**
 * @brief  Set on the led blink
 * @param  uiLedId LED id defined by application
 * @return None
 */
void fwkLedOn( uint8_t uiLedId )
{
  fwkLedSet( uiLedId, 40, 0, 1 );
}

/**
 * @brief  Set off the led blink
 * @param  uiLedId LED id defined by application
 * @return None
 */
void fwkLedOff( uint8_t uiLedId )
{
  fwkLedSet( uiLedId, 0, 40, 1 );
}

/**
 * @brief  sets led blink
 * @param  uiLedId LED id defined by application
 * @param  onTime  Number of 1/40 parts of second to LED is ON
 * @param  offTime Number of 1/40 parts of second to LED is OFF
 * @return None
 */
void fwkLedBlink( uint8_t uiLedId, uint8_t ontime, uint8_t offTime )
{
  fwkLedSet( uiLedId, ontime, offTime, 1 );
}

/**
 * @brief  Leds processing fuction which is called in Systicks handler
 * @param  None
 * @return None
 */
void fwkLedsProcessing( void )
{
  for(uint8_t i=0; i<APP_LEDS_NUMBER; i++ )
  {
      if( LEDS[ i ].OnNum && LEDS[ i ].OffNum )
      {
          switch (LEDS[ i ].OnOffState)
          {
            case OnTime:
              if( LEDS[ i ].OnTicks >= LEDS[ i ].OnNum )
              {
                  setLedPinState( LEDS[ i ].ledId, 0 );
                  LEDS[ i ].OffTicks = 1;
                  LEDS[ i ].OnOffState = OffTime;
              }
              else
                LEDS[ i ].OnTicks++;
              break;

            case OffTime:
              if( LEDS[ i ].OffTicks >= LEDS[ i ].OffNum )
              {
                  if( LEDS[ i ].blPeriodic )
                  {
                    setLedPinState( LEDS[ i ].ledId, 1 );
                    LEDS[ i ].OnTicks     = 1;
                    LEDS[ i ].OnOffState  = OnTime;
                  }
                  else
                    LEDS[ i ].OnOffState = Block;
              }
              else
                LEDS[ i ].OffTicks++;
              break;

            case Block:
              break;
          }
      }
  }
}
// =========================================================================
// Application support functions
// =========================================================================
/**
 * @brief  Sends an Event to appropriate Appl. It uses to send commands directly
 *         between Applications
 * @param  aapID Application number in event handlers list
 * @param  pEv An Event to send
 * @return None
 */
void fwkSendEventToAppDirectly( uint8_t appId, _tEQ* pEv )
{
  uint16_t uiEvId = pEv->eId;
  for (uint16_t i = 0; i < sizeof(eventHandlersList) / sizeof(_tAppEventLink); i++)
  {
      uint16_t uiTmp = eventHandlersList[i].event;
      if (uiTmp == uiEvId)
      {
          const pfnAppEventHandler * pHandlerList = eventHandlersList[i].pEventHandlersList;
          pHandlerList[appId](pEv);
          return;
      }
  }
}

/**
 * @brief  Initialize registered applications before Events handling launch 
 * @param  None
 * @return None
 */
void fwkAppInit(void)
{
    _tEQ eq = { EV_APP_INIT, 00 };
    
		/* LEDS setting */
		for (uint8_t i = 0; i < APP_LEDS_NUMBER; ++i)
		    fwkLedSet( i, 0, 0, 0 );

    eqReset();
    eqAddEvent(&eq);
}

/**
 * @brief  Delivers &pE event to registered Application Handlers 
 * @param  pE pointer to an Application Message 
 * @return None
 */
void fwkAppEventLoop(_tEQ* pE)
{
    uint16_t uiEvId = pE->eId;
    for (uint16_t i = 0; i < sizeof(eventHandlersList) / sizeof(_tAppEventLink); i++)
    {
        uint16_t uiTmp = eventHandlersList[i].event;
        if (uiTmp == uiEvId)
        {
            const pfnAppEventHandler * pHandlerList = eventHandlersList[i].pEventHandlersList;

            uint16_t j = 0;
            while (pHandlerList[j] != 0)
            {
                if (pHandlerList[j++](pE))
                    return;
            }
        }
    }
}

/**
 * @brief  main framework function 
 * @param   blDoContinuesLoop - 0x01 continues Event Queue handling without return
 *                            - 0x00 one time EQ reading (mostly for tetsting purposes)
 * @return  None.
 * 
 * @note fwkMain should be continuesly called in main cycle
 */
void fwkMain(uint8_t blDoContinuesLoop)
{
    _tEQ eqR;
    uint8_t blQueueNotEmpty;

    do
    {
        blQueueNotEmpty = eqGetEvent(&eqR);
        if (blQueueNotEmpty != 0 )
            fwkAppEventLoop(&eqR);
        else
        { 
            eqR.eId = EV_APP_IDLE;
            eqR.reserved = 0;
        }
    } while( blDoContinuesLoop || blQueueNotEmpty );
}

/* ------------------------------------------------------------------------- 
    FRAMEWORK APPLICATIONS
---------------------------------------------------------------------------- */
