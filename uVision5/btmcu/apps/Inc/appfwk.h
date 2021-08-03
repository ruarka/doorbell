/*! \file      appfwk.h
    \version   0.1
    \date      2017-05-18 11:55
    \brief     header for application framework 
    \author    ruarka
*/


/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
/*! \var APP_LEDS_NUMBER
    \brief Number of LEDS in defined in Application

    <Details>.
*/
#define APP_LEDS_NUMBER                 2


/* ------------------------------------------------------------------------------------------------
*                                 Type definitions
* ------------------------------------------------------------------------------------------------
*/

typedef  uint8_t (*pfnAppInit)(void);

/*! \var typedef const bool(*const pfnAppEventHandler)(_tEQ* p)
    \brief Type of Application Event Handler Function definition
*/
typedef uint8_t(*pfnAppEventHandler)(_tEQ* p);

/**
 Framework application interface
*/

typedef struct _application_event_link
{
    const uint16_t event;
    const pfnAppEventHandler  * pEventHandlersList;
} _tAppEventLink;

/* ------------------------------------------------------------------------------------------------
*                                    Functions declarations
* ------------------------------------------------------------------------------------------------
*/
extern void fwkLedSet( uint8_t uiLedId, uint8_t ontime, uint8_t offTime, uint8_t blPeriodic );
extern void fwkLedOn( uint8_t uiLedId );
extern void fwkLedOff( uint8_t uiLedId );
extern void fwkLedBlink( uint8_t uiLedId, uint8_t ontime, uint8_t offTime );

extern uint8_t fwkLedsProcessing(_tEQ* p);


#ifdef __cplusplus
extern "C"
{
#endif

void fwkAppInit(void);
void fwkMain(uint8_t blDoContinuesLoop);
uint8_t dispCheckStr(uint8_t row, const char* const pcStr);

#ifdef __cplusplus
}
#endif
