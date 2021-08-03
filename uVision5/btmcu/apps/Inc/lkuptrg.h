/**************************************************************************************************
    Filename:       gsmtrgfs.h
    Revised:        $Date: 2010-04-22 17:30:00 - (Tue, 22 April 2010) $
    Revision:       $Revision: 1 $

    Description:    SIM300 GSM incoming data steam sniffer.
                    Functions fire an appropriate events if searching 
                    template is met.

    Copyright 
	
  0xfc - miss chars until the next met
  0xfd - miss one char
  0xfe - place one char into result buffer
  0xff - place chars into result buffer until the next char met 
    
Example
	Commands
	CONST_CHAR_FLASH AT_SMS_CMD_ENTER[]= { 0x0d, 0x0a };

	CONST_CHAR_FLASH GPRS_REPLY_RX_SMS[]=
		{ '+','C','M','T','I',':',' ','"','S','M','"',',',0xff,0x0d,0x0a };

	CONST_CHAR_FLASH READ_SMS_TAG_RESP[]=
		{ '+','C','M','G','R',':',0xff,0x0d,0x0a };     // Result contains the number of SMS parameters

	CONST_CHAR_FLASH AT_DEL_SMS[]=
		{ 'A','T','+','C','M','G','D','=' };

	CONST_CHAR_FLASH AT_DEL_SMS_RESP[]=
		{ 'A','T','+','C','M','G','D','=', 0x0d, 0x0d, 0xff, 0x0d, 0x0a };

	CONST_CHAR_FLASH AT_FIRST_SMS_IDX_FOR_DEL[]= { '1' };

	
	uint8 pNewSmsIdx[ 10 ];	
	
	tNewSms.pResult       = pNewSmsIdx;
	tNewSms.resultLen     = sizeof( pNewSmsIdx )-1;

	// Clear trigger 
	smsTriggerClear( &tSms );

	// Set NewSms trigger template to SMS waiting
	smsTriggerSet( &tNewSms, GPRS_REPLY_RX_SMS, sizeof( GPRS_REPLY_RX_SMS ));
  
	
**************************************************************************************************/
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
