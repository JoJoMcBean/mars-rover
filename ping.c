/**
 * @file ping.c
 * @brief contains functions for using the cybot ping))) sensor
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */

#include "ping.h"
#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"
#include <inc/tm4c123gh6pm.h>


volatile unsigned long long startTime;
volatile unsigned long long endTime;
volatile int lastPing;

/**
 * This function handles ping interrupts for ping sensor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void TIMER3B_Handler(void){
    //check level

    if(startTime == 0){
        startTime = endTime;
        endTime = TIMER3_TBR_R;//register holds time at last interrupt, ie now
    }
    TIMER3_ICR_R = 0x400;

}
//set actual period to 21 ms aka 336000 ticks
/**
 * This function configures the cybot to use the ping sensor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_init() {
    //initialize
    //
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;//gpio port b
    ///*
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;//timer 3
    TIMER3_CTL_R &= ~(TIMER_CTL_TBEN);//disable timer to configure
    TIMER3_CTL_R |= TIMER_CTL_TBEVENT_BOTH;//enable Trigger on both edges
    TIMER3_CFG_R |= TIMER_CFG_16_BIT; //set to 16 bit timer
    TIMER3_TBMR_R = 0b000000010111; //count up, edge-time mode, capture mode
    TIMER3_TBILR_R = 0xFFFF; //value to count down from
    TIMER3_ICR_R = 0x400; //clears TIMER3 time-out interrupt flags
    TIMER3_IMR_R |= 0b10000000000; //enable timer b capture mode event interrupt
    NVIC_EN1_R |= 0x10;//enable interrupt for timer 3b
    //SET pin B3 as digital output
    IntRegister(INT_TIMER3B, TIMER3B_Handler); //register TIMER3B interrupt handler

    IntMasterEnable(); //Initialize global interrupts

    TIMER3_CTL_R |= TIMER_CTL_TBEN; //Enable TIMER3B
    //*/


    GPIO_PORTB_DEN_R |= 0b00001000;//digital enable for pin 3
    GPIO_PORTB_DIR_R &= 0b11110111;//pin 3 input
    GPIO_PORTB_AFSEL_R |= 0b00001000;//alternate function for pin 3
    GPIO_PORTB_PCTL_R |= 0x00007000; //pin b3 timer 3 capture/compare/PWMv (CCP) 1







}
int OF_count = 0;
/**
 * This function operates the ping sensor to emit a pulse
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_sendPulse(){
    //pin b3 = digital input, afsel = 0
    GPIO_PORTB_DIR_R |= 0b00001000;//output
    GPIO_PORTB_AFSEL_R &= ~(0b00001000);//AF off
    GPIO_PORTB_DATA_R |= 0x08;//pin b3 high
    timer_waitMicros(5);//wait
    GPIO_PORTB_DATA_R &= ~(0x08);//pin b3 low
    GPIO_PORTB_DIR_R &= ~(0b00001000);//input
    GPIO_PORTB_AFSEL_R |= 0b00001000;//AF on
}
//assume ping is initialized
/**
 * This function emits a pulse, records the time for it to return
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return time for pulse to return in clock cycles ie 16th of a microsecond
 */
int ping_pulse() {
    ping_sendPulse();
    startTime = 0;
    endTime = 0;//reset values
    while(startTime == 0);//at first interrupt. ET -> ST so ST = 0, only at the second interrupt (falling edge) is ST > 0

    if(endTime > startTime){
        return endTime - startTime;
    }else{
        OF_count++;
        return (endTime + 0x00FFFFFF) - startTime;//account for overflow, 24 bit timer
    }

}
/**
 * This function calculates actual distance to nearby object in cm using other ping functions
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return distance to nearby object in cm
 */
int ping_read(){
    return ping_pulse()*0.001071875;
}

//part 1
/**
 * This function from the ping lab sends out a sonic pulse every 500 milliseconds
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_p1_main(){
    ping_init();
    while(1)//send flash every half second
    {
        ping_sendPulse();
        timer_waitMillis(400);
    }
}

//part 3
/**
 * This function demonstrates ping code by measuring distance every 500 milliseconds and displaying it on the LCD screen
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ping_test() {
    lcd_init();
    ping_init();
    while(1){
        int time = ping_pulse()/16;//microseconds
        int dist = time*0.01715;//speed of sound in cm per us, divided by 2
        lcd_printf("time: %d us\ndist: %d cm\nOverflow: %d",time,dist,OF_count);
        timer_waitMillis(200);//send pulse every half second
    }
}
/**
 * This function was developed for the project so it may passively measure distance
 * Checks if recently emitted pulse has returned, if so calculates distance
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 * @return measured distance in cm or zero if return pulse has not been received yet
 */
int ping_check(){
    if(startTime != 0){
        if(endTime > startTime){
            return (endTime - startTime)*0.001071875;
        }else{
            return ((endTime + 0x00FFFFFF) - startTime)*0.001071875;//account for overflow, 24 bit timer
        }
    } else
        return 0;
}
/**
 * This function is meant to be used in conjunction with ping_check to prepare variables for measurement
 * This function used with ping_pulse and ping_check can passively measure distance without use of a while loop which stalls other project functions
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void ping_ready(){
    startTime = 0;
    endTime = 0;//reset values
}
