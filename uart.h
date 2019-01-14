/**
 * @file uart.h
 * @brief functions for uart communication
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */

#ifndef UART_H_
#define UART_H_

#include "Timer.h"
//#include "WiFi.h"
#include <inc/tm4c123gh6pm.h>
/**
 * Initialize uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_init(void);
/**
 * Transmit character
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_sendChar(char data);
/**
 * Receive character via uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
char uart_receive(void);
/**
 * Send string via uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_sendStr(const char *data);


#endif /* UART_H_ */
