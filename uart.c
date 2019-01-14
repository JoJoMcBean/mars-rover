/**
 * @file uart.c
 * @brief functions for uart communication
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
#define baud 115200
#include "uart.h"
/**
 * Initialize uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_init(void){
    //enable clock to GPIO, R1 = port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    //enable clock to UART1, R1 = UART1. ***Must be done before setting Rx and Tx (See DataSheet)
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    //wait a bit before accessing device
    timer_waitMillis(1);
    //enable alternate functions on port b pins 0 and 1
    GPIO_PORTB_AFSEL_R |= (BIT0 | BIT1);
    //enable Rx and Tx on port B on pins 0 and 1
    GPIO_PORTB_PCTL_R |= 0x00000011;
    //set pin 0 and 1 to digital
    GPIO_PORTB_DEN_R |= (BIT0 | BIT1);
    //set pin 0 to Rx or input
    GPIO_PORTB_DIR_R &= ~BIT0;
    //set pin 1 to Tx or output
    GPIO_PORTB_DIR_R |= BIT1;

    //calculate baudrate
    uint16_t iBRD;
    uint16_t fBRD;

    switch(baud){//default 115200
        case 9600:
            iBRD = 0x68;
            fBRD = 0xB;
            break;

        case 115200:
            iBRD = 0x8;
            fBRD = 0x2c;
            break;

        default://same as 115200
            iBRD = 0x8;
            fBRD = 0x2c;
            break;
    }

    //turn off uart1 while we set it up
    UART1_CTL_R &= ~(UART_CTL_UARTEN);
    //set baud rate
    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;
    //set frame, 8 data bits, 1 stop bit, no parity, no FIFO
    UART1_LCRH_R = UART_LCRH_WLEN_8 ;
    //use system clock as source
    UART1_CC_R = UART_CC_CS_SYSCLK;
    //re-enable enable RX, TX, and uart1
    UART1_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);
}
/**
 * Transmit character
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_sendChar(char data){
    //wait until there is room to send data
    while(UART1_FR_R & 0x20);
    //send data
    UART1_DR_R = data;

}
/**
 * Receive character via uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
char uart_receive(void){
    char data = 0;
     //wait to receive
     while(UART1_FR_R & UART_FR_RXFE);//while fifo empty
    //mask the 4 error bits and grab only 8 data bits
    data = (char)(UART1_DR_R & 0xFF);

    return data;
}
/**
 * Send string via uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void uart_sendStr(const char *data){

    while(*data != '\0')//go until it reaches a null byte
    {
        uart_sendChar(*data);//send char
        data++;//increment pointer value ie next char in string
    }
}
