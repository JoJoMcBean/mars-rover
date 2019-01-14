/**
 * @file ir.c
 * @brief this C file contains functions for using the ADC to measure distance using the cybot infrared sensor
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */



#include "ir.h"
#include "tm4c123gh6pm.h"
#include "lcd.h"
#include "timer.h"
#include "uart.h"
#include "servo.h"
#include "ping.h"
#include "button.h"

#define CALI 38647//15077(8)//50327
//(#,CALI): (9,43681) (11,31770)
//cali must be calibrated for accurate use
//currently calibrated for cybot 8
/**
 * This method is designed to automatically handle ADC interrupts but it is currently disabled and unused
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_Handler(void)//RUN WHEN INTERRUPT IS TRIGGERED triggered in completion of ADC0SS0
{
    ADC0_ISC_R=ADC_ISC_IN0;// nothing will happen when interrupt method is triggered
    return;
    //initiate SS0 conversion
    ADC0_PSSI_R=ADC_PSSI_SS0;
    //wait for ADC conversion to be complete
    while((ADC0_RIS_R & ADC_RIS_INR0) == 0){
    //wait
    }
    //clear interrupt
    ADC0_ISC_R=ADC_ISC_IN0;//RESET INTERRUPT SO IT CAN BE RUN AGAIN
}


/**
 * This function configures the processor to use the ADC
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_init()
{
    //GPIO STUFF
    //enable ADC 0 module on port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    //enable clock for ADC
    SYSCTL_RCGCADC_R |= 0x1;
    //enable port B pin 4 to work as alternate functions
    GPIO_PORTB_AFSEL_R |= 0x10;
    //set to input - pin 4
    GPIO_PORTB_DIR_R &= 0b11101111;
    //set pin to input - 4 disable digital
    GPIO_PORTB_DEN_R &= 0b11101111;
    //disable analog isolation for the pin
    GPIO_PORTB_AMSEL_R |= 0x10;
    //initialize the port trigger source as processor (default)
    GPIO_PORTB_ADCCTL_R = 0x00;



    //ADC STUFF
    //disable SS0 sample sequencer to configure it
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN0;
    //initialize the ADC trigger source as processor (default)
    ADC0_EMUX_R = ADC_EMUX_EM0_PROCESSOR;
    //set 1st sample to use the AIN10 ADC pin
    ADC0_SSMUX0_R |= 0x000A;
    //enable raw interrupt status & establish 1 sample per sequence
    ADC0_SSCTL0_R |= (ADC_SSCTL0_IE0 | ADC_SSCTL0_END0);
    //enable oversampling to average - 16x
    ADC0_SAC_R |= ADC_SAC_AVG_16X;
    //re-enable ADC0 SS0
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN0;

    /*
    //INTERRUPT STUFF
    //clear interrupt flags
    ADC0_ISC_R |= ADC_ISC_IN0;
    //enable ADC0SS0 interrupt
    ADC0_IM_R |= ADC_IM_MASK0;
    //enable interrupt for IRQ 14 set bit 14 for ADC0 sequence 1
    NVIC_EN0_R |= 0x00004000;
    //tell cpu to use ISR handler for ADC0SS0
    IntRegister(INT_ADC0SS0, ir_Handler);
    //enable global interrupts
    IntMasterEnable();
    */
}
/*
//initiate SS1 conversion
ADC0_PSSI_R=ADC_PSSI_SS1;
//wait for ADC conversion to be complete
while((ADC0_RIS_R & ADC_RIS_INR1) == 0){
//wait
}
//grab result
int value = ADC0_SSFIFO1_R;
*/
/**
 * This function activates ADC conversion
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return approximated voltage output from ir sensor
 */
short ir_pulse(){
    //disable ADC0SS0 sample sequencer to configure it
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN0;
    //set 1st sample to use the channel ADC pin
    ADC0_SSMUX0_R |= 0x000A;
    //re-enable ADC0 SS0
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN0;
    //initiate SS0 conversion
    ADC0_PSSI_R=ADC_PSSI_SS0;
    //wait for ADC conversion to be complete
    while((ADC0_RIS_R & ADC_RIS_INR0) == 0){}
    //clear interrupt
    ADC0_ISC_R=ADC_ISC_IN0;
    return ADC0_SSFIFO0_R & 0x0FFF; //return value between 0 and 4096
}

/**
 * This function interprets ADC data to calculate distance
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return distance in cm
 */
int ir_read(){


    return CALI/ir_pulse();
}

