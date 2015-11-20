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
#define TRUE    1
#define FALSE   0
#define DEVICE_OSC  8
#define ONE_MS      (unsigned int)(DEVICE_OSC/4)*80


/*********************************************************************
*
*                        Function Prototypes 
*
*********************************************************************/
void InitDevice(void);
void ECAN_Transmit();
void Delay(unsigned int count);
void Heartbeat(void);


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
    
    while(1)
    {
         // Transmit message
         ECAN_Transmit();
         // Toggle LED
         Heartbeat();
         // Delay for one millisecond 
         Delay(ONE_MS);
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
    
    // PORTB as outputs 
    LATB = 0x00;
    TRISB = 0x00;
  
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
    
    // TRANSMIT BUFFER n STANDARD IDENTIFIER REGISTERS
    CAN_TX_Adress_H = 0x32;
    CAN_TX_Adress_L = 0xC0;
    
    // TRANSMIT BUFFER n DATA FIELD BYTE m REGISTERS
    // Each transmit buffer has an array of registers
    // TXBnDLC: TRANSMIT BUFFER n DATA LENGTH CODE REGISTERS
    TXB0DLC = 0x08; // 8 bytes
    
    TXB0D0=0X00;    // Write initial value
    TXB0D1=0X01;
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

/*********************************************************************
*
*             Toggle LED to show device is working 
*
*********************************************************************/
void Heartbeat(void)
{
    // Toggle LED every 256th time this function is called
    if (heartbeatCount < 255)
    {
        heartbeatCount++;
    }
    else
    {
        heartbeatCount = 0;
        LATBbits.LATB7 ^= 1;
    }
}






