/*
 * File:   microwave.c
 * Author: Jorge Madrid
 *
 * Created on 16 de octubre de 2020, 06:16 PM
 */


#include <xc.h>
#include <math.h>
#define _XTAL_FREQ 8000000

 
//#pragma config FOSC = INTOSC_EC    //internal oscillator, CLKOUT on RA6, EC used by USB
//#pragma config FOSC = INTOSCIO_EC	// Internal oscillator, port function on RA6, EC used by USB
 #pragma configFOSC = ECIO_EC	// External clock, port function on RA6, EC used by USB
// #pragma configFOSC = EC_EC		// External clock, CLKOUT on RA6, EC used by USB
#pragma config PLLDIV = 1   //// No prescale 
// #pragma config FOSC = HSPLL_HS  //10MHz x 4 gives 40MHz clock
#pragma config FCMEN = OFF   //no fail safe clock monitor 
#pragma config IESO = OFF   //oscillator switchover disabled 
#pragma config PWRT = ON           //oscillator power up timer enabled (release version only) 
#pragma config BOR = OFF      //hardware brown out reset 
#pragma config WDT = OFF   //watchdog timer disabled 
#pragma config MCLRE = ON    //MCLR pin enabled 
#pragma config LPT1OSC = ON  //timer1 low power operation 
#pragma config PBADEN = OFF   //portB 0to 4 digital - not analogue 
#pragma config LVP = OFF     //low voltage programming disabled 
#pragma config CCP2MX = OFF  //portc1 = CCP2 
#pragma config XINST = OFF         //do not allow PIC18 extended instructions 
#pragma config  STVREN = ON         //stack overflow will cause reset 
#pragma config CP0 = OFF      //code protection block 0 
#pragma config CP1 = OFF 
#pragma config CP2 = OFF 
#pragma config CP3 = OFF 
#pragma config CPB = OFF      //code protection boot block 
#pragma config CPD = OFF      //code protection data EEPROM 
#pragma config WRT0 = OFF      //write protection block 0 
#pragma config WRT1 = OFF 
#pragma config WRT2 = OFF 
#pragma config WRT3 = OFF 
#pragma config WRTB = OFF      //write protection boot block 
#pragma config WRTC = OFF      //write protection configuration register 
#pragma config WRTD = OFF      //write protection data EEPROM 
#pragma config EBTR0 = OFF      //table read protection block 0 
#pragma config EBTR1 = OFF 
#pragma config EBTR2 = OFF 
#pragma config EBTR3 = OFF 
#pragma config EBTRB = OFF      //table read protection boot block 