/**
 * This function automates calibration process for a using a novel cybot
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 * @return calculated variable CALI used in distance calculation
 */
double ir_calibrate()
{
    ir_init();
    lcd_init();

    //assume lcd is initialzied
    double scans[10];
    int dist = 1;
    int mnum = 0;
    int sum;
    //measure stuff
    while(dist <= 10)
    {//countdown before measurment
        lcd_printf("measuring %d cm in 5 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 4 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 3 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 2 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 1 second",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring...");
        mnum = 0;
        sum = 0;
        while(mnum < 16)//measure 16 times and average
        {

            sum = sum + ir_pulse();
            timer_waitMillis(100);
            mnum++;
        }
        scans[dist-1] = sum/16.0;//average samples
        dist++;
    }
    //calculate P
    //P value is calculated such that P/x = distance where x is the 12 bit value from the ADC register
    double r1 = 10000;//best and second best r values
    double r2 = 100000;//not actually r more like a sum of percent error
    int inc = 1000;
    double p1 = 1;//best and second best p values
    double p2 = 50000;
    double test;//test P
    double high;
    double low;//range of test values
    double rval;//percent error
    double tval;//value to be compared to scans array
    while(inc >= 1)
    {
        if(p1 > p2)//set low as lower of p1 and p2
        {
            low = p2;
            high = p1;
        }
        else
        {
            low = p1;
            high = p2;
        }
        test = low;
        while(test <= high)//test goes from low to high by increments of inc
        {
            //
            dist = 0;//index counter of scans array where 10*(dist+1) is the actual distance in cm
            rval = 0;
            while(dist < 10)//calculate distances from test P value
            {
                //
                tval = test/(10*(dist+1));//calculate tval
                if(tval > scans[dist])
                    rval = rval + (tval-scans[dist])/scans[dist];//calculate r in a means similar to the sum of percent error
                else
                    rval = rval + (scans[dist] - tval)/scans[dist];
                dist++;
            }
            if(rval < r1)//compare rval to r1 and r2, assign accordingly
            {
                r1 = rval;
                p1 = test;
            }
            else
            {
                if(rval < r2)
                {
                    r2 = rval;
                    p2 = test;
                }

            }
            test = test + inc;//increment test p
        }
        inc = inc / 10;//lower increment by 1 order of magnitude
    }

    lcd_printf("%lf",p1);//print calculated p value
    while(1)//stall so that p1 can by copied and used as a constant at the head of the file
    {
        //
    }

    return p1;//will usually never be reached
}

/**
 * This function demonstrates ir measurment and can be used to observe accuracy
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_test()
{
    lcd_init();
    ir_init();
    servo_init();
    servo_setAngle(90);
    //adc_calibrate();//optional, used to calculate P value for a new cybot
    unsigned short value  = 0;
    int dist = 0;
    int num = 0;
    while(1)
    {
      /* num = 0;
        value = 0;
        while(num<16)//take 16 samples and average
        {
            value = value + ir_pulse();//generate sum
            timer_waitMillis(50);//50 milliseconds
            num++;
        }*/
        value = ir_pulse();//average
        dist = CALI/value;//calculate distance with P value aka CALI
        lcd_printf("val: %hu\ndist: %d",value,dist);//print value from adc read as well as calculated distance
        timer_waitMillis(1000);//wait 1 second and repeat */
    }

}

