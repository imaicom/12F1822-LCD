/*********************************************************************
 *  ????LCD??????
 *    ??(ACM1602NI)I2C LCD??????
 *          4MHz (???????
 *
 *     1 VDD         8 VSS
 *     2 RA5         7 RA0
 *     3 RA4         6 RA1 SCL
 *     4 MCLR        5 RA2 SDA
 *
 * PIC12F1822  MPLAB X IDE with XC8 Ver1.32
 * Copyright (c) 2014 iwamoto All Rights Reserved
 * *******************************************************************/
#define _XTAL_FREQ 4000000

//*********************************************************************
//  ?????
//*********************************************************************
#define CONTRAST  0x28          // for 3.3V
// #define CONTRAST  0x18               // for 5.0V

#include <xc.h>

//******************* ?????????? ****************************
#pragma config FOSC = INTOSC, WDTE = OFF, PWRTE = OFF, MCLRE = OFF, CP = OFF
#pragma config CPD = OFF, BOREN = ON, CLKOUTEN = OFF, IESO = OFF, FCMEN = OFF
#pragma config WRT = OFF, PLLEN = OFF, STVREN = ON, BORV = LO, LVP = OFF
//
//******************* ?????? *******************************
void i2cByteWrite(char, char, char);
void i2cTxData(char);
void LCD_dat(char);
void LCD_cmd(char);
void LCD_clr(void);
void LCD_posyx(char,char);
void LCD_int(void);

void LCD_str(char *);
void LCD_ROMstr(const char *);

// ******************* main ******************************************
void main() {
    char msgStart[] ="LCD Test";

    OSCCON = 0b01101010;       // ??????4Mhz
    PORTA = 0x00;
    TRISA = 0xFF;              // Port ?????
    ANSELA = 0x00;             // ???????
    WPUA   = 0xFF;             // ?????? ON
    nWPUEN = 0;
    // SSP1?? -----------------------------------------------
    SSP1STAT = 0b10000000;     // ?????????Off
    SSP1ADD  = 9;              // ?????? 100k@4MHz
    SSP1CON1 = 0b00101000;     // I2C Master mode???
    // --------------------------------------------------------
    LCD_int();                 // LCD????
    LCD_str(msgStart);         // LCD???"LCD Disp Test"
    LCD_posyx(1,1);            // ?????????
    LCD_ROMstr("I2C com");     // ?????
    while(1);
}

//********************************************************************
// I2C ??
//********************************************************************
//-------- ByteI2C??
void i2cByteWrite(char addr, char cont, char data){
    SSP1CON2bits.SEN = 1;      // Start condition ??
    while(SSP1CON2bits.SEN);   // Start condition ??
    i2cTxData(addr);           // ??????
    i2cTxData(cont);           // ???????
    i2cTxData(data);           // ?????
    SSP1CON2bits.PEN = 1;      // Stop condition ??
    while(SSP1CON2bits.PEN);   // Stop condition ??
}
//-------- Data??
void i2cTxData(char data){
    PIR1bits.SSP1IF = 0;       // ????????
    SSP1BUF = data;            // ??????
    while(!PIR1bits.SSP1IF);   // ??????
}
//********************************************************************
// LCD ??
//********************************************************************
//-------- ?????
void LCD_dat(char chr){
    i2cByteWrite(0x7C, 0x40, chr);
    __delay_us(50);            // 50?sec
}
//-------- ??????
void LCD_cmd(char cmd){
    i2cByteWrite(0x7C, 0x00, cmd);
    if(cmd & 0xFC)             // ?????????????
        __delay_us(50);        // 50usec
    else
        __delay_ms(2);         // 2msec Clear???Home????
}
//-------- ???
void LCD_clr(void){
    LCD_cmd(0x01);             //Clear??????
}
//-------- ????????
void LCD_posyx(char ypos, char xpos){
    unsigned char pcode;
    switch(ypos & 0x03){
        case 0:    pcode=0x80;break;
        case 1:    pcode=0xC0;break;
    }
    LCD_cmd(pcode += xpos);
}
//-------- ???
void LCD_int(void){
    __delay_ms(100);
    LCD_cmd(0x38);             // 8bit 2? ???????
    LCD_cmd(0x39);             // 8bit 2? ???????
    LCD_cmd(0x14);             // OSC  BIAS ??1/5
                               // ????????
    LCD_cmd(0x70 + (CONTRAST & 0x0F));
    LCD_cmd(0x5C + (CONTRAST >> 4));
    LCD_cmd(0x6B);             // Ffollwer
    __delay_ms(100);
    __delay_ms(100);
    LCD_cmd(0x38);             // ???????
    LCD_cmd(0x0C);             // Display On
    LCD_cmd(0x01);             // Clear Display
}
//-------- ?????
void LCD_str(char *str){
    while(*str)                //???????(00)????
        LCD_dat(*str++);       //???????????
}
//-------- Rom ?????
void LCD_ROMstr(const char *str){
    while(*str)                //???????(00)????
        LCD_dat(*str++);       //???????????
}

