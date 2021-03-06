/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _BootLCD_H_
#define _BootLCD_H_

typedef struct _xLCD {
    int DisplayType;
    int enable;
    int    LineSize;
    int    nbLines;
    int    TimingCMD;
    int    TimingData;

    unsigned char    Line1Start;
    unsigned char    Line2Start;
    unsigned char    Line3Start;
    unsigned char    Line4Start;

    unsigned char  LineOwner[4];

    void    (*Init)(void);
    void    (*Command)(unsigned char value);
    void    (*Data)(unsigned char value);

    void    (*WriteIO)(unsigned char data, unsigned char RS, unsigned short wait);

    void    (*PrintLine[4])(bool centered, char *lineText);

    void    (*ClearLine)(unsigned char line);
}__attribute__((packed)) _xLCD;    //Will be know as xLCD from now on.

_xLCD xLCD;

//Various predefined values to write to LCD to init.
//bit1 = RS
//bit2 = E
//bit3 = D4
//bit4 = D5
//bit5 = D6
//bit6 = D7
#define DISPLAY_E         0x04        //Mapped for LPCMod output format.
#define DISPLAY_RS        0x02        //Mapped for LPCMod output format.
#define DISP_CLEAR          0x01 // cmd: clear display command
#define DISP_HOME           0x02 // cmd: return cursor to home
#define DISP_ENTRY_MODE_SET 0x04 // cmd: enable cursor moving direction and enable the shift of entire display
#define DISP_S_FLAG         0x01
#define DISP_ID_FLAG        0x02
#define DISP_CONTROL        0x08  //cmd: display ON/OFF
#define DISP_D_FLAG         0x04  // display on
#define DISP_C_FLAG         0x02  // cursor on
#define DISP_B_FLAG         0x01  // blinking on
#define DISP_EXT_CONTROL    0x08  //RE_FLAG = 1
#define DISP_FW_FLAG        0x04  //RE_FLAG = 1
#define DISP_BW_FLAG        0x02  //RE_FLAG = 1
#define DISP_NW_FLAG        0x01  //RE_FLAG = 1
#define DISP_SHIFT          0x10  //cmd: set cursor moving and display shift control bit, and the direction without changing of ddram data
#define DISP_SC_FLAG        0x08
#define DISP_RL_FLAG        0x04
#define DISP_FUNCTION_SET   0x20  // cmd: set interface data length
#define DISP_DL_FLAG        0x10  // set interface data length: 8bit/4bit
#define DISP_N_FLAG         0x08  // number of display lines:2-line / 1-line
#define DISP_F_FLAG         0x04  // display font type 5x11 dots or 5x8 dots
#define DISP_RE_FLAG        0x04
#define DISP_CGRAM_SET      0x40  // cmd: set CGRAM address in address counter
#define DISP_SEGRAM_SET     0x40  //RE_FLAG = 1
#define DISP_DDRAM_SET      0x80  // cmd: set DDRAM address in address counter

//Xecuter 3 specific
#define X3_DISPLAY_RS      0x01
//#define X3_DISPLAY_RW      0x02   //not used, we only write.
//#define X3_DISPLAY_E       0x04     //Same as others

#define CENTERSTRING        1
#define JUSTIFYLEFT         0

//Who owns the right to print onto the line
//Priority is as numerically represented (Script is more important than custom which is more important than system)
#define SYSTEM_OWNER    0       //System, base
#define CUSTOM_OWNER    1       //Custom string
#define SCRIPT_OWNER    2       //Script engine


void BootLCDInit(void);
void BootLCDSwitchType(void);
void BootLCDUpdateLinesOwnership(unsigned char line, unsigned char fromScript);

void toggleEN5V(unsigned char value);
void setLCDContrast(unsigned char value);
void setLCDBacklight(unsigned char value);

void assertInitLCD(void);
void initialLCDPrint(void);

void WriteLCDInit(void);
void WriteLCDCommand(unsigned char value);
void WriteLCDData(unsigned char value);
void WriteLCDIO(unsigned char data, unsigned char RS, unsigned short wait);
void WriteLCDLine0(bool centered, char *lineText);
void WriteLCDLine1(bool centered, char *lineText);
void WriteLCDLine2(bool centered, char *lineText);
void WriteLCDLine3(bool centered, char *lineText);
void WriteLCDCenterString(char * StringOut, char * stringIn);
void WriteLCDFitString(char * StringOut, char * stringIn);
void WriteLCDSetPos(unsigned char pos, unsigned char line);
void WriteLCDClearLine(unsigned char line);

#endif // _BootLCD_H_
