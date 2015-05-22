/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Redesigned icon menu, allowing icons to be added/removed at runtime.
 * 02/10/04 - David Pye dmp@davidmpye.dyndns.org
 * You should not need to edit this file in normal circumstances - icons are
 * added/removed via boot/IconMenuInit.c
 */

#include "boot.h"
#include "video.h"
#include "memory_layout.h"
#include <shared.h>
#include <filesys.h>
#include "rc4.h"
#include "sha1.h"
#include "BootFATX.h"
#include "xbox.h"
//#include "BootFlash.h"
#include "cpu.h"
#include "BootIde.h"
#include "MenuActions.h"
#include "config.h"
#include "IconMenu.h"
#include "lib/LPCMod/BootLCD.h"
#include "lpcmod_v1.h"

#define TRANSPARENTNESS 0x30
#define SELECTED 0xff

ICON *firstIcon=NULL;
ICON *selectedIcon=NULL;
ICON *firstVisibleIcon=NULL;    //Could also have been named rightIcon
ICON *lastVisibleIcon=NULL;        //and leftIcon since only 3 icons are shown on the iconMenu.
int timedOut=0;
int iconTimeRemain = 0;
u32 temp=1;



void AddIcon(ICON *newIcon) {
    ICON *iconPtr = firstIcon;
    ICON *currentIcon = NULL;
    while (iconPtr != NULL) {
        currentIcon = iconPtr;
        iconPtr = iconPtr->nextIcon;
    }
    
    if (currentIcon==NULL) { 
        //This is the first icon in the chain
        firstIcon = newIcon;
    }
    //Append to the end of the chain
    else currentIcon->nextIcon = newIcon;
    iconPtr = newIcon;
    iconPtr->nextIcon = NULL;
    iconPtr->previousIcon = currentIcon; 
}

