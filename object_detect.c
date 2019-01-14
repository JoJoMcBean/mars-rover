/**
 * @file object_detect.c
 * @brief contains functions for using ir, ping, and servo in conjunction to detect nearby objects
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
#include <stdint.h>
#include "timer.h"
#include "lcd.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "tm4c123gh6pm.h"
#include "stdlib.h"

#include "servo.h"
#include "ir.h"
#include "ping.h"
#include "uart.h"
#include "button.h"
#include "movement.h"
#include "open_interface.h"

#define rad 0.01745329252 //decimal approximation of pi/180
/**
 * Main function from lab 9 used to detect object and send information over uart
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void object_detect() {
    servo_setAngle(90);
    typedef struct object{
        int angle1;//first detected angle
        int angle2;//last detected angle
        int radius;//first detection
        int detect; //0 or 1 if
        double width;
    } object;
    object obj[20];
    obj[0].detect = 0;
    int numObj = 0;
    char s[50];
    int x = 0;
    uart_sendChar('\r');
    uart_sendChar('\n');
    sprintf(s,"angle\tIR\tSONAR\r\n");
    uart_sendStr(s);
    int ang = 0;
    int ir;
    int ping;
    int prevIR = 0;
    double smallWidth = 10000000;
    int smallNum = -1;
    timer_waitMillis(500);
    ang = 0;
    smallNum = -1;
    smallWidth = 100000;
    prevIR = 0;
    while (ang <= 180){
        servo_setAngle(ang);
        ping = ping_read();
        ir = ir_read();
        timer_waitMillis(8);
        ir += ir_read();
        timer_waitMillis(8);
        ir += ir_read();
        timer_waitMillis(8);
        ir += ir_read();
        ir = ir/4;
        sprintf(s,"%d\t%d\t%d",ang,ir,ping);
        if((obj[numObj].detect && ir > 0.8*ping && ir < 1.3*ping) //if an object is detected, keep it OR
                || (ir < 0.7*prevIR && (ir/prevIR)*ir < ping && ping < 100 && ir < 100 && 1.05*ir > ping && ir < 1.5*ping) //if there is suddenly a big change in IR, call it OR
                || (ir > 0.9*ping && ir < 1.1*ping && ping < 100 && ir < 100)){ //if the values are very close and within range, it's probably an object

            obj[numObj].angle2 = ang;
            if(! obj[numObj].detect){
                obj[numObj].detect = 1;
                obj[numObj].angle1 = ang;
                obj[numObj].radius = ping;
            }
            sprintf(s,"%s\tobject detected #%d\r\n",s,numObj);
        }
        else {
            sprintf(s,"%s\r\n",s);
            if(obj[numObj].detect){
                obj[numObj].detect = 0;
                obj[numObj].width = (1 + (obj[numObj].angle2) - (obj[numObj].angle1))*(obj[numObj].radius)*rad;
                if(obj[numObj].width < smallWidth && obj[numObj].angle1 != obj[numObj].angle2){
                    smallNum = numObj;
                    smallWidth = obj[numObj].width;
                    lcd_printf("Smallest Object:\nObject %d r=%d\nAngular size: %d deg\nLinear size: %lf cm",smallNum,obj[smallNum].radius,(1+obj[smallNum].angle2-obj[smallNum].angle1),obj[smallNum].width);
                }
                if(obj[numObj].angle1 != obj[numObj].angle2)
                    numObj++;
                obj[numObj].detect = 0;
            }
        }

        uart_sendStr(s);
        timer_waitMillis(50);
        ang += 1;
        prevIR = ir;
    }

    for(x = 0; x < numObj;x++){
        if(obj[x].angle1 == obj[x].angle2){
            sprintf(s,"Object %d: DUD\r\n",x);
        } else {

            sprintf(s,"Object %d: detected from %d-%d deg at distance %d cm. Width: %lf cm\r\n",x,obj[x].angle1,obj[x].angle2,obj[x].radius,obj[x].width);
        }
        uart_sendStr(s);
    }
    for(x = 0; x < numObj; x++){
        servo_setAngle((obj[x].angle1 + obj[x].angle2)/2);
        timer_waitMillis(500);
    }
    numObj = 0;
    if(smallNum >= 0)
        servo_setAngle((obj[smallNum].angle1 + obj[smallNum].angle2)/2);
}

/**
 * This function is used by the cybot in the main project code to more scan the nearby environment over a 180 degree angle
 * @author Jordan Fox, Scott Beard,  Daksh Goel, James Volpe
 * @data 12/2/2018
 * @param obj integer array of size 64 to be passed by reference and filled with information regarding detected objects
 */
void scan180(int obj[64]){//16 objects for object n, detect is at index 4*n, angle1 at 4*n+1, angle2 at 4*n+2, radius at 4*n+3
    char s[50];
    int numObj = 0, ang = 0, ir = 0, ping = 0, prevIR = 0;
    for(ang = 0; ang < 64; ang++)
        obj[ang] = 0;

    sprintf(s,"\r\nAngle\tIR\tSONAR\r\n");
    uart_sendStr(s);

    for(ang = 0; ang <= 180; ang++){
        servo_setAngle(ang);

        ir = ir_read();
        timer_waitMillis(4);
        ir += ir_read();
        ping = ping_read();
        timer_waitMillis(4);
        ir += ir_read();
        timer_waitMillis(4);
        ir += ir_read();
        ir = ir/4;
        sprintf(s,"%d\t%d\t%d",ang,ir,ping);
        /*if((obj[numObj*4] && ir > 0.8*ping && ir < 1.3*ping) //if an object is detected, keep it OR
                || (ir < 0.7*prevIR && (ir/prevIR)*ir < ping && ping < 100 && ir < 100 && 1.05*ir > ping && ir < 1.5*ping) //if there is suddenly a big change in IR, call it OR
                || (ir > 0.9*ping && ir < 1.1*ping && ping < 100 && ir < 100)){ //if the values are very close and within range, it's probably an object
                */
        if(!obj[numObj*4] && prevIR > 100 && ir < 100 && ping < 80
                || obj[numObj*4] && ir < 100 && (obj[numObj*4+3] - 5) < ping && (obj[numObj*4+3] + 5) > ping){

            obj[numObj*4+2] = ang;

            if(!obj[numObj*4]){
                obj[numObj*4] = 1;
                obj[numObj*4+1] = ang;
                obj[numObj*4+3] = ping;
            }
            sprintf(s,"%s\tobject detected #%d\r\n",s,numObj);
        }
        else {
            sprintf(s,"%s\r\n",s);
            if(obj[numObj*4]){
                if(obj[numObj*4+1] != obj[numObj*4+2])
                    numObj++;
                obj[numObj*4] = 0;
            }
        }

        uart_sendStr(s);
        timer_waitMillis(50);
        ang += 1;
        prevIR = ir;
    }

    servo_setAngle(90);
}




