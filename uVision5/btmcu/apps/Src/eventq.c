/*! @file      eventq.c
    @date      2017-05-30 22:25
    @brief     It is my message queue 
    @details   
    @bug
    @author     ruarka
*/

/* ------------------------------------------------------------------------------------------------
*                                 Includes
* ------------------------------------------------------------------------------------------------
*/
#include <stdint.h>

#include "eventq.h"

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
/** 
 * ring buffer of message queue
 */ 
_tEQ eqBuff[ EQ_BUFF_SIZE ];
/**
 * reading index of buffer
 */
uint16_t rdIdx;
/**
 * writing index of buffer
 */
uint16_t wrIdx;
/**
 * the number of message in queue
 */
uint16_t eqNumber;

/* ------------------------------------------------------------------------------------------------
 *                                 Local Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Functions
 * ------------------------------------------------------------------------------------------------
 */


/**
  * @fn     void eqReset(void)
  * @brief  
  * @param  
  * @return
*/
void eqReset(void)
{
#ifndef VS_UTEST
	__disable_irq();
#endif

    rdIdx = wrIdx = eqNumber = 0;

#ifndef VS_UTEST
	__enable_irq();
#endif
}


/**
 * @fn     bool eqGetEvent(_tEQ* p)
 * @brief
 * @param
 * @return 
 */
uint8_t eqGetEvent(_tEQ* p)
{
    uint8_t blF = 0x01;

#ifndef VS_UTEST
	__disable_irq();
#endif

    if (eqNumber)
    {
        if (rdIdx >= EQ_BUFF_SIZE)
            rdIdx -= EQ_BUFF_SIZE;

        eqNumber--;
        p->eId = eqBuff[rdIdx].eId;
        p->reserved = eqBuff[rdIdx++].reserved;
    }
    else
        blF = 0x00;

#ifndef VS_UTEST
	__enable_irq();
#endif

    return blF;
}

/**
 * @brief    Adds Event message into Event Queue
 * @param    p Pointer to Event Structure 
 * @return   0x01 in case of success Event registration
 */
uint8_t eqAddEvent(_tEQ* p)
{
    uint8_t blF = 0x01;

#ifndef VS_UTEST
	__disable_irq();
#endif

    if (eqNumber < EQ_BUFF_SIZE)
	{
        if (wrIdx >= EQ_BUFF_SIZE)
            wrIdx -= EQ_BUFF_SIZE;

		eqBuff[wrIdx].eId = p->eId;
		eqBuff[wrIdx++].reserved = p->reserved;
        eqNumber++;
	}
    else
        blF = 0x00;

#ifndef VS_UTEST
	__enable_irq();
#endif

    return blF;
}

/**
 * @brief    Adds Event message into Event Queue from interrupt. Interrupts disabling not needed.
 * @note     Hang on if queue is full
 * @param    p Pointer to Event Structure 
 * @return   Nothing
 */
void eqAddEventFromIt(_tEQ* p)
{
  if (eqNumber < EQ_BUFF_SIZE)
	{
    if (wrIdx >= EQ_BUFF_SIZE)
        wrIdx -= EQ_BUFF_SIZE;

		eqBuff[wrIdx].eId = p->eId;
		eqBuff[wrIdx++].reserved = p->reserved;
        eqNumber++;
	}
}

/**
 * @brief Defined queue buffer size 
 * @param None
 * @return Queue buffer size 
 */
uint16_t eqGetBufferSize(void)
{
    return EQ_BUFF_SIZE;
}

/**
 * @brief Actual number of messages in queue 
 * @param None
 * @return number of messages in queue   
 */
uint16_t eqGetNumbers(void)
{
    uint16_t uiTmp;

#ifndef VS_UTEST
    __disable_irq();
#endif
    uiTmp = eqNumber;
#ifndef VS_UTEST
    __enable_irq();
#endif

    return uiTmp;
}

