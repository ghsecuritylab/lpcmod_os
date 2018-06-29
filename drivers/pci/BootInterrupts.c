/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

 */

#include "boot.h"
#include "i2c.h"
#include "cpu.h"
#include "lib/LPCMod/BootLPCMod.h"
#include "lib/time/timeManagement.h"
#include "xblast/HardwareIdentifier.h"
#include "xblast/PowerManagement.h"

volatile int nCountI2cinterrupts, nCountUnusedInterrupts, nCountUnusedInterruptsPic2, nCountInterruptsSmc, nCountInterruptsIde;
volatile TRAY_STATE traystate;
unsigned int wait_ms_time;

volatile int nInteruptable = 0;

// interrupt service stubs defined in BootStartup.S
extern void IntHandlerTimer0(void);
extern void IntHandlerI2C(void);
extern void IntHandlerSmc(void);
extern void IntHandlerIde(void);
extern void IntHandlerUnused(void);
//extern void IntHandlerUnusedPic2(void);

extern void IntHandler1(void);
extern void IntHandler2(void);
extern void IntHandler3(void);
extern void IntHandler4(void);
extern void IntHandler5(void);
extern void IntHandler6(void);
extern void IntHandler7(void);
extern void IntHandler8(void);
extern void IntHandler9(void);
extern void IntHandler10(void);
extern void IntHandler13(void);
extern void IntHandler15(void);

extern void IntHandlerException0(void);
extern void IntHandlerException1(void);
extern void IntHandlerException2(void);
extern void IntHandlerException3(void);
extern void IntHandlerException4(void);
extern void IntHandlerException5(void);
extern void IntHandlerException6(void);
extern void IntHandlerException7(void);
extern void IntHandlerException8(void);
extern void IntHandlerException9(void);
extern void IntHandlerExceptionA(void);
extern void IntHandlerExceptionB(void);
extern void IntHandlerExceptionC(void);
extern void IntHandlerExceptionD(void);
extern void IntHandlerExceptionE(void);
extern void IntHandlerExceptionF(void);
extern void IntHandlerException10(void);

// structure defining our ISRs

typedef struct
{
    unsigned char m_bInterruptCpu;
    unsigned int m_dwpVector;
} ISR_PREP;

const ISR_PREP isrprep[] =
{
    { 0x00, (unsigned int)IntHandlerException0 },
    { 0x01, (unsigned int)IntHandlerException1 },
    { 0x02, (unsigned int)IntHandlerException2 },
    { 0x03, (unsigned int)IntHandlerException3 },
    { 0x04, (unsigned int)IntHandlerException4 },
    { 0x05, (unsigned int)IntHandlerException5 },
    { 0x06, (unsigned int)IntHandlerException6 },
    { 0x07, (unsigned int)IntHandlerException7 },
    { 0x08, (unsigned int)IntHandlerException8 },
    { 0x09, (unsigned int)IntHandlerException9 },
    { 0x0a, (unsigned int)IntHandlerExceptionA },
    { 0x0b, (unsigned int)IntHandlerExceptionB },
    { 0x0c, (unsigned int)IntHandlerExceptionC },
    { 0x0d, (unsigned int)IntHandlerExceptionD },
    { 0x0e, (unsigned int)IntHandlerExceptionE },
    { 0x0f, (unsigned int)IntHandlerExceptionF },
    { 0x10, (unsigned int)IntHandlerException10 },

            // interrupts from PIC1

    { 0x20, (unsigned int)IntHandlerTimer0 },
    { 0x21, (unsigned int)IntHandler1 },
    { 0x22, (unsigned int)IntHandler2 },
    { 0x23, (unsigned int)IntHandler3 },
    { 0x24, (unsigned int)IntHandler4 },
    { 0x25, (unsigned int)IntHandler5 },
    { 0x26, (unsigned int)IntHandler6 },
    { 0x27, (unsigned int)IntHandler7 },

            // interrupts from PIC 2

    { 0x70, (unsigned int)IntHandler8 },
    { 0x71, (unsigned int)IntHandler9 },
    { 0x72, (unsigned int)IntHandler10 },
    { 0x73, (unsigned int)IntHandlerI2C },
    { 0x74, (unsigned int)IntHandlerSmc },
    { 0x75, (unsigned int)IntHandler13 },
    { 0x76, (unsigned int)IntHandlerIde },
    { 0x77, (unsigned int)IntHandler15 },

    { 0, 0 }
};

