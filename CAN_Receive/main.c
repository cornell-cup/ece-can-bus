/* 
 * File:   main.c
 * Author: Sunny Jing
 *
 * Created on November 17, 2015, 8:29 PM
 */

/*********************************************************************
*
*                            Includes 
*
*********************************************************************/

#include <xc.h>
#include <p18f2480.h>
#include "ECAN.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

/*********************************************************************
*
*                       Config Bit Settings
*
**********************************************************************

/*********************************************************************
*
*                             Defines 
*
*********************************************************************/
#define TRUE        1
#define FALSE       0
#define DEVICE_OSC  8
#define ONE_MS      666 //(unsigned int)(DEVICE_OSC/4)*80

#define clrscr()    printf( "\x1b[2J")
#define home()      printf( "\x1b[H")
#define pcr()       printf( '\r')
#define crlf()      {putchar(0x0a); putchar(0x0d);}


/*********************************************************************
*
*                        Function Prototypes 
*
*********************************************************************/
void InitDevice(void);
void ECAN_Transmit();
void Delay(unsigned int count);
void Heartbeat(void);
void InitUART(void);
void putch(unsigned char byte);


/*********************************************************************
*
*                            Global Variables 
*
*********************************************************************/
char CAN_TX_Adress_H,CAN_TX_Adress_L;  // CAN Send 
char CAN_RX_Adress_H,CAN_RX_Adress_L;  // CAN Receive
unsigned int heartbeatCount;


/*********************************************************************
*
*                            Main Function 
*
*********************************************************************/
void main(void)
{    
    InitDevice();
    LATAbits.LATA0 = 1;
    
    clrscr();
    crlf();
    printf("Hello from Cornell Cup!\n\r");
    
    while(1)
    {
         // Transmit message
        // ECAN_Transmit();
        ECAN_Receive(void);
         LATA ^= 1; // toggle RA0
         
#ifdef debug_canerr
//         int i;
//         for (i=0; i<500;i++)
//             Delay(ONE_MS);
         
//         if (TXB0CONbits.TXREQ){
//             printf("Error: TXB0CON = %d\n", TXB0CON);
//             printf("TXABT = %d, TXLARB = %d, TXERR = %d\n\r",
//                     TXB0CONbits.TXABT,
//                     TXB0CONbits.TXLARB,
//                     TXB0CONbits.TXERR);
//         }
//         ECAN_Receive();
//         LATA ^= 1; // toggle RA0
         // Toggle LED
#endif
         
         
    }
}


/*********************************************************************
*
*                       Initialize the Device 
*
*********************************************************************/
void InitDevice(void)
{
    /* Set the internal oscillator to 8MHz
     * OSCCON<6:4> = 111 */
    OSCCONbits.IRCF = 7;
    OSCTUNEbits.PLLEN = 0;  // PLL disabled - use 8MHz
    
    // Initialize global variables to 0
    heartbeatCount = 0;
    
    // Initialize I/O to be digital
    ADCON1bits.PCFG = 0xF; 
    
    // RA0 output for debugging
    TRISAbits.TRISA0 = 0;
    
    // CAN TX, RX
    TRISBbits.TRISB2 = 0;   // Output for CAN TX
    TRISBbits.TRISB3 = 1;   // Input for CAN RX
  
    
    // Initialize UART module
    InitUART();
  
    // Initialize CAN module
    InitECAN();
}

/*********************************************************************
*
*                      Transmit Message
*
*********************************************************************/
void ECAN_Transmit(void)
{
    // TRANSMIT BUFFER n EXTENDED IDENTIFIER REGISTERS
    TXB0EIDH = 0x00;
    TXB0EIDL = 0x00;
    TXB0SIDH = 0x55;
    TXB0SIDL = 0x00;    // message will transmit standard ID, EID ignored
    
    // TRANSMIT BUFFER n STANDARD IDENTIFIER REGISTERS
    CAN_TX_Adress_H = 0x32;
    CAN_TX_Adress_L = 0xC0;
    
    // So that the MCU has write access to message buffer in case
    // TXREQ wasn't already set low
    TXB0CONbits.TXREQ = 0;  
    
    // TRANSMIT BUFFER n DATA FIELD BYTE m REGISTERS
    // Each transmit buffer has an array of registers
    // TXBnDLC: TRANSMIT BUFFER n DATA LENGTH CODE REGISTERS
    TXB0DLC = 0x01; // 1 byte
    
    TXB0D0=0X55;    // Write initial value
    
    
    TXB0D1=0XAA;  
    TXB0D2=0X02;
    TXB0D3=0X03;                                                                                        
    TXB0D4=0X04;
    TXB0D5=0X05;
    TXB0D6=0X06;
    TXB0D7=0X07;
    
    TXB0CONbits.TXREQ = 1; //Set the buffer to transmit
    
}

/*********************************************************************
*
*                 Perform a simple delay 
*
*********************************************************************/
void Delay(unsigned int count)
{
    // Countdown until equal to zero and then return
    while(count--);
}    

void InitUART(void){
    
    TRISCbits.TRISC6 = 0;   // RC6 = TX = output
    
    BAUDCONbits.BRG16 = 0;  // 8-bit baud generator
    //From p. 237 table, using 8MHz Fosc and 9600 Baud rate
    SPBRG = 12;
    // actual baud rate = 9615 -> -0.16% error
    
    TXSTAbits.TX9 = 0;      // 8-bit data
    TXSTAbits.TXEN = 1;     // enable transmit
    TXSTAbits.SYNC = 0;     // asynchronous mode
    TXSTAbits.BRGH = 0;     // low-speed mode
    
    RCSTAbits.SPEN = 1;     // enable serial port

}

void putch(unsigned char byte){
    TXREG = byte;           // move to transmit buffer
    while (TXSTAbits.TRMT == 0);  // wait for transmit completion
}