//variables
unsigned int TABLA[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
//Seconds and minutes variables
unsigned int Useg=0,Dseg=0,Umin=0,Dmin=0;
//Aux variables
unsigned int cont=0,pos=0,reset=0,paro=0;

//functions
void programacion();
void temporizador();
int resta(int n);
void multiplexar(unsigned int veces);

//Main
void main(void) {   
    ADCON1 = 0x0F;  //Deactivate analog inputs
    TRISD=0;        //D Port output
    TRISC=0;        //C Port output
    TRISB=1;        //B Port output
    
    //Program loop
    while(1){
        reset=0;    //Deactivate reset
        paro=3;     //paro has to be even to initialize the timer
        //Press-button-wait loop
        while(1){   
            //PopCorn Button
            if(PORTBbits.RB4==1){
                //timer 1:30
                Useg=0;
                Dseg=3;
                Umin=1;
                Dmin=0;
                break;
            }
            //Defrost Button
            if(PORTBbits.RB5==1){
                //timer 5:30
                Useg=0;
                Dseg=3;
                Umin=5;
                Dmin=0;  
                break;
            }
            //Start button
            if(PORTBbits.RB0==1){
                //timer set manually
                programacion();
                break;
            }
        }
        //while reset=false
        //time set is saved
        while(reset==0){
            //Reset button
            if(PORTBbits.RB3==1){
                reset=1;    //reset=true
                LATCbits.LATC6=0;   //Blue LED = ON
                __delay_ms(200);
            }
            //Start/Pause button
            if(PORTBbits.RB0==1){
                paro++;     //paro is even or odd
                            //depeding of the last paro value (timer state)
            }
            //paro even
            //timer continues
            if(paro%2==0){
                LATCbits.LATC6=1;
                temporizador();
            }
            //paro odd
            //timer pause
            else
                LATCbits.LATC6=0;   //Blue LED = ON
            //while reset=false
            //timer is multiplexing
             multiplexar(16);
        }      
    }
}

//Timer manually-set function
void programacion(){
    pos=0;  //pos = 0: seconds unit, pos = 1: seconds decena, pos = 2: minutes unit, pos = 3: minutes decena
    cont=0; //counter
    //Loop to cycle through all time positions (pos values)
    while(pos<=3){
        //Displays the chosen value
        PORTD=TABLA[cont];
        //if the unit is greater than 9, returns to 0
        if(cont>9 && pos == 0) cont=0;
        if(cont>5 && pos == 1) cont=0;
        if(cont>9 && pos == 2) cont=0;
        if(cont>9 && pos == 3) cont=0;
        
        //pos = 0
        //seconds unit display = ON
        if(pos == 0) PORTCbits.RC7 = 1;
        else
            PORTCbits.RC7 = 0;
        
        //pos = 1
        //seconds decena display = ON
        if(pos == 1) PORTCbits.RC2 = 1;
        else
            PORTCbits.RC2 = 0;
        
        //pos = 2
        //minutes unit display = ON
        if(pos == 2) PORTCbits.RC1 = 1;
        else
            PORTCbits.RC1 = 0;
        
        //pos = 3
        //minutes decena display = ON
        if(pos == 3) PORTCbits.RC0 = 1;
        else
            PORTCbits.RC0 = 0;
        
        //if the increment-unit-button is pressed
        //the counter increments in one unit
        if(PORTBbits.RB2==1){
            __delay_ms(150);
            if(PORTBbits.RB2==1){
               cont++; 
            }
        }
        //if the next-button is pressed
        //change the time position (pos value)
        if(PORTBbits,RB1==1){
            __delay_ms(150);
            if(PORTBbits,RB1==1){
                if(pos == 0) Useg = cont;
                if(pos == 1) Dseg = cont;
                if(pos == 2) Umin = cont;
                if(pos == 3) Dmin = cont;
                cont=0;
                pos++;
            }
        }
    }
}

//Timer function
void temporizador(){
    //time 00:00
    if(Useg==0 && Dseg==0 && Umin==0 && Dmin==0){
        LATCbits.LATC6=0;
        //timer stop
        reset=1;
        return;
    }
    //time n0:00
    else if(Useg==0 && Dseg==0 && Umin==0){
        //it changes to (n-1)9:59
        Dmin=resta(Dmin);
        Umin=9;
        Dseg=5;
        Useg=9;
    }
    //timer mn:00
    else if(Useg==0 && Dseg==0){
        //it changes to m(n-1):59
        Umin=resta(Umin);
        Dseg=5;
        Useg=9;
    }
    //timer lm:n0
    else if(Useg==0){
        //it changes to lm:(n-1)9
        Dseg=resta(Dseg);
        Useg=9;
    }
    //timer kl:mn
    else{
        //it changes to kl:m(n-1)
        Useg=resta(Useg);
    }
    
}
//Timer changer function
int resta(int n){
    //One unit is decremented
    n--;
    //if 0 is reached
    if(n<0){
        n=9;    //restarts in 9
    }
    return n;
}

//Multiplexor unit
//veces is the frequency variable
void multiplexar(unsigned int veces){
    while(veces){
        //Second unit
        LATD=TABLA[Useg];
        LATCbits.LATC7=1;
        __delay_ms(15);
        LATCbits.LATC7=0;
        
        //Second decena
        LATD=TABLA[Dseg];
        LATCbits.LATC2=1;
        __delay_ms(15);
        LATCbits.LATC2=0;
        
        //Minute unit
        LATD=TABLA[Umin];
        LATCbits.LATC1=1;
        __delay_ms(15);
        LATCbits.LATC1=0;
        
        //Minute decena
        LATD=TABLA[Dmin];
        LATCbits.LATC0=1;
        __delay_ms(15);
        LATCbits.LATC0=0;
        veces--;
    }
}
