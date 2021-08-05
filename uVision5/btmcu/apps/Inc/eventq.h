/*! \file      eventq.h
    \version
    \date      2017-05-12 11:32
    \brief     System events queue header file.
    \details   Interactions between hal and app level was done througth the events queue.
    \bug
    \copyright
    \author
*/

/* ------------------------------------------------------------------------------------------------
*                                     Macros
* ------------------------------------------------------------------------------------------------
*/
/*! \def     EQ_BUFF_SIZE
    \brief   It is defines the size of system events queue in event objects.
    
    It is assumed that an app will never reach a queue size. In correctly built system messages 
    placed in the queue should be read quicker then they are come into. 
*/
//#define EQ_BUFF_SIZE  32 
#define EQ_BUFF_SIZE  100 

/* ------------------------------------------------------------------------------------------------
*                                 Type definitions
* ------------------------------------------------------------------------------------------------
*/
/*! \typedef  _tEQ
    \brief    It defines System Events Queue object.
    
    HAL and Apps leayer communicates by placing objects of defined type into system events queue. 
*/
typedef struct _Event_Queue
{
	uint16_t eId      : 16;  /*!< Event number  */ 
	uint16_t reserved : 16;  /*!< Event param or undefined  */
} _tEQ;

/* ------------------------------------------------------------------------------------------------
*                                    Functions declarations
* ------------------------------------------------------------------------------------------------
*/
#ifdef __cplusplus
extern "C"
{
#endif

void eqReset(void);
uint8_t eqGetEvent(_tEQ* p);
uint8_t eqAddEvent(_tEQ* p);
void eqAddEventFromIt(_tEQ* p);
uint16_t eqGetBufferSize(void);
uint16_t eqGetNumbers(void);

#ifdef __cplusplus
}
#endif
