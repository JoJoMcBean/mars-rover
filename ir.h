/**
 * @file ir.h
 * @brief this header file contains functions for using the ADC to measure distance using the cybot infrared sensor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */

#ifndef IR_H_
#define IR_H_

#include "tm4c123gh6pm.h"
#include "lcd.h"
#include "timer.h"
/**
 * This function configures the processor to use the ADC
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_init();
/**
 * This function activates ADC conversion
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return approximated voltage output from ir sensor
 */
short ir_pulse();
/**
 * This function interprets ADC data to calculate distance
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return distance in cm
 */
int ir_read();
/**
 * This function automates calibration process for a using a novel cybot
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return calculated variable CALI used in distance calculation
 */
double ir_calibrate();
/**
 * This function demonstrates ir measurement and can be used to observe accuracy
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_test();
/**
 * This function is for calibration and sends data over putty so it may be interpreted by other softwares
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_putty();
/**
 * This function calibrates the ir sensor using the ping sensor which is more precise
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_ping();
/**
 * This function is used to read ir data directly to hand calibrate and test different calibration factors
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_hand();

#endif /* IR_H_ */
