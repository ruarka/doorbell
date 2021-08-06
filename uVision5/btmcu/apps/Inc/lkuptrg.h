/*! \file      lkuptrg.h
    \version   0.1
    \date      2021-08-05
    \brief     <A Documented file.>
    \details   <Details.>
    \bug
    \copyright
    \author    ruarka
*/

#ifndef GSMTRGFS_H
#define GSMTRGFS_H
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */
/**

*/
typedef struct token_trigger_t
{
  const char*	pTemplate;
  uint8_t   	templateLen;
  uint8_t   	templateIdx;
  uint8_t*  	pResult;
  uint8_t   	resultLen;
  uint8_t		resultBuffSize;
  uint8_t   	resIdx;
  uint8_t   	fired;
  uint8_t   	expressionChar;
  uint8_t  		expression;
} token_trigger_t; 

extern uint8_t sniffByTemplate( 
		uint8_t byte,  // Incoming char
		struct token_trigger_t* pTrigger // Trigger structure
        );

extern void triggerSet( struct token_trigger_t* pTrg, const char* pRespTmpl, uint8_t rLen );
extern void triggerClear( struct token_trigger_t * pTrg );
