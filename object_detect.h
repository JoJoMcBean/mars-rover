/**
 * @file object_detect.h
 * @brief contains functions for using ir, ping, and servo in conjunction to detect nearby objects
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 */
#ifndef OBJECT_DECTECT_H_
#define OBJECT_DETECT_H_


/**
 * Main function from lab 9 used to detect object and send information over uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void object_detect();
/**
 * This function is used by the cybot in the main project code to more scan the nearby environment over a 180 degree angle
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 * @param obj integer array of size 64 to be passed by reference and filled with information regarding detected objects
 */
void scan180(int obj[64]);

#endif /* MOVEMENT_H_ */
