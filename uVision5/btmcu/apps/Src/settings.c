/*! \file       settings.c
    \version    0.1  
    \date       2017-06-08 22:02
    \brief      An interface between an App layer and Conf layer 
    \bug       
    \copyright  
    \author    ruarka

    \details   Application profile is placed in memory. 
               (+) Module provides functionality to save application 
                   profile into Eeprom.

               (+) Also it reass data from Eeprom into tSetting structure 
                   into memory.

               (+) Default data to fill tSetting structure also provided. 
*/
/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include "eventq.h"
#include "settings.h"
#include "appbtfms.h"
#include "appfwk.h"
#include "main.h"

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                    Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                 Global Variables
 * ------------------------------------------------------------------------------------------------
 */
tSettings settings;     /**< Settings application profile */

uint32_t  uiBeacon = 15;     /**< beacon value to be loaded into it. 
                                  15 means seconds after leaving config mode */

/* ------------------------------------------------------------------------------------------------
 *                                 Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                    Functions
 * ------------------------------------------------------------------------------------------------
 */
uint32_t calculateCrcEx( uint8_t* pBuff, uint32_t size );

/**
 * \brief  Flush settings from memory into eeprom
 * \param  None
 * \return None
 */
void flushSettings( void )
{
  /* calculate CRC */
  settings.crc = calculateCrcEx(( uint8_t* )&settings, sizeof( settings )-sizeof( settings.crc ));
  
  hwkWriteBufferToEeprom( 0,( uint8_t* )&settings, sizeof( tSettings ));
}
/**
 * \brief  Flush settings from memory into eeprom
 * \param  None
 * \retval Pointer to tSettings structure 
 */
tSettings* getSettings( void )
{
  return &settings;
}
/**
 * \brief  Load setting from eeprom into tSettings struct
 * \param  None
 * \retval Pointer to tSettings structure 
 */
void loadSettings( void )
{
	uint32_t tmp;
  
  hwkReadBufferFromEeprom( 0,( uint8_t* )&settings, sizeof( tSettings ));
	
  /* calculate CRC */
	tmp = calculateCrcEx(( uint8_t* )&settings, sizeof( settings )-sizeof( settings.crc ));
  
	if( settings.crc == tmp )
		return;
	
  assing_default_settings();
}
/**
 * \brief  Assing default settings to profile placed in memory
 * \param  None
 * \retval None 
 * \note   Data is missed after reset and it is needed to be flushed into Eeprom to keep them.
 */
void assing_default_settings( void )
{
	settings.secondsToSleep     = TIME_TO_SLEEP_DEF_VAL;
  settings.crc = calculateCrcEx(( uint8_t* )&settings, sizeof( settings )-sizeof( settings.crc ));
}
/**
 * \brief  Calculates CRC
 * \param  pBuff pointer onto tSetting struct in memory
 * \param  size  of buff
 * \retval calculated CRC 
 */
uint32_t calculateCrcEx( uint8_t* pBuff, uint32_t size )
{
  uint32_t tmpCRC = 0;
  
  for( uint32_t i=0; i<size; i++)
    tmpCRC += pBuff[ i ];
  
  return tmpCRC;
}