static void IconMenuDraw(int nXOffset, int nYOffset) {
    ICON *iconPtr;            //Icon the code is currently working on.
    ICON *iconSeeker;        //Pointer to the icon we want to be selected at boot.
    int iconcount;
    u8 opaqueness;
    int tempX, tempY;

    u8 uncommittedChanges = LPCMod_CountNumberOfChangesInSettings();

    //Seeking icon with desired bankID value must be done when both firstVisibleIcon and selectedIcon are NULL.
    //This way, seeking desired icon will only occur at initial draw.
/*
    if (firstVisibleIcon==NULL) firstVisibleIcon = firstIcon;
    if (selectedIcon==NULL) selectedIcon = firstIcon;
    iconPtr = firstVisibleIcon;
*/
    if (firstVisibleIcon==NULL && selectedIcon==NULL){
        switch(LPCmodSettings.OSsettings.activeBank){
            default:
            case BNK512:
                firstVisibleIcon = firstIcon;
                selectedIcon = firstIcon;
                break;
            case BNK256:
                firstVisibleIcon = firstIcon;
                selectedIcon = firstIcon->nextIcon;
                break;
            case BNKFULLTSOP:
            case BNKTSOPSPLIT0:
                if(!LPCmodSettings.OSsettings.TSOPhide){
                    firstVisibleIcon = firstIcon->nextIcon;
                    selectedIcon = firstIcon->nextIcon->nextIcon;
                }
                break;
            case BNKTSOPSPLIT1:
                if(!LPCmodSettings.OSsettings.TSOPhide){
                    firstVisibleIcon = firstIcon->nextIcon->nextIcon;
                    selectedIcon = firstIcon->nextIcon->nextIcon->nextIcon;
                }
                break;
        }
    }
    iconPtr = firstVisibleIcon;

    //There are max 3 (three) 'bays' for displaying icons in - we only draw the 3.
    for (iconcount=0; iconcount<3; iconcount++) {
        if (iconPtr==NULL) {
            //No more icons to draw
            return;
        }
        if (iconPtr==selectedIcon) {
            //Selected icon has less transparency
            //and has a caption drawn underneath it
            opaqueness = SELECTED;
            VIDEO_CURSOR_POSX=nXOffset+140*(iconcount+1)*4;
            VIDEO_CURSOR_POSY=nYOffset+20;
            VIDEO_ATTR=0xffffff;
            printk("%s\n",iconPtr->szCaption);
        }
        else opaqueness = TRANSPARENTNESS;
        
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(140*(iconcount+1))) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData+(iconPtr->iconSlot * 3)),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + nXOffset+(140*(iconcount+1))) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
        lastVisibleIcon = iconPtr;
        iconPtr = iconPtr->nextIcon;
    }

    // If there is an icon off screen to the left, draw this icon.
    if(firstVisibleIcon->previousIcon != NULL) {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(50)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + nXOffset+(50)) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    // If there is an icon off screen to the right, draw this icon.
    if(lastVisibleIcon->nextIcon != NULL) {
        //opaqueness = TRANSPARENTNESS;
        opaqueness = SELECTED;
        BootVideoJpegBlitBlend(
            (u8 *)(FB_START+((vmode.width * (nYOffset-74))+nXOffset+(510)) * 4),
            vmode.width, // dest bytes per line
            &jpegBackdrop, // source jpeg object
            (u8 *)(jpegBackdrop.pData+(ICON_WIDTH * 3)),
            0xff00ff|(((u32)opaqueness)<<24),
            (u8 *)(jpegBackdrop.pBackdrop + ((1024 * (nYOffset-74)) + nXOffset+(510)) * 3),
            ICON_WIDTH, ICON_HEIGHT
        );
    }

    tempX = VIDEO_CURSOR_POSX;
    tempY = VIDEO_CURSOR_POSY;
    VIDEO_CURSOR_POSX=((172+((vmode.width-640)/2))<<2);
    VIDEO_CURSOR_POSY=vmode.height - 250;

    if(temp != 0) {
        printk("\2Select from Menu \2 (%i)", iconTimeRemain);    
    } else {
        VIDEO_CURSOR_POSX += 52;
        printk("\2Select from Menu \2");    
    }

    if(uncommittedChanges > 0){
        //There are settings that have changed.
        VIDEO_CURSOR_POSY = vmode.height - 40;
        VIDEO_CURSOR_POSX = vmode.width - 550;
        VIDEO_ATTR=0xffc8c8c8;
        printk("\1Uncommitted changes: %u", uncommittedChanges);
    }

    VIDEO_CURSOR_POSX = tempX;
    VIDEO_CURSOR_POSY = tempY;
}

