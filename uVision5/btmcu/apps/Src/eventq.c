/*! \file      eventq.c
    \version
    \date      2017-05-30 22:25
    \brief     <A Documented file.>
    \details   <Details.>
    \bug
    \copyright
    \author
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

_tEQ eqBuff[ EQ_BUFF_SIZE ];
uint16_t rdIdx;
uint16_t wrIdx;
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
  * \fn     void eqReset(void)
  * \brief  
  * \param  
  * \return
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
 * \fn     bool eqGetEvent(_tEQ* p)
 * \brief
 * \param
 * \return 
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
 * \fn       bool eqAddEvent(_tEQ* p )
 * \brief    Adds Event messege into Event Queue
 * \param    _tEQ* p Pointer to Event Structure 
 * \return   0x01 in case of success Event registration
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
 * \fn       bool eqAddEventFromIt(_tEQ* p )
 * \brief    Adds Event messege into Event Queue from interrupt. Interrupts disabling not needed.
 * \param    _tEQ* p Pointer to Event Structure 
 * \return   Hangon if queue is full
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
 * \fn     uint16_t eqGetBufferSize(void)
 * \brief
 * \param
 * \return
 */
uint16_t eqGetBufferSize(void)
{
    return EQ_BUFF_SIZE;
}

/**
 * \fn     uint16_t eqGetNumbers(void)
 * \brief
 * \param
 * \return 
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

