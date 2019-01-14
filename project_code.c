/**
 * @file project_code.c
 * @brief main code for project
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
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
#include "movement.h"
#include "open_interface.h"
#include "object_detect.h"

//to calibrate: ir,servo,cliffSignal,oi_angle,oi_cliffSignal,wheel_speed,

#define rad 0.017453292519943
#define mSpeed 100
#define tSpeed 40
#define edgeThresh 2640 //determine better values for these via testing, may need more specialized values for each sensor
#define hype 47 //diagonal across rectangular grid, must be measured before testing
/*
    blank ' '
    roomba 'R'
    object 'B'
    cliff 'C'
    edge 'G'
    unexplored '*'
*/
char map[2*hype][2*hype];
char str[100];
volatile double xPos;//cords work however we want so 0,0 is bottom left, its, (x,y) and +y is up and +x is right
volatile double yPos;//in dm decimeter (1/10 m)
volatile double heading;//ccw from +x direction in degrees
volatile double distanceDelta;
volatile double angleDelta;
volatile int moving; //0 or 1 conditional
volatile int turning;
/**
 * Main method for project execution
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void main()
{
    //initialize cybot
    servo_init();
    lcd_init();
    ping_init();
    ir_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    uart_init();

    oi_setWheels(0,0);
    servo_setAngle(90);

    moving  = 0;
    turning = 0;
    heading = 90;
    distanceDelta = 0;
    angleDelta = 0;
    xPos = hype;
    yPos = hype;

    map_init();
    music_init();

    char input = '~';
    int danger;
    //indicate when cybot is ready for user input
    sprintf(str,"\r\nInitialized!\r\nBattery at %d/%d\r\n",sensor_data->batteryCharge,sensor_data->batteryCapacity);
    uart_sendStr(str);
    //primary while loop
    while(1){
        //update open interface sensor
        oi_update(sensor_data);
        if(button_getButton() == 6){
            oi_free(sensor_data);
            exit(0);
        }
        if(!(UART1_FR_R & UART_FR_RXFE)){//TEST THIS
            input = (char)(UART1_DR_R & 0xFF);
            uart_sendChar(input);
        }
        if(moving ==1 && ping_check()){
            if(ping_check() <= 20){
                oi_setWheels(0,0);
                input = ' ';
                sprintf(str,"\r\nobject imminent");
                uart_sendStr(str);
            }
            ping_sendPulse();
            ping_ready();
        }
        if(moving==1 && check_cliff(sensor_data)){
            oi_setWheels(0,0);
            danger =  check_cliff(sensor_data);
			sprintf(str,"\r\ncliff detected at: ");
			if(danger & 0x8)
			    sprintf(str,"%sLeft, ",str);
            if(danger & 0x4)
                sprintf(str,"%sFront Left, ",str);
            if(danger & 0x2)
                sprintf(str,"%sFront Right, ",str);
            if(danger & 0x1)
                sprintf(str,"%sRight, ",str);
			uart_sendStr(str);
			map[(int)xPos][(int)yPos] = 'C';
			input = ' ';
		}
        if(moving==1 && check_edge(sensor_data)){
            oi_setWheels(0,0);
            danger = check_edge(sensor_data);
            sprintf(str,"\r\nedge detected at: ");
            if(danger & 0x8)
                sprintf(str,"%sLeft, ",str);
            if(danger & 0x4)
                sprintf(str,"%sFront Left, ",str);
            if(danger & 0x2)
                sprintf(str,"%sFront Right, ",str);
            if(danger & 0x1)
                sprintf(str,"%sRight, ",str);
            uart_sendStr(str);
            map[(int)xPos][(int)yPos] = 'G';
            input = ' ';
        }
        if(moving==1 && check_bump(sensor_data)){
            oi_setWheels(0,0);
            danger = check_bump(sensor_data);
            sprintf(str,"\r\nbump detected! ");
            if(danger & 0x2)
                sprintf(str,"%sLeft, ",str);
            if(danger & 0x1)
                sprintf(str,"%sRight, ",str);
            uart_sendStr(str);
            map[(int)xPos][(int)yPos] = 'L';
            input = ' ';
        }
		if(input != '~'){
			switch(input){
				case 'w' :
					if(!moving && !turning){
					    update_position();
						oi_setWheels(mSpeed, mSpeed);
						moving = 1;
						//maybe send a putty message
					}
					break;
				case 'a' :
					if(!moving && !turning){
					    update_position();
						oi_setWheels(tSpeed,-tSpeed);
						turning = 1;
					}
					break;
				case 's' :
					if(!moving && !turning){
					    update_position();
						oi_setWheels(-mSpeed,-mSpeed);
						moving = 2;//used in danger detection
					}
					break;
				case 'd' :
					if(!moving && !turning){
					    update_position();
						oi_setWheels(-tSpeed,tSpeed);
						turning = 1;
					}
					break;
				case ' ' :
					oi_setWheels(0, 0);
					if(moving){
						distanceDelta += sensor_data->distance;//convert mm to dm
						moving = 0;
					}
					if(turning){
						angleDelta += sensor_data->angle;
						turning = 0;
					}
					update_position();
					break;
				case 'c' :
					if(!moving && !turning)
						scan1();
					break;
				case 'v' :
				    scan2();
				    break;
				case 'z' :
				    draw_map();
				    break;
				case 'x' :
				    draw_heading();
				case 'l':
					lcd_printf("please love me");
					break;
				case 'm':
					//play music
				    oi_play_song(1);
					break;
				case 'b' :
				    sprintf(str,"\r\nBattery at %d/%d",sensor_data->batteryCharge,sensor_data->batteryCapacity);
				    uart_sendStr(str);
			}
			input = '~';
		}
		if(moving)
		    distanceDelta += sensor_data->distance;
		if(turning)
		    angleDelta += sensor_data->angle;
		if(moving == 2)
		    oi_play_song(2);


		//lcd_printf("L: %d\nFL: %d\nFR: %d\nR: %d",sensor_data->cliffLeftSignal,sensor_data->cliffFrontLeftSignal,sensor_data->cliffFrontRightSignal,sensor_data->cliffRightSignal);
		lcd_printf("Battery: %d/%d",sensor_data->batteryCharge,sensor_data->batteryCapacity);
    }


}
/**
 * Primary object detection scan, using ping and ir, display data on uart
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void scan1(){
    int *s = malloc(64*sizeof(int));
    int n = 0, objX,objY,ang=0,dist=0,m=0,tg=0,nw = 0,mw=0;
    for(ang = 0; ang <= 180; ang += 5){
        for(dist = 0; dist < 50; dist += 5){
            if(map[(int)(xPos + dist*cos((heading-90+ang)*rad)/10)][(int)(yPos + dist*sin((heading-90+ang)*rad)/10)] == '#' | map[(int)(xPos + dist*cos((heading-90+ang)*rad)/10)][(int)(yPos + dist*sin((heading-90+ang)*rad)/10)] == 'B')
                map[(int)(xPos + dist*cos((heading-90+ang)*rad)/10)][(int)(yPos + dist*sin((heading-90+ang)*rad)/10)] = ' ';
        }
    }
    scan180(s);//for obj[n] use *(s+4*n)

    for(n = 0; n < 16 && s[4*n]; n++){
        nw = (s[4*n+3]*(s[4*n+2]-s[4*n+1]))*rad;
        sprintf(str,"\r\nObject %d width: %d",n,nw);
        uart_sendStr(str);
    }
    for(n=0;s[4*n] && n<16;n++){
        nw = (s[4*n+3]*(s[4*n+2]-s[4*n+1]))*rad;
        objX = xPos + *(s+4*n+3)*cos((heading-90+*(s+4*n+1))*rad)/10;
        objY = yPos + *(s+4*n+3)*sin((heading-90+*(s+4*n+1))*rad)/10;
        if(objX >= 0 && objX <= 2*hype && objY >= 0 && objY <= 2*hype)
            map[objX][objY] = 'B';
        objX = xPos + *(s+4*n+3)*cos((heading-90+*(s+4*n+2))*rad)/10;//repeat with angle 2, will probably overlap unless large angular width
        objY = yPos + *(s+4*n+3)*sin((heading-90+*(s+4*n+2))*rad)/10;
        if(objX >= 0 && objX <= 2*hype && objY >= 0 && objY <= 2*hype)
            map[objX][objY] = 'B';
        for(m=n+1;s[4*m] && m<16; m++){
            mw = (s[4*m+3]*(s[4*m+2]-s[4*m+1]))*rad;
            tg = ((int)sqrt((s[4*n+3]+1)*(s[4*n+3]+1)+(s[4*m+3]+1)*(s[4*m+3]+1)-2*(s[4*n+3]+1)*(s[4*n+3]+1)*cos(rad*(((s[4*m+1]+s[4*m+2])/2)-((s[4*n+1]+s[4*n+2])/2)))));
            //sprintf(str,"\r\nFor objects %d & %d: %d, %d, %d, %d",n,m,(int)sqrt((*(s+4*n+3))*(*(s+4*n+3))+(*(s+4*m+3))*(*(s+4*m+3))-2*(*(s+4*n+3))*(*(s+4*m+3))*cos(rad*((*(s+4*m+1))-(*(s+4*n+1))))),(int)sqrt((*(s+4*n+3))*(*(s+4*n+3))+(*(s+4*m+3))*(*(s+4*m+3))-2*(*(s+4*n+3))*(*(s+4*m+3))*cos(rad*((*(s+4*m+2))-(*(s+4*n+1))))),(int)sqrt((*(s+4*n+3))*(*(s+4*n+3))+(*(s+4*m+3))*(*(s+4*m+3))-2*(*(s+4*n+3))*(*(s+4*m+3))*cos(rad*((*(s+4*m+1))-(*(s+4*n+2))))),(int)sqrt((*(s+4*n+3))*(*(s+4*n+3))+(*(s+4*m+3))*(*(s+4*m+3))-2*(*(s+4*n+3))*(*(s+4*m+3))*cos(rad*((*(s+4*m+2))-(*(s+4*n+2))))));
            sprintf(str,"\r\nDistance between objects %d--%d: %d",n,m,tg);
            if(tg > 53 && tg < 68 && nw > 3 && nw < 8 && mw > 3 && mw < 8 )
                sprintf(str,"%s likely to be posts");
            uart_sendStr(str);
        }
    }

    free(s);
    //
}
/**
 * Secondary scan using only ir, display data visually iva uart
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void scan2(){
    double ang = 0,pang;
    int ir = 0, x= 0;
    for(ang = 0; ang <= 180; ang+=0.25){
        servo_setAngle(ang);
        ir = ir_read();

        if(ir < 100){
            timer_waitMillis(2);
            ir = ir_read();
            timer_waitMillis(2);
            ir += ir_read();
            timer_waitMillis(2);
            ir += ir_read();
            timer_waitMillis(2);
            ir += ir_read();
            ir = ir/4;
            if(ir < 100){
                pang = ang;
                sprintf(str,"\r\n%.2lf:",ang);
                for(x = 0; x < ir; x+=2)
                    sprintf(str,"%s ",str);
                sprintf(str,"%s|#",str);
                uart_sendStr(str);
            }
        }
        if((ang-pang) == 4){
            sprintf(str,"\r\n");
            uart_sendStr(str);
        }
    }
    sprintf(str,"\r\n");
    uart_sendStr(str);
    servo_setAngle(90);
}

/**
 * Check for dropoff
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 * @param sensor_data oi sensor to use
 * @return int danger with each bit indicating which sensor is being triggered
 */