bool IconMenu(void) {

    bool reloadUI = true;
    u32 COUNT_start;
    int oldIconTimeRemain = 0;
    ICON *iconPtr=NULL;
    char bankString[20];

    extern int nTempCursorMbrX, nTempCursorMbrY; 
    int nTempCursorResumeX, nTempCursorResumeY ;
    int nTempCursorX, nTempCursorY;
    int nModeDependentOffset=(vmode.width-640)/2;  
    u8 varBootTimeWait = LPCmodSettings.OSsettings.bootTimeout;        //Just to have a default value.
    char timeoutString[21];                            //To display timeout countdown on xLCD
    timeoutString[20] = 0;
    
    nTempCursorResumeX=nTempCursorMbrX;
    nTempCursorResumeY=nTempCursorMbrY;

    nTempCursorX=VIDEO_CURSOR_POSX;
    nTempCursorY=vmode.height-80;
    
    // We save the complete framebuffer to memory (we restore at exit)
    videosavepage = malloc(FB_SIZE);
    memcpy(videosavepage,(void*)FB_START,FB_SIZE);
    
    VIDEO_CURSOR_POSX=((252+nModeDependentOffset)<<2);
    VIDEO_CURSOR_POSY=nTempCursorY-100;

    if(LPCmodSettings.OSsettings.bootTimeout == 0 || cromwell_config==XROMWELL ||
            //No countdown if activeBank is set to a TSOP bank and TSOP boot icon are hidden.
       (LPCmodSettings.OSsettings.TSOPhide && (LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT0 ||
                                               LPCmodSettings.OSsettings.activeBank == BNKTSOPSPLIT1 ||
                                               LPCmodSettings.OSsettings.activeBank == BNKFULLTSOP)))
        temp = 0;                                    //Disable boot timeout
    else
        varBootTimeWait = LPCmodSettings.OSsettings.bootTimeout;

//#ifndef SILENT_MODE
    //In silent mode, don't draw the menu the first time.
    //If we get a left/right xpad event, it will be registered,
    //and the menu will 'appear'. Otherwise, it will proceed quietly
    //and boot the default boot item
    VIDEO_ATTR=0xffc8c8c8;
    //printk("Select from Menu\n");
    VIDEO_ATTR=0xffffffff;
    IconMenuDraw(nModeDependentOffset, nTempCursorY);
//#endif
    //Initial LCD string print.
    if(xLCD.enable == 1){
        if(LPCmodSettings.LCDsettings.customTextBoot == 0){
            xLCD.PrintLine[1](CENTERSTRING, selectedIcon->szCaption);
            xLCD.ClearLine(2);
            xLCD.ClearLine(3);
        }
    }
    COUNT_start = IoInputDword(0x8008);
    //Main menu event loop.
    while(reloadUI)
    {
        int changed=0;
        wait_ms(10);    
        if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_RIGHT) == 1)
        {
            if (selectedIcon->nextIcon!=NULL) {
                //A bit ugly, but need to find the last visible icon, and see if 
                //we are moving further right from it.
                lastVisibleIcon=firstVisibleIcon;
                int i=0;
                for (i=0; i<2; i++) {
                    if (lastVisibleIcon->nextIcon==NULL) {
                        break;
                    }
                    lastVisibleIcon = lastVisibleIcon->nextIcon;
                }
                if (selectedIcon == lastVisibleIcon) {
                    //We are moving further right, so slide all the icons along. 
                    if(lastVisibleIcon->nextIcon != NULL) {
                        firstVisibleIcon = firstVisibleIcon->nextIcon;    
                    }
                    //As all the icons have moved, we need to refresh the entire page.
                    memcpy((void*)FB_START,videosavepage,FB_SIZE);
                }
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                selectedIcon = selectedIcon->nextIcon;
                changed=1;
            }
            temp=0;
        }
        else if (risefall_xpad_BUTTON(TRIGGER_XPAD_PAD_LEFT) == 1)
        {
            if (selectedIcon->previousIcon!=NULL) {
                if (selectedIcon == firstVisibleIcon) {
                    //We are moving further left, so slide all the icons along. 
                    if(firstVisibleIcon->previousIcon != NULL) {
                        firstVisibleIcon = firstVisibleIcon->previousIcon;
                    }
                    //As all the icons have moved, we need to refresh the entire page.
                    memcpy((void*)FB_START,videosavepage,FB_SIZE);
                }
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
                selectedIcon = selectedIcon->previousIcon;
                changed=1;
            }
            temp=0;
        }
        //If anybody has toggled the xpad left/right, disable the timeout.
        if(temp != 0) {
            temp = IoInputDword(0x8008) - COUNT_start;
            oldIconTimeRemain = iconTimeRemain;
            iconTimeRemain = varBootTimeWait - temp/0x369E99;
            if(oldIconTimeRemain != iconTimeRemain) {
                changed = 1;
                memcpy((void*)FB_START,videosavepage,FB_SIZE);
            }
        }
        
        if ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) == 1) || risefall_xpad_STATE(XPAD_STATE_START) == 1 || 
            (u32)(temp>(0x369E99*varBootTimeWait))) {
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
            VIDEO_CURSOR_POSX=nTempCursorResumeX;
            VIDEO_CURSOR_POSY=nTempCursorResumeY;
            
            if (temp>(0x369E99*varBootTimeWait)){
                timedOut=1;

            }
            if(xLCD.enable == 1){
                if(selectedIcon->nextIcon != NULL){ //Last Icon in line is Advanced Settings.
                    if(LPCmodSettings.LCDsettings.displayMsgBoot == 0){    //Other icons boots banks so LCD rules apply here.
                        xLCD.Command(DISP_CLEAR);
                    }
                    else if(LPCmodSettings.LCDsettings.customTextBoot == 0){            //Do not use custom strings.
                        if(LPCmodSettings.LCDsettings.displayBIOSNameBoot == 0){        //Do not display bank name
                            xLCD.ClearLine(1);                                          //Only top line will be left on LCD.
                        }
                        else{                                                             //Display booting bank,
                            LPCMod_LCDBankString(bankString, selectedIcon->bankID);
                            xLCD.PrintLine[1](CENTERSTRING, bankString);
                        }
                        xLCD.ClearLine(2); //Clear countdown line on screen since countdown is over and we're about to boot.
                    }
                }
            }
            //Icon selected - invoke function pointer.
            if (selectedIcon->functionPtr!=NULL) selectedIcon->functionPtr(selectedIcon->functionDataPtr);
            //If we come back to this menu, make sure we are redrawn, and that we replace the saved video page
            changed=1;
            memcpy((void*)FB_START,videosavepage,FB_SIZE);
        }

        if (changed) {
            BootVideoClearScreen(&jpegBackdrop, nTempCursorY, VIDEO_CURSOR_POSY+1);
            IconMenuDraw(nModeDependentOffset, nTempCursorY);
            //LCD string print.
            if(xLCD.enable == 1){
                if(LPCmodSettings.LCDsettings.customTextBoot == 0){
                    LPCMod_LCDBankString(bankString, selectedIcon->bankID);
                    xLCD.PrintLine[1](CENTERSTRING, bankString);
                    if(LPCmodSettings.LCDsettings.nbLines >= 4){
                        if(temp != 0) {
                            sprintf(timeoutString, "Auto boot in %ds", iconTimeRemain);
                            xLCD.PrintLine[2](CENTERSTRING, timeoutString);
                        }
                        else {
                            xLCD.ClearLine(2);
                        }
                        xLCD.ClearLine(3);
                    }
                }
            }
            changed=0;
        }
        
    }
    return 1;   //Always return 1 to stay in while loop in BootResetAction.
}


