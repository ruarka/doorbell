/*! \file      settings.h
    \version   0.1
    \date      2017-06-08 22:01
    \brief     <A Documented file.>
    \details   <Details.>
    \bug       
    \copyright  
    \author    
*/
#include <stdint.h>

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
 
 /* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
 
 /* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
extern uint32_t  uiBeacon;

extern void hwkReadBufferFromEeprom( uint16_t eeprom_offset, uint8_t* pBuf, uint32_t size); 
extern void hwkWriteBufferToEeprom( uint16_t eeprom_offset, uint8_t* pBuff, uint32_t size );
 
 
/* ------------------------------------------------------------------------------------------------
*                                     Macros
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
*                                 Type definitions
* ------------------------------------------------------------------------------------------------
*/

/*!

 */
enum {
  MODE_AOT = 0,   /**< Action On Timer mode   */
  MODE_AOS,       /**< Action On Sensor mode  */
  MODE_COUNT
};

/* ------------------------------------------------------------------------------------------------
*                                    Functions declarations
* ------------------------------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
typedef struct _settings
{
  uint16_t		secondsToSleep;
	uint32_t		crc;
} tSettings;

/* ------------------------------------------------------------------------------------------------
 *                                    Functions
 * ------------------------------------------------------------------------------------------------
 */

#ifdef __cplusplus
extern "C"
{

  tSettings settings;

//  uint32_t settings_load( void );
  
  void loadSettings( void );

//  uint8_t settings_flush( void );

  void flushSettings( void );
  
  tSettings* getSettings( void );

  void assing_default_settings(void);

}
#else
extern tSettings settings;

// extern uint32_t settings_load( void );

extern void loadSettings( void );

// extern uint8_t settings_flush( void );
extern void flushSettings( void );

extern void flushSettings( void );

extern tSettings* getSettings( void );

extern void assing_default_settings(void);

#endif

// extern void assing_default_settings(void);