/**
 * This function is for calibration and sends data over putty so it may be interpreted by other softwares
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_putty(){
    //assume lcd is initialzied
    lcd_init();
    uart_init();
    ir_init();
    servo_init();
    servo_setAngle(90);
    char s[50];
    double scans[10];
    int dist = 1;
    int mnum = 0;
    int sum;
    sprintf(s,"Distance\tP-value\n\r");
    uart_sendStr(s);
    //measure stuff
    while(dist <= 10)
    {//countdown before measurment
        lcd_printf("measuring %d cm in 5 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 4 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 3 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 2 seconds",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring %d cm in 1 second",dist*10);
        timer_waitMillis(1000);
        lcd_printf("measuring...");
        mnum = 0;
        sum = 0;
        while(mnum < 16)//measure 16 times and average
        {

            sum = sum + ir_pulse();
            timer_waitMillis(100);
            mnum++;
        }
        scans[dist-1] = sum/16.0;//average samples
        sprintf(s,"%d\t%lf\n\r",(dist*10),scans[dist-1]);
        uart_sendStr(s);
        dist++;
        servo_setAngle(85);
        servo_setAngle(95);
        servo_setAngle(90);
    }
    lcd_printf("done!");
    while(1);
}

/**
 * This function calibrates the ir sensor using the ping sensor which is more precise
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_ping(){
    ir_init();
    servo_init();
    ping_init();
    lcd_init();
    servo_setAngle(69);
    int num = 200;
    int measured[200];
    int actual[200];
     int x = 0;
    double lowError = 1000000.0;
    int bestCal = 0;
    double error = 0.0;
    double testError;
    double cal = 1;
    int low;
    int high;
    int q3;//mid high = (high+q2)/2
    int q2;//mid aka (high+low)/2
    int q1;//mid low = (q2+low)/2
    double eq3;//error at c=(high+mid)/2
    double eq2;//error at c=mid
    double eq1;//error at c=(mid+low)/2
    timer_waitMillis(5000);
    servo_setAngle(90);
    timer_waitMillis(500);
    lcd_printf("measuring...");
    while(x < num){
        actual[x] = ping_read();
        measured[x] = (ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse())/8;
        x++;
        timer_waitMillis(100);//fuck around with this if you need to
    }
    servo_setAngle(0);
    lcd_printf("calculating...");


/*
    high = 1000000;
    low = 1;
    while(high > low){
        q2 = (high+low)/2;
        q3 = (high+q2)/2;
        q1 = (q2+low)/2;
        lcd_printf("%d\n%d\n%d",high,q2,low);
        //test q1
        cal = q1*1.0;
        x = 0;
        error = 0.0;
        while(x < num){
            testError = ((cal/measured[x])-actual[x])/actual[x];//sum of percent error
            if(testError > 0)
                error += testError;//testError might be negative
            else
                error -= testError;//this is a poor mans absolute value
            x++;
        }
        eq1 = error;

        //test q2
        cal = q2*1.0;
        x = 0;
        error = 0.0;
        while(x < num){
            testError = ((cal/measured[x])-actual[x])/actual[x];//sum of percent error
            if(testError > 0)
                error += testError;//testError might be negative
            else
                error -= testError;//this is a poor mans absolute value
            x++;
        }
        eq2 = error;

        //test q3
        cal = q3*1.0;
        x = 0;
        error = 0.0;
        while(x < num){
            testError = ((cal/measured[x])-actual[x])/actual[x];//sum of percent error
            if(testError > 0)
                error += testError;//testError might be negative
            else
                error -= testError;//this is a poor mans absolute value
            x++;
        }
        eq3 = error;

        if(eq1 < lowError){
            bestCal = q1;
            lowError = eq1;
        }
        if (eq2 < lowError){
            bestCal = q2;
            lowError = eq2;
        }
        if (eq3 < lowError){
            bestCal = q3;
            lowError = eq3;
        }

        if(eq2 < eq1 && eq2 < eq3){
            high = q3;
            low = q1;
        } else if (q3 < q1){
            low = q2;
        } else {
            high = q2;
        }
    }*/
    lcd_printf("CALI: %d",bestCal);
    //while(1);//optional loop here to hold and display value
    servo_setAngle(90);
    int value = 0;
    while(1){
        value = (ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse()+ir_pulse())/8;
        lcd_printf("CALI: %d\nVal: %d\nDist: %d ",bestCal,value,(bestCal/value));
        timer_waitMillis(500);
    }
}
/**
 * This function is used to read ir data directly to hand calibrate and test different calibration factors
 * @author Jordan Fox, Scott Beard
 * @date 12/2/2018
 */
void ir_cal_hand(){
    button_init();
    ir_init();
    lcd_init();
    uart_init();
    char str[50];
    int testCali = 1000;
    int pulse = 0;
    int dist = 0;
    int x = 0;
    int dir = 1;
    while(1){
        for(x = 0,pulse = 0; x < 16; x++){
            pulse += ir_pulse();
            timer_waitMillis(8);
        }
        pulse = pulse / 16;
        dist = testCali/pulse;
        lcd_printf("CALI: %d\nIR: %d\nDist: %d\nDirection: %d",testCali,pulse,dist,dir);
        sprintf(str,"\r\nCALI: %d\r\nIR: %d\r\nDist: %d\r\nDirection: %d",testCali,pulse,dist,dir);
        uart_sendStr(str);
        switch(button_getButton()){//interpret button input
            case 1:
                testCali += dir*10;
                break;
            case 2:
                testCali += dir*100;
                break;
            case 3:
                testCali += dir*1000;
                break;
            case 4:
                testCali += dir*10000;
                break;
            case 5:
                dir = -1*dir;
            }


            timer_waitMillis(100);
    }
}
