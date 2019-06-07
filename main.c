#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <xc.h>
#define _XTAL_FREQ 20000000
#define TMR2PRESCALE 4
long PWM_freq = 5000;
int init_run = 1;
int start_flag = 0;
int toggle_flag = 0;
short unsigned int analog_reading;
void PWM_Duty(unsigned int duty){
      if(duty<1023){
        duty = ((float)duty/1023)*(_XTAL_FREQ/(PWM_freq*TMR2PRESCALE)); //duty = (((float)duty/1023)*(1/PWM_freq)) / ((1/_XTAL_FREQ) * TMR2PRESCALE);
        CCP1X = duty & 1; //Store the 1st bit
        CCP1Y = duty & 2; //Store the 0th bit
        CCPR1L = duty>>2;// Store the remining 8 bit
  }
}
void Initialize(){
    ADCON0=0x01;
    ADCON1=0x00;
    TRISB = 0;
    TRISD = 1;
    LATB = 16;
    PR2 = (_XTAL_FREQ/(PWM_freq*4*TMR2PRESCALE)) - 1; //PR2 formulae using Datasheet
    CCP1M3 = 1; CCP1M2 = 1;  //configure the CCP1 module 
    T2CKPS0 = 1;T2CKPS1 = 0; TMR2ON = 1; //configure the timer
    TRISC2 = 0; // make port pin on C as output
}
void AD_read(){
    ADCON0bits.GO_DONE=1; //do A/D measurement
    while(ADCON0bits.GO_DONE==1);
    analog_reading= ADRESL + (ADRESH*256);
    analog_reading=analog_reading/256;
    return analog_reading;
}
void start_stop(){
    if(PORTDbits.RD0){ //check stop/start button
            if(init_run){
                init_run = 0;
                LATB = 21;
            }
            if(start_flag){ //toggle
                start_flag = 0;
            }else{
                start_flag = 1;
            }
            while(PORTDbits.RD0); //de-bounce
        }
    if(start_flag){ //run
            PWM_Duty(analog_reading*4);
        }else{ //stop
            
            PWM_Duty(0);
        }
}
void toggle_direction(){ //toggle motor direction
    if(PORTDbits.RD1){ //if push button pressed
            if(toggle_flag){ //toggle flag check
                toggle_flag = 0;
                PWM_Duty(0); //turn off motor before reversing direction
                while(PORTDbits.RD1); //de-bounce
                LATB = 21;
            }else{
                toggle_flag = 1;
                PWM_Duty(0); //turn off motor before reversing direction
                while(PORTDbits.RD1); //de-bounce
                LATB = 26;
            }  
        }
}
void main(){
    Initialize(); //initialize everything
    while(1){ //main loop
        AD_read(); //do analog to digital read
        start_stop(); //check push button to start/stop motor
        toggle_direction(); //check push button to toggle motor direction
    }
};
