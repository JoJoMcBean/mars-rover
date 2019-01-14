/**
 * @file ping.h
 * @brief contains functions for using the cybot ping))) sensor
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */


#ifndef PING_H_
#define PING_H_

#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"

/**
 * This function configures the cybot to use the ping sensor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_init();
/**
 * This function operates the ping sensor to emit a pulse
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_sendPulse();
/**
 * This function emits a pulse, records the time for it to return
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return time for pulse to return in clock cycles ie 16th of a microsecond
 */
int ping_pulse();
/**
 * This function calculates actual distance to nearby object in cm using other ping functions
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return distance to nearby object in cm
 */
int ping_read();
/**
 * This function from the ping lab sends out a sonic pulse every 500 milliseconds
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_p1_main();
/**
 * This function demonstrates ping code by measuring distance every 500 milliseconds and displaying it on the LCD screen
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_test();
/**
 * This function was developed for the project so it may passively measure distance
 * Checks if recently emitted pulse has returned, if so calculates distance
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 * @return measured distance in cm or zero if return pulse has not been received yet
 */
int ping_check();
/**
 * This function is meant to be used in conjunction with ping_check to prepare variables for measurement
 * This function used with ping_pulse and ping_check can passively measure distance without use of a while loop which stalls other project functions
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void ping_ready();



#endif /* PING_H_ */
