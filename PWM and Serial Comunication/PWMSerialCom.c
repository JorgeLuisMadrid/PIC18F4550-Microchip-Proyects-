/*
 * File:   microwave.c
 * Author: Jorge Madrid
 *
 * Created on 5 de diciembre de 2020, 04:30 PM
 */

#include <xc.h>
#include <stdio.h>
#define _XTAL_FREQ 1000000

 
#pragma config FOSC = INTOSC_EC    //internal oscillator, CLKOUT on RA6, EC used by USB
//#pragma config FOSC = INTOSCIO_EC	// Internal oscillator, port function on RA6, EC used by USB
//#pragma configFOSC = ECIO_EC	// External clock, port function on RA6, EC used by USB
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
unsigned long p,c,c5,c4;

//functions
void conf_serial(void);
void inicio(void);
void Write(unsigned char data);
void Write_Text(unsigned char *text);
unsigned char Read();

//Main
void main(void) {
    inicio();

    unsigned char *M1[]={"PWM Select\r\n"};
    unsigned char *M2[]={"1.Freq = 2KHz, CT = 50%\r\n"};
    unsigned char *M3[]={"2.Freq = 10KHz, CT = 80%\r\n"};
    unsigned char *M4[]={"3.Freq = 1KHz, CT = 30%\r\n"};
    unsigned char *M5[]={"Frequency = 2KHz\n CT = 50%\r\n"};
    unsigned char *M6[]={"Frequency = 10KHz\n CT = 80%\r\n"};
    unsigned char *M7[]={"Frequency = 1KHz\n CT = 30%\r\n"};
    unsigned char *ME[]={"ERROR\r\n"};
    unsigned char dato1=0;

    //PWM
    //Frequency clock (1MHz)
    OSCCON=0b01000010;

    Write_Text(*M1);
    Write_Text(*M2);
    Write_Text(*M3);
    Write_Text(*M4);

    //Main loop
    while(1){
        dato1=0;
        dato1=Read();
        switch(dato1){
            case '1'://Freq=2k CT=50%
                Write_Text(*M5);
                p=124;
                //Duty cycle=250
                //CCPRxL (8 bits)
                c=0b00111110;
                    //CCPxCON (bits 5 y 4)
                c5=1;
                c4=0;
                break;
            case '2'://Freq=10K CT=80%
                Write_Text(*M6);
                p=24;
                //Duty cycle=80
                //CCPRxL (8 bits)
                c=0b00010100;
                    //CCPxCON (bits 5 y 4)
                c5=0;
                c4=0;
                break;
            case '3'://Freq=1K CT=30%
                Write_Text(*M7);
                p=249;
                //Duty cycle=300
                //CCPRxL (8 bits)
                c=0b01001011;
                    //CCPxCON (bits 5 y 4)
                c5=0;
                c4=0;
                break;
            default:
                Write_Text(*ME);
                break;
        }
        //PWM configuration
        PR2=p;
        //CCPRxL (8 bits)
            CCPR1L=c;
        //CCPxCON (bits 5 y 4)
        CCP1CONbits.DC1B1=c5;
        CCP1CONbits.DC1B0=c4;

        TRISCbits.RC2=0; // CCP1 bit output (PWM)
        T2CON=0b00000100; //Timer 2 (TMR2) bit 2 is activated
        // CCP1 PWM configuration
        CCP1CONbits.CCP1M3=1;
        CCP1CONbits.CCP1M2=1;
    }
}

//Serial configuration function
void conf_serial(void){
    CREN=1; //asyncronus comunication 
    BRGH=1; //High speed (1)
    BRG16=1;    //speed 16 bits
    SPBRGH=0;
    SPBRG=25;   //9600 baud
    SYNC=0; //asyncronus com
    SPEN=1; //serial port avialable (RX TX)
    TX9=0;  //8 bits package (0)
    RX9=0;  //8 bits package (0)
    TXEN=1; //transmission enable
    TXREG=1; //transmission register
    TRISC6=1;   //TRISCbits.TRISC6=1;
    TRISC7=1;   //TRISCbits.TRISC7=1;
}

//init function
void inicio(void){
    conf_serial();
}

//data input function
void Write(unsigned char data){
    while(!TRMT);   //function is running until stop reading
    TXREG=data; //character input
}

//string input function
void Write_Text(unsigned char *text){
    int i;
    for ( i = 0; text[i]!='\0'; i++)
    {
        Write(text[i]);
    }
}

//read function
unsigned char Read(){
    while(!RCIF);
    return RCREG;
}