#ifndef lpcmod_v1_h
#define lpcmod_v1_h

#define BNK512  0x00
#define BNK256  0x02
#define BNKOS  0x03
#define BNKTSOP 0x04
#define LPCMOD_TRUE 0x01
#define LPCMOD_FALSE	0x00


#define BNK_CONTROL	0xF710
#define DISABLE_MOD	0xF711
#define ENABLE_5V	0xF713
#define LCD_DATA	0xF700
#define LCD_BL		0xF701
#define LCD_CT		0xF703

#define SYSCON_REG	0xF701
#define SYSCON_ID	0x15	//Spoof SmartXX OPX using lower nibble. Highest nibble is what's different from SmartXX.

#define HD44780		0x0

/*
#define ADR_ACTIVEBANK 0xFF010
#define ADR_QUICKBOOT 0xFF011
#define ADR_ACTIVEITEM 0xFF012
#define ADR_FANSPEED 0xFF013
#define ADR_BOOTTIMEOUT 0xFF014

#define ADR_BIOSNAME0 0xFF023
#define ADR_BIOSNAME1 0xFF037
#define ADR_BIOSNAME2 0xFF04B
#define ADR_BIOSNAME3 0xFF05F
#define ADR_BIOSNAME4 0xFF073
#define ADR_BIOSNAME5 0xFF087
#define ADR_BIOSNAME6 0xFF09B
#define ADR_BIOSNAME7 0xFF0AF
#define ADR_BIOSNAME0 0xFF0C3

#define ADR_ENABLENET 0xFF0ED
#define ADR_USEDHCP 0xFF0EE
#define ADR_IP 0xFF0EF
#define ADR_GATEWAY 0xFF0F3
#define ADR_DNS1 0xFF0F7
#define ADR_DNS2 0xFF0FB

#define ADR_EN5V 0xFF101
#define ADR_LCDTYPE 0xFF102
#define ADR_NBLINES 0xFF103
#define ADR_LINELENGTH 0xFF104
#define ADR_BACKLIGHT 0xFF105
#define ADR_CONTRAST 0xFF106
#define ADR_DISPMSGBOOT 0xFF107
#define ADR_CUSTOMTEXT 0xFF108
#define ADR_BIOSNAMEDISP 0xFF109
#define ADR_LCDSTRING0 0xFF110
#define ADR_LCDSTRING0 0xFF124
#define ADR_LCDSTRING0 0xFF138
#define ADR_LCDSTRING0 0xFF14C
*/




#endif
