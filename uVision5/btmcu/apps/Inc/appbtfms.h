/*! \file      appbtfms.h
    \version   
    \date      2017-07-18 18:25
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
 /* TimeOuts */
#define ESP_PWRON_STAB_TIME           100    /**< ms Seconds of power stabilization                            */
#define BT_ADC_SAMPLES                10     /**< The number of ADC sampling to understand watering is needed  */
#define BT_ADC_TO                     1000   /**< ms Seconds TO of ADC sampling                                */
#define BT_RST_LEAVE_TO               200    /**< ms Seconds TO wait until RST is OFF                          */
#define BT_BEACON_CFG_TO              20000  /**< ms Seconds TO to wait Beacon cfg value                       */
#define BT_MQTT_SEND_TO               20000  /**< ms Seconds wait until TO is reached                          */
#define BT_ERR_STATE_TO               5000   /**< ms Seconds in error state                                    */  

#define ACD_BATTERY_CHNL							ADC_CHANNEL_1   /**< ADC chanel for sensor 1 - overwatering  */

#define ON                    1
#define OFF                   0

#define BT_BEACON_MODE      0x30
#define BT_BELL_MODE        0x31
#define BT_CFG_MODE         0x32
 
 /* ------------------------------------------------------------------------------------------------
 *                                 Type definitions
 * ------------------------------------------------------------------------------------------------
 */
/**
	 Button FSM states
 */
enum {
	BT_IDLE= 0,					    /**< State after reset                                             */
  BT_CHECK_MODE,          /**< In this state running mode (Bell/Beacon/Cfg) is calculated    */
	BT_ESP_PWRON,			  	  /**< ESP power on and waits until stabilized                       */
	BT_ADC_BATTERY_CONTROL, /**< Check value of battery                                        */
	BT_ESP_RESET_LEAVE,			/**< Leave ESP reset and wait for TO                               */
	BT_BEACON_CFG_RECV,		  /**< Wait for beacon cfg from ESP. It is also mean ESP is started  */
  BT_MODE_SEND_WAIT_MQTT_RES,	/**< Send ESP Mode and waits for result -"Ok" or "Err"         */
  BT_ERR                 /**< */
};
 
enum{
  modeBeacon = BT_BEACON_MODE,
  modeBell = BT_BELL_MODE,
  modeCfg  = BT_CFG_MODE
};

 /* ------------------------------------------------------------------------------------------------
 *                                    Functions declarations
 * ------------------------------------------------------------------------------------------------
 */
extern uint8_t appBtFsmEvHandler(_tEQ* p);
