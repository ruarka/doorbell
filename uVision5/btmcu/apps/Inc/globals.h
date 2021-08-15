/*! \file      globals.h
    \version   0.1
    \date      2018-04-16 17:54
    \brief     Contains global variables and definitions for project 
    \details   <Details.>
    \bug       
    \copyright  
    \author    
*/
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

/* ------------------------------------------------------------------------------------------------
 *                                     Macros
 * ------------------------------------------------------------------------------------------------
 */
/*! \var MILISECONDS_PER_SYSTICK
    \brief Number of miliseconds for EV_SYS_TICK

    <Details>.
*/
#define MILISECONDS_PER_SYSTICK		25

#define DBG_STREAM

#define DBG_UART_WEMOS
/*
 * Type for custom logging functions
 */
typedef void (*logger_p)(int entity_level, const int level, const char * format, va_list ap);


/*!
 Logging macros and associated log levels.
 The current log level is kept in entity_level.
 */
enum {
	LOG_NONE = 0,
	LOG_OUTPUT,
	LOG_ERROR,
	LOG_WARNING,
	LOG_TRACE,
	LOG_DEBUG,
};

#ifdef  DBG_STREAM

#define BAR(fmt, ...)  printf(fmt, ##__VA_ARGS__)

#define LOGTIME() printf( "\r\n%d ",  HAL_GetTick())


#define LOG(entity_level, level, format, ...) \
	do { \
			if( entity_level >= level ) \
			{ \
					printf( format, ##__VA_ARGS__  ); \
			} \
	} while(0)

#define LOGT(entity_level, level, format, ...) \
		LOGTIME();  \
		LOG(entity_level, level,  format, ##__VA_ARGS__)

#define TRACE(entity_level, format, ...) \
  LOG(entity_level, LOG_TRACE,  format, ##__VA_ARGS__)

#define TRACET(entity_level, format, ...) \
	LOGTIME(); \
  LOG(entity_level, LOG_TRACE,  format, ##__VA_ARGS__)

#define WRN(entity_level,  format, ... ) \
	LOG(entity_level, LOG_WARNING,  format, ##__VA_ARGS__)

#define WRNT(entity_level,  format, ... ) \
	LOGTIME(); \
	LOG(entity_level, LOG_WARNING,  format, ##__VA_ARGS__)


#define DBG(entity_level, format, ... ) \
	LOG(entity_level, LOG_DEBUG, format, ##__VA_ARGS__)

#define DBGT(entity_level, format, ... ) \
	LOGTIME(); \
	LOG(entity_level, LOG_DEBUG, format, ##__VA_ARGS__)

#else

#define LOG(entity_level, level, ...)
#define LOGT(entity_level, level, format, ...)
#define TRACE(entity_level, ... )
#define TRACET(entity_level, format, ...)
#define WRN(entity_level, ...)
#define WRNT(entity_level,  format, ... )
#define DBG(entity_level, ... )
#define DBGT(entity_level, format, ... ) 

#define LOGTIME()

#endif


 /* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
 
 /* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
// extern char AlarmNum;
extern uint8_t blPwrWuFlag;
extern uint8_t Pa0PinVal;

extern uint8_t pUartRxBuff[ 2 ];

extern void global_logger( int entity_level, const int level, const char * format, ... );

extern uint32_t mapDigitalValue( uint32_t val, uint32_t biggestVal, uint32_t lowestVal, uint32_t scalePoints );

extern int my_atoi( char* text, int len );
