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
#include "CONFIG.h"
#include <stdio.h>
#include <stdlib.h>

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

// Define Address
#define MyAddrID 0xCC
#define DestinationAddrID 0x55


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

void adc_init(void);
unsigned int adc_read_sensor(void);


/*********************************************************************
 *
 *                            Global Variables 
 *
 *********************************************************************/
char CAN_TX_Adress_H, CAN_TX_Adress_L; // CAN Send 
char CAN_RX_Adress_H, CAN_RX_Adress_L; // CAN Receive
unsigned char temperature;
unsigned char temp_DLC;
unsigned char temp_EIDH;
unsigned char temp_EIDL;
unsigned char temp_SIDH;
unsigned char temp_SIDL;

unsigned char RecData0;
unsigned char RecData1;
unsigned char RecData2;
unsigned char RecData3;
unsigned char RecData4;
unsigned char RecData5;
unsigned char RecData6;
unsigned char RecData7;
unsigned char TransData0;
unsigned char TransData1;
unsigned char TransData2;
unsigned char TransData3;
unsigned char TransData4;
unsigned char TransData5;
unsigned char TransData6;
unsigned char TransData7;
unsigned char TransData8;

/*********************************************************************
 *
 *                            Main Function 
 *
 *********************************************************************/
void main(void) {
    InitDevice();
    LATAbits.LATA0 = 1;

    clrscr();
    crlf();
    printf("Hello from Cornell Cup!\n\r");

    while (1) {
        int x = ECAN_Receive();
        //        printf("ECAN_Receive() == %d, RXB0CON == %d\n\r", x, RXB0CON);
        if (x) {
            // new data received
            printf("Data received: %d byte(s),  ", temp_DLC);
            printf("Temperature = %d Fahrenheit\n\r", RecData0);
        } else {

        }

    }
}

/*********************************************************************
 *
 *                       Initialize the Device 
 *
 *********************************************************************/
void InitDevice(void) {
    /* Set the internal oscillator to 8MHz
     * OSCCON<6:4> = 111 */
    OSCCONbits.IRCF = 7;
    OSCTUNEbits.PLLEN = 0; // PLL disabled - use 8MHz
    // Initialize I/O to be digital
    ADCON1bits.PCFG = 0xF;
    // RA0 output for debugging
    TRISAbits.TRISA0 = 0;
    // CAN TX, RX
    TRISBbits.TRISB2 = 0; // Output for CAN TX
    TRISBbits.TRISB3 = 1; // Input for CAN RX

    // Initialize UART module
    InitUART();
    adc_init();

    // --------------------------Priority----------------------------
    TXB0CONbits.TXPRI1 = TXB0CONbits.TXPRI0 = 1; // TX Buffer0 Highest Priority
    TXB1CONbits.TXPRI1 = TXB1CONbits.TXPRI0 = 0; // TX Buffer1 Lowest Priority
    TXB2CONbits.TXPRI1 = TXB2CONbits.TXPRI0 = 0; // TX Buffer2 Lowest Priority
    CIOCON = 0x20; // CANTX pin will drive VDD when recessive
    // --------------------------------------------------------------
    // Initialize CAN module
    InitECAN();
    // --------------------------Interrupt----------------------------
    PIE3 = PIE3 | 0x05; // TXB0IE: CAN Transmit Buffer 0 Interrupt Enable bit
    // RXB1IE: CAN Receive Buffer 0 Interrupt Enable bit
    PIR3 = 0; // Interrupt Flag bit
    IPR3 = IPR3 | 0X05; // TXB0IP & RXB0IP: INTERRUPT PRIORITY
    // --------------------------------------------------------------
}

/*********************************************************************
 *
 *                      Transmit Message
 *
 *********************************************************************/