void BootInterruptsWriteIdt(void)
{

    volatile ts_pm_interrupt* ptspmi = (volatile ts_pm_interrupt *)(IDT_LOC);  // ie, start of IDT area
    int n, n1 = 0;

    // init storage used by ISRs

    VIDEO_VSYNC_POSITION = 0;
    BIOS_TICK_COUNT = 0;
    VIDEO_VSYNC_DIR = 0;
    nCountI2cinterrupts = 0;
    nCountUnusedInterrupts = 0;
    nCountUnusedInterruptsPic2 = 0;
    nCountInterruptsSmc = 0;
    nCountInterruptsIde = 0;
    traystate = ETS_NOTHING;
    VIDEO_LUMASCALING = 0;
    VIDEO_RSCALING = 0;
    VIDEO_BSCALING = 0;

        // set up default exception, interrupt vectors to dummy stubs

    for(n = 0; n < 0x100; n++)   // have to do 256
    {
        ptspmi[n].m_wSelector = 0x08;
        ptspmi[n].m_wType = 0x8e00;  // interrupt gate, 32-bit
        if(n == isrprep[n1].m_bInterruptCpu)   // is it next on our prep list?  If so, stick it in
        {
            ptspmi[n].m_wHandlerHighAddressLow16 = (unsigned short)isrprep[n1].m_dwpVector;
            ptspmi[n].m_wHandlerLinearAddressHigh16 = (unsigned short)(((unsigned int)isrprep[n1].m_dwpVector) >> 16);
            n1++;
        }
        else  // otherwise default handler (pretty useless, but will catch it)
        {
            ptspmi[n].m_wHandlerHighAddressLow16 = (unsigned short)((unsigned int)IntHandlerUnused);
            ptspmi[n].m_wHandlerLinearAddressHigh16 = (unsigned short)(((unsigned int)IntHandlerUnused) >> 16);
        }
    }

    // set up the Programmable Inetrrupt Controllers

    IoOutputByte(0x20, 0x15);  // master PIC setup
    IoOutputByte(0x21, 0x20);  // base interrupt vector address
    IoOutputByte(0x21, 0x04);  // am master, INT2 is hooked to slave
    IoOutputByte(0x21, 0x01);  // x86 mode, normal EOI
    IoOutputByte(0x21, 0x00);        // enable all ints

    IoOutputByte(0xa0, 0x15);    // slave PIC setup
    IoOutputByte(0xa1, 0x70);  // base interrupt vector address
    IoOutputByte(0xa1, 0x02);  // am slave, hooked to INT2 on master
    IoOutputByte(0xa1, 0x01);  // x86 mode normal EOI

    if (isXBE() == false)
    {
        IoOutputByte(0xa1, 0x00);    // enable int14(IDE) int12(SMI)
    }
    else
    {
        IoOutputByte(0xa1, 0xaf);
    }

    // enable interrupts
    intel_interrupts_on();
}



///////////////////////////////////////////
//
// ISRs


void IntHandlerCSmc(void)
{
    unsigned char bStatus, nBit=0;
        unsigned int temp;
        unsigned char temp_AV_mode;
        
    nCountInterruptsSmc++;
   
        
    temp = IoInputWord(0x8000);
    if (temp!=0x0)
    {
        IoOutputWord(0x8000,temp);
        //printk("System Timer wants to sleep we kill him");
        //return;
    }
        
   
    
    bStatus=I2CTransmitByteGetReturn(0x10, 0x11); // Query PIC for interrupt reason
    
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "\n\nreturn byte : 0x%02X\n", bStatus);
    // we do nothing, if there is not Interrupt reason
    if (bStatus==0x0)
    {
        return;
    }
    

    while(nBit<7)
    {
        if(bStatus & 1)
        {
            unsigned char b=0x04;
            switch(nBit)
            {
            case 0: // POWERDOWN EVENT
                //if(canPowerDown())
                {
                    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: Powerdown", nCountInterruptsSmc);
                    I2CTransmitWord(0x10, 0x0200);
                    I2CTransmitWord(0x10, 0x0100|b);
                    I2CTransmitWord(0x10, 0x0500|b);
                    I2CTransmitWord(0x10, 0x0600|b);
                    I2CTransmitWord(0x10, 0x0900|b);
                    I2CTransmitWord(0x10, 0x0a00|b);
                    I2CTransmitWord(0x10, 0x0b00|b);
                    I2CTransmitWord(0x10, 0x0d00|b);
                    I2CTransmitWord(0x10, 0x0e00|b);
                    I2CTransmitWord(0x10, 0x0f00|b);
                    I2CTransmitWord(0x10, 0x1000|b);
                    I2CTransmitWord(0x10, 0x1200|b);
                    I2CTransmitWord(0x10, 0x1300|b);
                    I2CTransmitWord(0x10, 0x1400|b);
                    I2CTransmitWord(0x10, 0x1500|b);
                    I2CTransmitWord(0x10, 0x1600|b);
                    I2CTransmitWord(0x10, 0x1700|b);
                    I2CTransmitWord(0x10, 0x1800|b);
                }
                break;

            case 1: // CDROM TRAY IS NOW CLOSED
                traystate = ETS_CLOSED;
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: CDROM Tray now Closed", nCountInterruptsSmc);
                DVD_TRAY_STATE = DVD_CLOSED;
                break;

            case 2: // CDROM TRAY IS STARTING OPENING
                traystate = ETS_OPEN_OR_OPENING;
                DVD_TRAY_STATE = DVD_OPENING;
                I2CTransmitWord(0x10, 0x0d02);
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: CDROM starting opening", nCountInterruptsSmc);
                break;

            case 3: // AV CABLE HAS BEEN PLUGGED IN

                temp_AV_mode = I2CTransmitByteGetReturn(0x10, 0x04);
                // Compare to global variable
                if (VIDEO_AV_MODE != temp_AV_mode )
                {
                    VIDEO_AV_MODE = 0xff;
                    wait_ms_blocking(30);
                    VIDEO_AV_MODE = temp_AV_mode;
                    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);
                    wait_ms_blocking(200);
                    BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);

                }
                break;

            case 4: // AV CABLE HAS BEEN UNPLUGGED
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: AV cable unplugged", nCountInterruptsSmc);
                VIDEO_AV_MODE = 0xff;
                //vmode.m_bAvPack=0xff;
                break;

            case 5: // BUTTON PRESSED REQUESTING TRAY OPEN
                traystate = ETS_OPEN_OR_OPENING;
                I2CTransmitWord(0x10, 0x0d04);
                I2CTransmitWord(0x10, 0x0c00);
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: CDROM tray opening by Button press", nCountInterruptsSmc);
                bStatus&=~0x02; // kill possibility of conflicting closing report
                break;

            case 6: // CDROM TRAY IS STARTING CLOSING
                traystate = ETS_CLOSING;
                DVD_TRAY_STATE = DVD_CLOSING;
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: CDROM tray starting closing", nCountInterruptsSmc);
                break;

            case 7: // UNKNOWN
                XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "SMC Interrupt %d: b7 Reason code", nCountInterruptsSmc);
                break;
            }
        }
        nBit++;
        bStatus >>= 1;
    }
}

