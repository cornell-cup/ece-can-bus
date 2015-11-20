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
void InitECAN(void)
{
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
//-------------------------------------------------------------------     
    // Initialize Receive Masks
    //  The first mask is used that accepts all SIDs and no EIDs
    RXM0EIDH = 0x00;    // 
    RXM0EIDL = 0x00;
    RXM0SIDH = 0xFF;    // Standard ID FILTER
    RXM0SIDL = 0xE0;
    
    //  The second mask is used to ignore all SIDs and EIDs
    RXM1EIDH = 0x00;    // 0's for EID and SID
    RXM1EIDL = 0x00;
    RXM1SIDH = 0xFF;
    RXM1SIDL = 0xE0;
    
    // Enable Filters
    //  Only using two filters
    RXFCON0 = 0x03;     //Disable all 
    RXFCON1 = 0x00;     //Disable all
    
    // Initialize Receive Filters
    //  Filter 0 = 0x196
    //  Filter 1 = 0x19E
   
    RXF0EIDH = 0x00;
    RXF0EIDL = 0x00;
    RXF0SIDH = 0x32;
    RXF0SIDL = 0xC0;
    RXF2EIDH = 0x00;
    RXF2EIDL = 0x00;
    RXF2SIDH = 0x33;
    RXF2SIDL = 0xC0;
    
 //-------------------------------------------------------------------
// Step 6: Enter CAN module into normal mode
//-------------------------------------------------------------------   
    // REQOP<2:0> = 000
    CANCON = 0x00;
    while (CANSTAT & CANSTAT_OPMODE != OPMODE_NORMAL);
    
    // Set Receive Mode for buffers
    RXB0CON = 0x00;
    RXB1CON = 0x00;
    
}

/*********************************************************************
*
*                Check the buffers, if it have message
*
*********************************************************************/
unsigned char ECAN_Receive(void)
{
    unsigned char RXMsgFlag;

    RXMsgFlag = 0x00;
    
    if (RXB0CONbits.RXFUL) //CheckRXB0
    {
        temp_EIDH = RXB0EIDH;
        temp_EIDL = RXB0EIDL;
        temp_SIDH = RXB0SIDH;
        temp_SIDL = RXB0SIDL;
        temp_DLC = RXB0DLC;
        temp_D0 = RXB0D0;
        temp_D1 = RXB0D1;
        temp_D2 = RXB0D2;
        temp_D3 = RXB0D3;
        temp_D4 = RXB0D4;
        temp_D5 = RXB0D5;
        temp_D6 = RXB0D6;
        temp_D7 = RXB0D7;
        RXB0CONbits.RXFUL = 0;
        RXMsgFlag = 0x01;
    }
    else if (RXB1CONbits.RXFUL) //CheckRXB1
    {
        temp_EIDH = RXB1EIDH;
        temp_EIDL = RXB1EIDL;
        temp_SIDH = RXB1SIDH;
        temp_SIDL = RXB1SIDL;
        temp_DLC = RXB1DLC;
        temp_D0 = RXB1D0;
        temp_D1 = RXB1D1;
        temp_D2 = RXB1D2;
        temp_D3 = RXB1D3;
        temp_D4 = RXB1D4;
        temp_D5 = RXB1D5;
        temp_D6 = RXB1D6;
        temp_D7 = RXB1D7;
        RXB1CONbits.RXFUL = 0;
        RXMsgFlag = 0x01;
    }
    else if (B0CONbits.RXFUL) //CheckB0
    {
        temp_EIDH = B0EIDH;
        temp_EIDL = B0EIDL;
        temp_SIDH = B0SIDH;
        temp_SIDL = B0SIDL;
        temp_DLC = B0DLC;
        temp_D0 = B0D0;
        temp_D1 = B0D1;
        temp_D2 = B0D2;
        temp_D3 = B0D3;
        temp_D4 = B0D4;
        temp_D5 = B0D5;
        temp_D6 = B0D6;
        temp_D7 = B0D7;
        
        B0CONbits.RXFUL = 0;
        RXMsgFlag = 0x01;
    }
    
    if  (RXMsgFlag == 0x01)
    {
        RXMsgFlag = 0x00;
        RXB1IF = 0; //A CAN Receive Buffer has received a new message 
        return TRUE;
    }
    else
    {
        return FALSE;
    }    
}






