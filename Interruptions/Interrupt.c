/*
 * File:   interrupt.c
 * Author: Jorge Madrid
 *
 * Created on 14 de noviembre de 2020, 12:39 PM
 */


#include <xc.h>
#include <stdio.h>
#define _XTAL_FREQ 8000000
#include "flex_lcd.h"


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
unsigned char palabra[20];  //LCD sentence string
unsigned int i, low=0, high=0; //interruptions counter and iterator


void main(void) {
    TRISD=0;  //D port output (LCD)
    LATD=0;
    TRISBbits.TRISB0=1; //bit BO High Interruption
    TRISBbits.TRISB1=1; //bit B1 Low Interruption
    
    //LCD init
    Lcd_Init();
    Lcd_Cmd(LCD_CLEAR);
    Lcd_Cmd(LCD_CURSOR_OFF);
    __delay_ms(100);
    
    RCONbits.IPEN=1;  //priority levels enable
    INTCONbits.GIE=1; //high interruptions enable
    INTCONbits.GIEL=1;  //low interruptions enable
    
    //INT0 conf
    INTCONbits.INT0IE=1; //INT0 external interruption enable
    INTCON2bits.INTEDG0=1; //flanco de subida (1) flanco de bajada (0)
    INTCONbits.INT0IF=0;    // INT0 flag indicator is reset

    //INT1 conf 
    INTCON3bits.INT1IE=1; //INT1 external interruption enable
    INTCON2bits.INTEDG1=1; //flanco de subida (1) flanco de bajada (0)
    INTCON3bits.INT1IP=0; //high priority (1) low priority (0)
    INTCON3bits.INT1IF=0;   //INT1 flag indicator is reset
    
    //main loop
    while(1){
        for(i=0;i<9;i++){
            sprintf(palabra,"High: %d",high);
            Lcd_Out2(1,0,palabra);
            sprintf(palabra,"Low: %d Cont: %d",low,i);
            Lcd_Out2(2,0,palabra);
            __delay_ms(500);
            Lcd_Cmd(LCD_CLEAR);
        }
    }

}

//High priority interrupt
void __interrupt() ISR_INT0(void){
    if(PORTBbits.RB0==1){
        Lcd_Cmd(LCD_CLEAR);
        Lcd_Out(1,0,"High Interrupt");
        Lcd_Out(2,0,"Enable");
        high=high+1;    // interrupt counter
    }
    while(PORTBbits.RB0==1);
    Lcd_Cmd(LCD_CLEAR);
    INTCONbits.INT0IF=0;
}

//Low priority interrupt
void __interrupt(low_priority) ISR_INT1(void){
    if(PORTBbits.RB1==1){
        Lcd_Cmd(LCD_CLEAR);
        Lcd_Out(1,0,"Low Interrupt");
        Lcd_Out(2,0,"Enable");
        low=low+1;  //interrupt counter
    }
    while(PORTBbits.RB1==1);
    Lcd_Cmd(LCD_CLEAR);
    INTCON3bits.INT1IF=0;
}