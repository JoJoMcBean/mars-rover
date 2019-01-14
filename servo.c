/**
 * @file servo.c
 * @brief contains functions for operating servo motor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
#include "servo.h"
#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"
#include "stdlib.h"
#include "button.h"

//(cybot#,delta,offset): (9,9.6289,454) (10,9.9111,576)

//delta and offset must be calibrated with measurement and a linear regression
//currently calibrated for cybot 8
double position = 0;
double ppos; //previous position
unsigned pulse_period = 0x4E200; //320000 aka 20 ms
int zero_offset = 549;//(8)441; //pulse width in us at 0 deg
double delta = 10.122;//(8)9.4722; //us high per degree change
//t_high = angle*delta + zero_offset
/**
 * Sets PWM wave pulse width
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @param int micro, time that PWM is in high state in microseconds
 */
void servo_setPulse(int micro){
    TIMER1_TBMATCHR_R = (pulse_period - micro*16) & 0xFFFF;
    TIMER1_TBPMR_R = (pulse_period - micro*16) >> 16;
}
/**
 * Configure PWM output to operate servo motor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_init(){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    //SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;//timer 1
    GPIO_PORTB_DEN_R |= 0b100000;//digital enable on pin b5
    GPIO_PORTB_DIR_R |= 0b100000;//set pin b5 to output
    GPIO_PORTB_PCTL_R |= 0x700000;//set b5 alternate function to Timer 1 CCP 1


///*

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;//timer 1
    TIMER1_CTL_R &= ~TIMER_CTL_TBEN;//disable timer B
    TIMER1_CFG_R = 0x4;//16 bit timer
    TIMER1_TBMR_R |= 0b1010;//PWM mode, edge count mode, periodic timer, count down
    //TIMER1_CTL_R |= 0b1000000;//probably don't invert
    TIMER1_TBILR_R = pulse_period & 0xFFFF; //2100 with interval 0.01 ms = 21 ms    //lower 16 bits of interval
    TIMER1_TBPR_R =  pulse_period >> 16; // 160 cycles = 0.01 ms per tick      //upper 8 bits of interval
    servo_setAngle(position);
    TIMER1_CTL_R |= TIMER_CTL_TBEN; //enable timer

    GPIO_PORTB_AFSEL_R |= 0b100000;//set pin b5 to alternate function
//*/
}
/**
 * Sets servo angle
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_setAngle(double a){
    servo_setPulse((int)(a*delta+zero_offset));
    int change = abs((int)(a*1000-ppos*1000));
    timer_waitMicros(change*5);
    ppos = a;
}
/**
 * Demonstrative function from servo lab
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_p1_main(){
    servo_init();
    lcd_init();
    int pos = 0;

    while(1){//proof of concept
        timer_waitMillis(500);
        servo_setPulse(pos*100);
        lcd_printf("%d",pos);//used for protractor calibration
        pos++;
    }
}
/**
 * Lets servo angle be adjusted by buttons
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_test(){
    servo_init();
    lcd_init();
    button_init();
    int dir = 1;
    servo_setAngle(position);
    while(1){
        lcd_printf("Position: %.2lf deg\nT-high: %.2lf us\nDirection: %d",position,(position*delta+zero_offset),dir);//display servo data
        switch(button_getButton()){//interpret button input
        case 1:
            position += dir*1;
            break;
        case 2:
            position += dir*2.5;
            break;
        case 3:
            position += dir*5;
            break;
        case 4:
            dir = -1*dir;
            break;
        }
        servo_setAngle(position);
        timer_waitMillis(200);
    }
}
/**
 * Function to let servo calibration constants be calculated
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void servo_calibrate(){
    servo_init();
    lcd_init();
    button_init();
    int dir = 1;
    int pp = 500;
    servo_setPulse(pp);
    while(1){
        lcd_printf("pulse: %d us\nDirection: %d",pp,dir);
        switch(button_getButton()){//interpret button input
        case 1:
            pp += dir*1;
            break;
        case 2:
            pp += dir*10;
            break;
        case 3:
            pp += dir*100;
            break;
        case 4:
            dir = -1*dir;
            break;
        }
        servo_setPulse(pp);
        timer_waitMillis(200);
    }
}
