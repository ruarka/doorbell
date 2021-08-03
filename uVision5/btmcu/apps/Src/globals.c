/*! \file      globals.c
    \version   0.1  
    \date      2018-04-16 17:54
    \brief     Contents application layer GLOBAL variables and constants
    \author    ruarka
		
	\bug
				
	\details
*/
/* ------------------------------------------------------------------------------------------------
 *                                 Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>
#include "globals.h"
 
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
/*! \brief week days acronims 
 *  \details week day number starts from 1. 
 *           0 fake index
 */
const char* const weekDayStrs[] =
{
  "NN",
  "Mn",
  "Tu",
  "We",
  "Th",
  "Fr",
  "Sa",
  "Sn"
};

/* Stirng Buffer for display */
char pDisplayStrBuff[ 17 ];

/*! Int into HEX conversion support variable */
const char pIntToHex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

/*! English to Russaian week days indexes translation */
const uint8_t weekdayRu[] = { 7, 1, 2, 3, 4, 5, 6 };

/*! Time format string used for Debug purposes */
const char pTimeFormatStr[]="\n0 00:00:00 ";


/*!  */
uint8_t pUartRxBuff[ 2 ];

/* */
char AlarmNum = 'N';

/* */
uint8_t blPwrWuFlag;

/* */
uint8_t Pa0PinVal;

/**
 * \fn     uint32_t mapDigitalValue( uint32_t val, uint32_t biggestVal, uint32_t lowestVal, uint32_t scalePoints )

 * \brief
 * \param
 * \return
 */
uint32_t mapDigitalValue( uint32_t val, uint32_t biggestVal, uint32_t lowestVal, uint32_t scalePoints )
{
  if( lowestVal >= val )
    return 0;

  if( val >= biggestVal )
    return scalePoints-1;

  uint32_t interval =( biggestVal-lowestVal )/scalePoints;

  uint32_t normalizedVal =( val-lowestVal )/interval;

  return normalizedVal;
}

int my_atoi( char* text, int len )
{
	int sum = 0;
	int multiplier = 1;
	for( int idx = len-1; idx >= 0; idx-- )
	{
		switch( text[idx] ) {
			case 0:
				break;

			case 1: sum += multiplier; break;
			case 2: sum += 2*multiplier; break;
			case 3: sum += 3*multiplier; break;
			case 4: sum += 4*multiplier; break;
			case 5: sum += 5*multiplier; break;
			case 6: sum += 6*multiplier; break;
			case 7: sum += 7*multiplier; break;
			case 8: sum += 8*multiplier; break;
			case 9: sum += 9*multiplier; break;
		}
		multiplier *= 10;
	}
	return sum;
}
