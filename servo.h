/**
 * @file servo.h
 * @brief contains functions for operating servo motor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"
/**
 * Sets PWM wave pulse width
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @param int micro, time that PWM is in high state in microseconds
 */
void servo_setPulse(int micro);
/**
 * Configure PWM output to operate servo motor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_init();
/**
 * Sets servo angle
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_setAngle(double a);
/**
 * Demonstrative function from servo lab
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_p1_main();
/**
 * Lets servo angle be adjusted by buttons
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_test();
/**
 * Function to let servo calibration constants be calculated
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_calibrate();


#endif /* SERVO_H_ */
