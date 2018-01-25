## RTL8370 LED module 
```
/* Function Name:
 *      rtk_led_enable_set
 * Description:
 *      Set Led parallel mode enable congiuration
 * Input:
 *      group - LED group id.
 *      portmask - LED enable port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can be used to enable LED parallel mode per port per group. 
 */

/* Function Name:
 *      rtk_led_enable_get
 * Description:
 *      Get Led parallel mode enable congiuration
 * Input:
 *      group - LED group id.
 * Output:
 *      pPortmask - LED enable port mask.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can be used to get LED parallel mode enable status. 
 */

/* Function Name:
 *      rtk_led_operation_set
 * Description:
 *      Set Led operation mode
 * Input:
 *      mode - LED operation mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      LED_OP_SCAN,        
 *      LED_OP_PARALLEL,        
 *      LED_OP_SERIAL, 
 */

/* Function Name:
 *      rtk_led_operation_get
 * Description:
 *      Get Led operation mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED operation mode.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get Led operation mode.
 *      The modes that can be set are as following:
 *      LED_OP_SCAN,        
 *      LED_OP_PARALLEL,        
 *      LED_OP_SERIAL,  
 */

/* Function Name:
 *      rtk_led_mode_set
 * Description:
 *      Set Led to congiuration mode
 * Input:
 *      mode - Support LED mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port.
 *      LED groups are set to indicate different information of port in different mode.
 *      
 *      Mode0
 *      LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *      LED2-100Mb/s Speed Indicator. Low for 100Mb/s.
 *      
 *      Mode1
 *      LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *      LED2-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *      
 *      Mode2
 *      LED0-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *      LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED2-10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      
 *      Mode3
 *      LED0-10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED2-100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.  
 */

/* Function Name:
 *      rtk_led_mode_get
 * Description:
 *      Get Led to congiuration mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED mode.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port. 
 *      LED groups is set to indicate different information of port in different mode.
 */

/* Function Name:
 *      rtk_led_modeForce_set
 * Description:
 *      Set Led group to congiuration force mode
 * Input:
 *      group - Support LED group id.
 *      mode - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can force all Leds in the same group to one force mode.
 *      The force modes that can be set are as following:
 *      LED_FORCE_NORMAL,
 *      LED_FORCE_BLINK,
 *      LED_FORCE_OFF,
 *      LED_FORCE_ON.  
 */

/* Function Name:
 *      rtk_led_modeForce_get
 * Description:
 *      Get Led group to congiuration force mode
 * Input:
 *      group - Support LED group id..
 * Output:
 *      pMode - Support LED force mode
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      LED_FORCE_NORMAL,
 *      LED_FORCE_BLINK,
 *      LED_FORCE_OFF,
 *      LED_FORCE_ON.  
 */

/* Function Name:
 *      rtk_led_blinkRate_set
 * Description:
 *      Set LED blinking rate 
 * Input:
 *      blinkRate - blinking rate.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */

/* Function Name:
 *      rtk_led_blinkRate_get
 * Description:
 *      Get LED blinking rate at mode 0 to mode 3
 * Input:
 *      None
 * Output:
 *      pBlinkRate - blinking rate.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are  6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      group - LED group.
 *      config - LED configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *      Definition  LED Statuses      Description
 *      0000        LED_Off           LED pin Tri-State.
 *      0001        Dup/Col           Collision, Full duplex Indicator.
 *      0010        Link/Act          Link, Activity Indicator. 
 *      0011        Spd1000           1000Mb/s Speed Indicator.
 *      0100        Spd100            100Mb/s Speed Indicator.
 *      0101        Spd10             10Mb/s Speed Indicator. 
 *      0110        Spd1000/Act       1000Mb/s Speed/Activity Indicator. 
 *      0111        Spd100/Act        100Mb/s Speed/Activity Indicator. 
 *      1000        Spd10/Act         10Mb/s Speed/Activity Indicator. 
 *      1001        Spd100 (10)/Act   10/100Mb/s Speed/Activity Indicator. 
 *      1010        Fiber             Fiber link Indicator. 
 *      1011        Fault             Auto-negotiation Fault Indicator. 
 *      1100        Link/Rx           Link, Activity Indicator. 
 *      1101        Link/Tx           Link, Activity Indicator. 
 *      1110        Master            Link on Master Indicator. 
 *      1111        Act               Activity Indicator. Low for link established.
 */

/* Function Name:
 *      rtk_led_groupConfig_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *       The API can get LED indicated information configuration for each LED group.
 */

/* Function Name:
 *      rtk_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
 * Input:
 *      active - LED group.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active congiuration.
 */

/* Function Name:
 *      rtk_led_serialMode_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *       The API can get LED serial mode active configuration.
 */

```