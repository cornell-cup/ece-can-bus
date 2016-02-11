/*
 * File:   ECAN_P18.c
 * Author: Sunny Jing
 *
 * Created on November 13, 2015
 */

/*********************************************************************
*
*                            Includes 
*
*********************************************************************/
#include <p18f2480.h>
#include <xc.h>
#include "ECAN.h"
#include "config.h"


/*********************************************************************
*
*                             Defines 
*
*********************************************************************/

// Define Operation Mode
#define CANSTAT_OPMODE          (0x07 << 5)
#define OPMODE_CONFIGURATION    (0x04 << 5)
#define OPMODE_LISTEN_ONLY      (0x03 << 5)
#define OPMODE_LOOPBACK_ONLY    (0x02 << 5)
#define OPMODE_DISABLE_SLEEP    (0x01 << 5)
#define OPMODE_NORMAL           (0x00 << 5)
// Define Address
#define MyAddrID 0xCC
#define DestinationAddrID 0x55

/*********************************************************************
*
*                            Global Variables 
*
*********************************************************************/
unsigned char temp_EIDH;
unsigned char temp_EIDL;
unsigned char temp_SIDH;
unsigned char temp_SIDL;
unsigned char temp_DLC;
unsigned char temp_D0;
unsigned char temp_D1;
unsigned char temp_D2;
unsigned char temp_D3;
unsigned char temp_D4;
unsigned char temp_D5;
unsigned char temp_D6;
unsigned char temp_D7;

/*********************************************************************
*
*                       Configure the CAN Module
*
*********************************************************************/
void InitECAN(void){
//-------------------------------------------------------------------    
// Step 1: Configuration mode
//-------------------------------------------------------------------    
    // REQOP <2:0> = 1xx
    CANCON = 0x80;
    // Check to make sure that configuration mode has been entered.
    while (CANSTAT & CANSTAT_OPMODE != OPMODE_CONFIGURATION);
    
//-------------------------------------------------------------------    
// Step 2: Enter (legacy) mode (mode 0)
//-------------------------------------------------------------------   
    // MDSEL<1:0> = 00
    ECANCON = 0x00;
    
//-------------------------------------------------------------------    
// Step 3: Configure IO control    
//-------------------------------------------------------------------    
    // CANTX pin will drive VDD when recessive
    CIOCONbits.ENDRHI = 1;  
    
    // Disable CAN capture, RC2/CCP1 input to CCP1 module
    CIOCONbits.CANCAP = 0;  
    
//-------------------------------------------------------------------
// Step 4: Configure Baud Rate
//-------------------------------------------------------------------   
    /* 1 Mbps @ 8MHz
     * FOSC = 8MHz, BRP<5:0> = 00h, TQ = (2*1)/8 = 0.25 us
     * Nominal Bit Time = 4TQ, TBIT = 4 * 0.25 = 1 us (10^-6s)
     * Nominal Bit Rate = 1/10^-6 = 10^6 bits/s (1 Mb/s) */
    BRGCON1 = 0xC0;  /* Sync_Seg(bit7-6)=1TQ, BRP(bit5-0)=1,
                      * TQ=((2*(BRP+1))/Fosc=4/4M=1us */
    BRGCON2 = 0x80;  /* bit7=1 ????,bit6=0 ???????? 1 ?;
                      * Phase_Seg1(bit5-3)=3TQ;  Prog_Seg(bit2-0)=1TQ*/
    BRGCON3 = 0x05;  /* Phase_Seg2(bit2-0)=3TQ */
         /* ????? =TQ*(Sync_Seg+ Prop_Seg+ Phase_seg1+ Phase_seg2) 
          *          =(1+1+3+3)TQ=8TQ, ??=1/(8*1U)=0.125MHz */

//-------------------------------------------------------------------
// Step 5: Set up the Filter and Mask registers
//         Page 380; 24.2.3.2
//-------------------------------------------------------------------     
    // Initialize Receive Masks
    // The first mask is used that accepts all SIDs and no EIDs
    // Standard Identifier bits, Extended Identifier bits
    // RXF0, RXF1
    // 0's for SID; Disable EID
    RXM0EIDH = 0x00;     
    RXM0EIDL = 0x00;
    // Standard ID FILTER
    RXM0SIDH = 0xFF;
    RXM0SIDL = 0xE0;
    
    // The second mask is used to ignore all SIDs and EIDs
    // RXF2, RXF3, RXF4, RXF5
    // 0's for SID; Disable EID
    RXM1EIDH = 0x00;    
    RXM1EIDL = 0x00;
    // SID no use
    RXM1SIDH = 0xFF;
    RXM1SIDL = 0xE0; 
    
    // Enable Filters
    //  Only using two filters
    RXFCON0 = 0x01;     // RXF0EN
    RXFCON1 = 0x00;     // Disable all
    
    // Initialize Receive Filters
    //  Filter 0 = 0x196
    //  Filter 1 = 0x19E
   
    RXF0EIDH = 0x00;
    RXF0EIDL = 0x00;
    RXF0SIDH = 0x0A; // 01100010;
    RXF0SIDL = 0xA0; // 11000000; SID
    // RXF1 not use      
    RXF1SIDH = 0x00;
    RXF1SIDL = 0x00;
    // Receive only effective 
    // RXB0CONbits.RXBODBEN =0;  // No Receive Buffer 0 overflow to Receive Buffer 1  

    RXF2SIDH = 0xFF;
    RXF2SIDL = 0;
    RXF3SIDH = 0XFF;
    RXF3SIDL = 0;  
    RXF4SIDH = 0XFF;    
    RXF4SIDL = 0;  
    RXF5SIDH = 0XFF;    
    RXF5SIDL = 0; 
    
//-------------------------------------------------------------------
// Step 6: Enter CAN module into normal mode
//-------------------------------------------------------------------   
    // REQOP<2:0> = 000
    CANCON = 0x00;
    while (CANSTAT & CANSTAT_OPMODE != OPMODE_NORMAL);
    
    // Set Receive Mode for buffers
    RXB0CON = 0x00;
    RXB1CON = 0x00;
//    RXB0CONbits.RXM1 =1;  //0; // Only receive SID & Filter 
    RXB0CONbits.RXM0 = 1;
}