void ECAN_Transmit() {
    PIR3bits.TXB0IF = 0;
    ClrWdt(); // Watchdog
    //AgainTrans:
    // TRANSMIT BUFFER n EXTENDED IDENTIFIER REGISTERS
    TXB0EIDH = 0x00;
    TXB0EIDL = 0x00;
    TXB0SIDH = DestinationAddrID;
    TXB0SIDL = 0x00; // SID used, EID ignored

    // TRANSMIT BUFFER n DATA FIELD BYTE m REGISTERS
    // Each transmit buffer has an array of registers
    // TXBnDLC: TRANSMIT BUFFER n DATA LENGTH CODE REGISTERS
    TXB0DLC = 0x08; // 8 byte
    TXB0D0 = temperature; // Write data here
    TXB0D1 = 0XAA;
    TXB0D2 = 0X02;
    TXB0D3 = 0X03;
    TXB0D4 = 0X04;
    TXB0D5 = 0X05;
    TXB0D6 = 0X06;
    TXB0D7 = 0X07;
    // Start transmit
    TXB0CONbits.TXREQ = 1;
    while (TXB0CONbits.TXREQ);
}

/*********************************************************************
 *
 *                          Receive Message
 *
 *********************************************************************/
unsigned char ECAN_Receive() {

//    if (RXB0CONbits.RXFUL) //CheckRXB0
//    {
        temp_EIDH = RXB0EIDH;
        temp_EIDL = RXB0EIDL;
        temp_SIDH = RXB0SIDH;
        temp_SIDL = RXB0SIDL;
        temp_DLC = RXB0DLC;
        RecData0 = RXB0D0;
        RecData1 = RXB0D1;
        RecData2 = RXB0D2;
        RecData3 = RXB0D3;
        RecData4 = RXB0D4;
        RecData5 = RXB0D5;
        RecData6 = RXB0D6;
        RecData7 = RXB0D7;
//        RXB0CONbits.RXFUL = 0;
        return TRUE;
//    }

    return FALSE;

    //    unsigned char bRecCount;
    //    unsigned char iRecID_H,iRecID_L;
    //    // Page 293, 24.2.3, RXBOCON
    //   if (RXB0CONbits.RXFUL ==0) return(0);     // No data received
    //   if (RXB0CONbits.FILHIT0 ==1) return(1);   // Acceptance Filter 1 (RXF1)
    //    // Page 287, COMSTAT
    //   if (COMSTATbits.RXB0OVFL ==1) return(2);  // Receive Buffer 0 overflowed
    ////   if (PIR3bits.RXB0IF ==1){ 
    ////       PIR3bits.RXB0IF = 0;
    ////       RXB0CONbits.RXFUL=0;
    ////       iRecID_H = RXB0SIDH;             // Receive Identifier 
    //    
    ////     if (RXB0SIDH == 0x0A) {  
    //       if (RXB0CONbits.RXFUL){
    //       iRecID_L = RXB0SIDL & 0xE0; // EXID =0
    //       bRecCount = RXB0DLC & 0x0F; // Length of data
    //       // RXF0SIDH
    //       RecData0 = RXB0D0;
    //       RecData1 = RXB0D1;
    //       RecData2 = RXB0D2;
    //       RecData3 = RXB0D3;
    //       RecData4 = RXB0D4;
    //       RecData5 = RXB0D5;
    //       RecData6 = RXB0D6;
    //       RecData7 = RXB0D7;
    //       return(9);
    ////     }
    ////      else return(3);
    //   }
}

void InitUART(void) {

    TRISCbits.TRISC6 = 0; // RC6 = TX = output

    BAUDCONbits.BRG16 = 0; // 8-bit baud generator
    //From p. 237 table, using 8MHz Fosc and 9600 Baud rate
    SPBRG = 12;
    // actual baud rate = 9615 -> -0.16% error

    TXSTAbits.TX9 = 0; // 8-bit data
    TXSTAbits.TXEN = 1; // enable transmit
    TXSTAbits.SYNC = 0; // asynchronous mode
    TXSTAbits.BRGH = 0; // low-speed mode

    RCSTAbits.SPEN = 1; // enable serial port

}

void putch(unsigned char byte) {
    TXREG = byte; // move to transmit buffer
    while (TXSTAbits.TRMT == 0); // wait for transmit completion
}

void adc_init(void) {
    ADCON0 = 0x001; // read from an0, adc on
    ADCON1 = 0x0E; // only an0 analog
    ADCON2 = 0xB3; // right justified - some random slow adc speed

}

unsigned int adc_read_sensor(void) {
    ADCON0bits.GO = 1; // start conversion
    while (ADCON0bits.GO); // wait for end of conversion
    unsigned int result = (ADRESH << 8) | ADRESL;
    return result;
}