void IntHandlerCIde(void)
{
    if(!nInteruptable)
    {
        return;
    }

    unsigned char statusReg = IoInputByte(0x1f7);
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG | DBG_FLG_SPI, "IDE Interrupt. status:%02X", statusReg);
    nCountInterruptsIde++;
}

void IntHandlerCI2C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    nCountI2cinterrupts++;
}
void IntHandlerUnusedC(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Unhandled Interrupt");
    //printk("Unhandled Interrupt");
    nCountUnusedInterrupts++;
    //while(1) ;
}


void IntHandlerUnusedC2(void)
{
    if(!nInteruptable)
    {
        return;
    }

    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Unhandled Interrupt 2");
    nCountUnusedInterruptsPic2++;
}



// this guy is getting called at 1000.15Hz, 1ms

void IntHandlerCTimer0(void)
{
    BIOS_TICK_COUNT++;
    updateTime();
}
 
 
// USB interrupt

void IntHandler1C(void)
{
    // Interrupt for OHCI controller located on 0xfed00000
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "USB Interrupt 1C");
    USBGetEvents();
}
void IntHandler9C(void)
{
    // Interrupt for OHCI controller located on 0xfed08000
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "USB Interrupt 9C");
    USBGetEvents();
}


void IntHandler2C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 2");
}

void IntHandler3VsyncC(void)  // video VSYNC
{
    *((volatile unsigned int *)0xfd600100)=0x1;  // clear VSYNC int
} 


void IntHandler4C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 4");
}
void IntHandler5C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 5");
}

void IntHandler6C(void)
{

    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 6");
}

void IntHandler7C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 7");
}

void IntHandler8C(void)
{
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 8");
}


void IntHandler10C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 10");
}

void IntHandler13C(void)
{
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Interrupt 13");
}

void IntHandler15C(void)
{
    if(!nInteruptable)
    {
        return;
    }
    XBlastLogger(DEBUG_IRQ, DBG_LVL_DEBUG, "Unhandled Interrupt 15");
}

void IntHandlerException0C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Divide by Zero");
}
void IntHandlerException1C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Single Step");    while(1) ;
}

void IntHandlerException2C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: NMI");    while(1) ;
}
void IntHandlerException3C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Breakpoint");    while(1) ;
}
void IntHandlerException4C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Overflow Trap");    while(1) ;
}
void IntHandlerException5C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: BOUND exceeded");    while(1) ;
}
void IntHandlerException6C(void)
{
    unsigned int dwEbp = 0;
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Invalid Opcode");
    __asm__ __volatile__ ( " mov %%esp, %%eax\n " : "=a" (dwEbp) );
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "   %08X:%08X", *((volatile unsigned int *)(dwEbp+0x48)), *((volatile unsigned int *)(dwEbp+0x44)));
    while(1) ;
}

void IntHandlerException7C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Coprocessor Absent");
}
void IntHandlerException8C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Double Fault");    while(1) ;
}
void IntHandlerException9C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Copro Seg Overrun");
}
void IntHandlerExceptionAC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Invalid TSS");    while(1) ;
}
void IntHandlerExceptionBC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Segment not present");    while(1) ;
}
void IntHandlerExceptionCC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Stack Exception");    while(1) ;
}
void IntHandlerExceptionDC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: General Protection Fault");    while(1) ;
}
void IntHandlerExceptionEC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Page Fault");    while(1) ;
}
void IntHandlerExceptionFC(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Reserved");    while(1) ;
}
void IntHandlerException10C(void)
{
    XBlastLogger(DEBUG_EXCEPTIONS, DBG_LVL_FATAL, "CPU Exc: Copro Error");
}