int check_cliff(oi_t *sensor_data){//return 4 conditionals as a 4 bit number {L,FL,FR,R}

	int danger = 0;
	if(sensor_data->cliffLeft)
		danger += 8;
	if(sensor_data->cliffFrontLeft)
		danger += 4;
	if(sensor_data->cliffFrontRight)
		danger += 2;
	if(sensor_data->cliffRight)
		danger += 1;
	
	return danger;
}
/**
 * Check for edge
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 * @param sensor_data oi sensor to use
 * @return int danger with each bit indicating which sensor is being triggered
 */
int check_edge(oi_t *sensor_data){//return 4 conditionals as a 4 bit number {L,FL,FR,R}

	int danger = 0;
	if(sensor_data->cliffLeftSignal >= edgeThresh)
		danger += 8;
	if(sensor_data->cliffFrontLeftSignal >= edgeThresh)
		danger += 4;
	if(sensor_data->cliffFrontRightSignal >= edgeThresh)
		danger += 2;
	if(sensor_data->cliffRightSignal >= edgeThresh)
		danger += 1;
	
	return danger;
}
/**
 * Check for bump
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 * @param sensor_data oi sensor to use
 * @return int danger with each bit indicating which sensor is being triggered
 */
int check_bump(oi_t *sensor_data){//return 2 conditionals as a 2 bit number {L,R}
		
	int danger = 0;
	if(sensor_data->bumpLeft)
		danger += 2;
	if(sensor_data->bumpRight)
		danger += 1;
	
	return danger;
}
/**
 * update position and heading and tell user about recent movement
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void update_position(){
    if(angleDelta){
        heading+=angleDelta*1.3;
        sprintf(str,"\r\nturned %d deg ccw",(int)(angleDelta*1.3));
        uart_sendStr(str);
        angleDelta = 0;
        while(heading >= 360)
            heading -= 360;
        while(heading < 0)
            heading += 360;
    }
    if(distanceDelta){
        sprintf(str,"\r\nmoved %d cm forward",(int)(distanceDelta/10));
        uart_sendStr(str);
        distanceDelta = distanceDelta/100;//convert mm to dm
        xPos += distanceDelta*cos(rad*heading);
        yPos += distanceDelta*sin(rad*heading);
        distanceDelta = 0;
    }
    //need some sin and cosine shit
}
/**
 * Draw GUI map
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void draw_map(){
    uart_sendChar('\r');
    uart_sendChar('\n');
    int x = 0,y=0;
    for(x = 0; x < 2*hype+2; x++){
        uart_sendChar('-');
    }
    uart_sendChar('\r');
    uart_sendChar('\n');
    for(y= 2*hype-1;y>=0;y--){
        uart_sendChar('|');
        for(x = 0; x < 2*hype; x++){
            if(x == (int)xPos && y == (int)yPos)
                uart_sendChar('R');
            else
                uart_sendChar(map[x][y]);
        }
        uart_sendChar('|');
        uart_sendChar('\r');
        uart_sendChar('\n');
    }
    for(x = 0; x < 2*hype+2; x++){
        uart_sendChar('-');
    }
    uart_sendChar('\r');
    uart_sendChar('\n');
}
/**
 * set up map and fill with unexplored region
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void map_init(){
    int x= 0,y=0;
    for(y= 0;y<2*hype;y++){
        for(x = 0; x < 2*hype; x++){
            map[x][y] = '#';
        }
    }
}
/**
 * indicate current heading relative to initialization
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void draw_heading(){
    sprintf(str,"\r\nHeading is %.0lf degrees ccw of +x direction\r\nPosition is (%.2lf,%.2lf) relative to bottom left\r\n",heading,xPos,yPos);
    uart_sendStr(str);
}
/**
 * Load songs via open interface
 * @author Jordan Fox, Scott Beard, Daksh Goel, James Volpe
 * @date 12/2/2018
 */
void music_init(){
    unsigned char backupNotes[2]    = {84,0};
    unsigned char backupDuration[4] = {32, 32};
    oi_loadSong(2, 2, backupNotes, backupDuration);

    unsigned char succNotes[7] = {57,0,58,0,59,0,60};
    unsigned char succDuration[7] = {20,4,24,4,24,4,48};
    oi_loadSong(1, 7, succNotes, succDuration);
}