void freeIconMenuAllocMem(void){
    ICON *currentIcon = firstIcon;

    //Go to last icon in list
    while(currentIcon->nextIcon != NULL){
        currentIcon = currentIcon->nextIcon;
    }
    //While were not back at the first icon
    while(currentIcon->previousIcon != NULL){
        if(currentIcon->dataPtrAlloc){
            free(currentIcon->functionDataPtr);
        }
        currentIcon = currentIcon->previousIcon;
        free(currentIcon->nextIcon);
    }

    //Last icon free(back to firstIcon)
    if(currentIcon->dataPtrAlloc){
        free(currentIcon->functionDataPtr);
    }
    free(currentIcon);
    currentIcon = NULL;
    firstIcon = NULL;
    firstVisibleIcon = NULL;
    lastVisibleIcon = NULL;
}

void repositionIconPtrs(void){
    lastVisibleIcon = firstIcon;
    //Move up to last Icon (Advanced Settings).
    while(lastVisibleIcon->nextIcon!= NULL){
        lastVisibleIcon = lastVisibleIcon->nextIcon;
    }
    //Set First visible Icon 2 icons aways from last's.
    if(lastVisibleIcon->previousIcon != NULL){
        firstVisibleIcon = lastVisibleIcon->previousIcon;
        if(lastVisibleIcon->previousIcon->previousIcon != NULL){
            firstVisibleIcon = lastVisibleIcon->previousIcon->previousIcon;
        }
    }
    else
        firstVisibleIcon = firstIcon;
    selectedIcon = lastVisibleIcon; //Reselect Advanced Settings icon
}
