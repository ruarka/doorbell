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
uint8_t fwkLedsProcessing(_tEQ* p);

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
typedef struct _LED_Entity
{
  uint8_t  ledId;
  uint8_t  OnNum;
  uint8_t  OffNum;
  uint8_t  blPeriodic;
  uint8_t  OnTicks;
  uint8_t  OffTicks;
  uint8_t  OnOffState;
} _tLed;

enum {
  OnTime = 0,
  OffTime,
  Block
};


_tLed LEDS[ APP_LEDS_NUMBER ];

void fwkLedSet( uint8_t uiLedId, uint8_t ontime, uint8_t offTime, uint8_t blPeriodic )
{
  LEDS[uiLedId].ledId       = uiLedId;
  LEDS[uiLedId].OnNum       = ontime;
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

void fwkLedOn( uint8_t uiLedId )
{
  fwkLedSet( uiLedId, 40, 0, 1 );
}

void fwkLedOff( uint8_t uiLedId )
{
  fwkLedSet( uiLedId, 0, 40, 1 );
}

void fwkLedBlink( uint8_t uiLedId, uint8_t ontime, uint8_t offTime )
{
  fwkLedSet( uiLedId, ontime, offTime, 1 );
}

uint8_t fwkLedsProcessing(_tEQ* p)
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
  return 0;
}
// =========================================================================
// Application support functions
// =========================================================================
/**
 * \fn     void fwkSendEventToAppDirectly( uint8_t appId, _tEQ* pEv )
 * \brief  Sends an Event to appropriate Appl. It uses to send commands directly
 *         between Applications
 * \param  aapID Application number in event handlers list
 * \param  _tEQ* pEv An Event to send
 * \return None
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
 * \fn     void fwkAppInit(void)
 * \brief  Initialize registered applications before Events handling launch 
 * \param  None
 * \return None
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
 * \fn     void fwkAppEventLoop(_tEQ* pE)
 * \brief  Delivers &pE event to registered Application Handlers 
 * \param  pE pointer to an Application Message 
 * \return None
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
 * \fn      void fwkMain(uint8_t blDoContinuesLoop)
 * \brief
 * \param   blDoContinuesLoop - 0x01 continues Event Queue handling without return
 *                            - 0x00 one time EQ reading (mostly for tetsting purposes)
 * \return  None.
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
