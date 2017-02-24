/*********************************************************************
 *  データをLCDに表示する。
 *    秋月(ACM1602NI)I2C LCD表示器を使用
 *          4MHz (内部クロック）
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
//  調整用定数
//*********************************************************************
#define CONTRAST  0x28          // for 3.3V
// #define CONTRAST  0x18               // for 5.0V

#include <xc.h>

//******************* コンフィグレーション ****************************
#pragma config FOSC = INTOSC, WDTE = OFF, PWRTE = OFF, MCLRE = OFF, CP = OFF
#pragma config CPD = OFF, BOREN = ON, CLKOUTEN = OFF, IESO = OFF, FCMEN = OFF
#pragma config WRT = OFF, PLLEN = OFF, STVREN = ON, BORV = LO, LVP = OFF
//
//******************* プロトタイプ *******************************
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

    OSCCON = 0b01101010;       // 内部クロック4Mhz
    PORTA = 0x00;
    TRISA = 0xFF;              // Port すべて入力
    ANSELA = 0x00;             // すべてデジタル
    WPUA   = 0xFF;             // 弱プルアップ ON
    nWPUEN = 0;
    // SSP1設定 -----------------------------------------------
    SSP1STAT = 0b10000000;     // スルーレート制御はOff
    SSP1ADD  = 9;              // クロック設定 100k@4MHz
    SSP1CON1 = 0b00101000;     // I2C Master modeにする
    // --------------------------------------------------------
    LCD_int();                 // LCDを初期化
    LCD_str(msgStart);         // LCD上段に"LCD Disp Test"
    LCD_posyx(1,1);            // 下段にカーソル移動
    LCD_ROMstr("I2C com");     // 下段に追記
    while(1);
}

//********************************************************************
// I2C 関連
//********************************************************************
//-------- ByteI2C送信
void i2cByteWrite(char addr, char cont, char data){
    SSP1CON2bits.SEN = 1;      // Start condition 開始
    while(SSP1CON2bits.SEN);   // Start condition 確認
    i2cTxData(addr);           // アドレス送信
    i2cTxData(cont);           // 制御コード送信
    i2cTxData(data);           // データ送信
    SSP1CON2bits.PEN = 1;      // Stop condition 開始
    while(SSP1CON2bits.PEN);   // Stop condition 確認
}
//-------- Data送信
void i2cTxData(char data){
    PIR1bits.SSP1IF = 0;       // 終了フラグクリア
    SSP1BUF = data;            // データセット
    while(!PIR1bits.SSP1IF);   // 送信終了待ち
}
//********************************************************************
// LCD 関連
//********************************************************************
//-------- １文字表示
void LCD_dat(char chr){
    i2cByteWrite(0x7C, 0x40, chr);
    __delay_us(50);            // 50μsec
}
//-------- コマンド出力
void LCD_cmd(char cmd){
    i2cByteWrite(0x7C, 0x00, cmd);
    if(cmd & 0xFC)             // 上位６ビットに１がある命令
        __delay_us(50);        // 50usec
    else
        __delay_ms(2);         // 2msec ClearおよびHomeコマンド
}
//-------- 全消去
void LCD_clr(void){
    LCD_cmd(0x01);             //Clearコマンド出力
}
//-------- カーソル位置指定
void LCD_posyx(char ypos, char xpos){
    unsigned char pcode;
    switch(ypos & 0x03){
        case 0:    pcode=0x80;break;
        case 1:    pcode=0xC0;break;
    }
    LCD_cmd(pcode += xpos);
}
//-------- 初期化
void LCD_int(void){
    __delay_ms(100);
    LCD_cmd(0x38);             // 8bit 2行 表示命令モード
    LCD_cmd(0x39);             // 8bit 2行 拡張命令モード
    LCD_cmd(0x14);             // OSC  BIAS 設定1/5
                               // コントラスト設定
    LCD_cmd(0x70 + (CONTRAST & 0x0F));
    LCD_cmd(0x5C + (CONTRAST >> 4));
    LCD_cmd(0x6B);             // Ffollwer
    __delay_ms(100);
    __delay_ms(100);
    LCD_cmd(0x38);             // 表示命令モード
    LCD_cmd(0x0C);             // Display On
    LCD_cmd(0x01);             // Clear Display
}
//-------- 文字列出力
void LCD_str(char *str){
    while(*str)                //文字列の終わり(00)まで継続
        LCD_dat(*str++);       //文字出力しポインタ＋１
}
//-------- Rom 文字列出力
void LCD_ROMstr(const char *str){
    while(*str)                //文字列の終わり(00)まで継続
        LCD_dat(*str++);       //文字出力しポインタ＋１
}
