/*! \file      lkuptrg.c
    \version   0.1
    \date      2021-08-02 18:25
    \brief     
    \author    ruarka
    \details   

  @verbatim
  The function of this module is to sniff incoming stream for condition defined by trigger.
  Before use sniffByTemplate an appropriate trigger should be assigned.
  Assigning trigger could be done by triggerSet.
  If it is needed to clear trigger then call triggerClear.

  Trigger rules 
  sniffByTemplate looking for chars in trigger pattern for each char consistently until
  meta symbols (0xfc, 0xfd, 0xfe, 0xff) are met. 
  
  Meaning of meta symboles
  0xfc - miss chars until the next met
  0xfd - miss one char
  0xfe - place one char into result buffer
  0xff - place chars into result buffer until the next char met 

	Triggers Examples
	CONST_CHAR_FLASH AT_SMS_CMD_ENTER[]= { 0x0d, 0x0a };  // waits for <CR> & <LF>

	CONST_CHAR_FLASH GPRS_REPLY_RX_SMS[]=
		{ '+','C','M','T','I',':',' ','"','S','M','"',',',0xff,0x0d,0x0a }; // 

	CONST_CHAR_FLASH READ_SMS_TAG_RESP[]=
		{ '+','C','M','G','R',':',0xff,0x0d,0x0a };     // Result contains the number of SMS parameters

	CONST_CHAR_FLASH AT_DEL_SMS_RESP[]=
		{ 'A','T','+','C','M','G','D','=', 0x0d, 0x0d, 0xff, 0x0d, 0x0a };

	CONST_CHAR_FLASH AT_FIRST_SMS_IDX_FOR_DEL[]= { '1' };   

  @endverbatim  
*/


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdint.h>
#include <stdio.h>

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
/* ------------------------------------------------------------------------------------------------
 *                                    Functions
 * ------------------------------------------------------------------------------------------------
 */
/**
  * @brief  Sets sniffer trigger for templates.  
  *
  * @param  byte:  byte from incoming stream . 
  * @param  pTrigger: pointer to trigger struct   
  * 
  * @retval 0x01 in case of trigger met
  * @retval 0  otherwise 
  * 
  * @details Sniffer Template Structure
  *    
  *   - 0xfc - miss chars until the next met
  *   - 0xfd - miss one char
  * 
  *   - 0xfe - place one char into result buffer
  *   - 0xff - place chars into result buffer until the next char met 
  * 
  * Example \n 
  *         const char * AT_SERVER_IP         = "AT+CDNSORIP=0"; \n
  *         char pRespPattern[]={ 'b', '[', (char)0xff, ']' }; \n
  */
uint8_t sniffByTemplate( uint8_t byte,
		                 struct token_trigger_t* pTrigger  // Trigger structure
        )
{
  if(( pTrigger->fired )
    ||( pTrigger->templateIdx > pTrigger->templateLen ))
    return 0x00;

  if( !pTrigger->expression )
  {
     uint32_t blExpressionMet = 0;
     /* select expressionChar or expression */
     while(( pTrigger->templateIdx < pTrigger->templateLen )
    	&&( !blExpressionMet ))
    {
      unsigned char templateCh = pTrigger->pTemplate[ pTrigger->templateIdx++ ];

      switch( templateCh ) {
	    case 0xff:
		case 0xfc:
	      pTrigger->expression      = templateCh;   // look for comparing char
	      pTrigger->expressionChar  = 0x00;   // Wrong character for condition check
	      break;

		case 0xfd:
		case 0xfe:
		  pTrigger->expression      = templateCh;   // look for comparing char
		  pTrigger->expressionChar  = 0x00;   // Wrong character for condition check
		  blExpressionMet = 1;
		  break;

		default:
		  pTrigger->expressionChar  = templateCh;
		  blExpressionMet = 1;
		  break;
	  }
    }

    if( !blExpressionMet )
      return 0;
  }

  /* check with incoming stream data */
  switch( pTrigger->expression )
  {
    case 0xff:
      // Receive until the next char met
      if( byte == pTrigger->expressionChar )
      {
        // clear trigger expression
        pTrigger->expression 	 = 0x00;
        pTrigger->expressionChar = 0x00;
      }
      else
        if( pTrigger->resultBuffSize > pTrigger->resIdx )
          pTrigger->pResult[ pTrigger->resIdx++ ]= byte;
      break;

    case 0xfe:
      // place one char into result buffer
      if( pTrigger->resultLen > pTrigger->resIdx )
        pTrigger->pResult[ pTrigger->resIdx++ ]= byte;

      // clear trigger expression
      pTrigger->expression 	   = 0x00;
      pTrigger->expressionChar = 0x00;
      break;

    case 0xfd:
      // miss one char from incoming stream
      // clear action to compare
      pTrigger->expression 		= 0x00;
      pTrigger->expressionChar 	= 0x00;
      break;

    case 0xfc:
      // miss chars until the next met
      if(( char )byte == pTrigger->expressionChar )
      {
        // clear action to compare
        pTrigger->expression = 0x00; 
        pTrigger->expressionChar 	= 0x00;
      }
      break;

    default:
      // just comparing
      if((char) byte != pTrigger->expressionChar )
      {
        pTrigger->templateIdx = 0;
        pTrigger->resIdx      = 0;
      }
      break;
  }

  /* check template end is met */
  if((( pTrigger->templateIdx )>= pTrigger->templateLen )
	  &&( !pTrigger->expression ))
  {
    pTrigger->fired     = 0x01;
    pTrigger->resultLen = pTrigger->resIdx;
    return 0x01;
  }
  return 0x00;
}
/**
  * @brief  Sets sniffer trigger for templates.  
  * @param  pTrg  pointer to sniffer trigger. 
  * @param  pRespTmpl pointer to template   
  * @param  rLen Template length
  * @retval none
  */
void triggerSet( struct token_trigger_t* pTrg, const char* pRespTmpl, uint8_t rLen )
{
  triggerClear( pTrg );

  pTrg->pTemplate    	= pRespTmpl;
  pTrg->templateLen     = rLen;
}

/**
  * @brief  Clears sniffer trigger.  
  * @param  pTrg  pointer to sniffer trigger.   
  * @retval none
  */
void  triggerClear( struct token_trigger_t * pTrg )
{
  pTrg->expression   	  = 0;
  pTrg->expressionChar  = 0;
  pTrg->fired        	  = 0;
  pTrg->resIdx       	  = 0;
  pTrg->templateIdx  	  = 0;
  pTrg->pTemplate    	  = 0;
  pTrg->templateLen  	  = 0;      
  pTrg->resultBuffSize 	= 0;
  pTrg->resultLen		    = 0;
}
