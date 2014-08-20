/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ModchipMenuActions.h"
#include "TextMenu.h"
#include "lpcmod_v1.h"
#include "boot.h"
#include "LEDMenuActions.h"


void decrementActiveBank(void * itemStr) {
	switch(LPCmodSettings.OSsettings.activeBank){
	case BNK512:
		if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x01){		//TSOP is split
			if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02){		//4-way split
				LPCmodSettings.OSsettings.activeBank = BNKTSOP3;
				sprintf(itemStr,"TSOP bank3");
			}
			else{
				LPCmodSettings.OSsettings.activeBank = BNKTSOP1;
				sprintf(itemStr,"TSOP bank1");
			}
		}
		else {
			LPCmodSettings.OSsettings.activeBank = BNKTSOP;
			sprintf(itemStr,"TSOP");
		}
		break;
	case BNK256:
		LPCmodSettings.OSsettings.activeBank = BNK512;
		sprintf(itemStr,"512KB");
		break;
	case BNKTSOP:
		LPCmodSettings.OSsettings.activeBank = BNK256;
		sprintf(itemStr,"256KB");
		break;
	case BNKTSOP1:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP;
		if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x01){		//TSOP is split
			sprintf(itemStr,"TSOP bank0");
		}
		else {
			sprintf(itemStr,"TSOP");
		}
		break;
	case BNKTSOP2:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP1;
		sprintf(itemStr,"TSOP bank1");
		break;
	case BNKTSOP3:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP2;
		sprintf(itemStr,"TSOP bank2");
		break;
	}
	return;
}



void incrementActiveBank(void * itemStr) {
	switch(LPCmodSettings.OSsettings.activeBank){
	case BNK512:
		LPCmodSettings.OSsettings.activeBank = BNK256;
		sprintf(itemStr,"256KB");
		break;
	case BNK256:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP;
		if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){
			sprintf(itemStr,"TSOP bank0");
		}
		else {
			sprintf(itemStr,"TSOP");
		}
		break;
	case BNKTSOP:
		if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){
			LPCmodSettings.OSsettings.activeBank = BNKTSOP1;
			sprintf(itemStr,"TSOP bank1");
		}
		else {
			LPCmodSettings.OSsettings.activeBank = BNK512;
			sprintf(itemStr,"512KB");
		}
		break;
	case BNKTSOP1:
		if((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02){		//TSOP is split 4-way
			LPCmodSettings.OSsettings.activeBank = BNKTSOP2;
			sprintf(itemStr,"TSOP bank2");
		}
		else {
			LPCmodSettings.OSsettings.activeBank = BNK512;		//TSOP is split 2-way
			sprintf(itemStr,"512KB");							//So go back to modchip's bank0.
		}
		break;
	case BNKTSOP2:
		LPCmodSettings.OSsettings.activeBank = BNKTSOP3;
		sprintf(itemStr,"TSOP bank3");
		break;
	case BNKTSOP3:
		LPCmodSettings.OSsettings.activeBank = BNK512;
		sprintf(itemStr,"512KB");
		break;
	}
	return;
}


void decrementbootTimeout(void * itemStr){
	if(LPCmodSettings.OSsettings.bootTimeout > 0)	//Logic
		LPCmodSettings.OSsettings.bootTimeout -= 1;
	sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
	return;
}
void incrementbootTimeout(void * itemStr){
	if(LPCmodSettings.OSsettings.bootTimeout < 240)	//I've got to set a limit there.
		LPCmodSettings.OSsettings.bootTimeout += 1;
	sprintf(itemStr, "%ds", LPCmodSettings.OSsettings.bootTimeout);
	return;
}

void toggleQuickboot(void * itemStr){
	(LPCmodSettings.OSsettings.Quickboot) = (LPCmodSettings.OSsettings.Quickboot)? 0 : 1;
	sprintf(itemStr,"%s",LPCmodSettings.OSsettings.Quickboot? "Yes" : "No");
}

void resetSettings(void *whatever){
	initialLPCModOSBoot(&LPCmodSettings);
	QuickReboot();
}

void toggleTSOPControl(void * itemPtr){
	TEXTMENUITEM * tempItemPtr = (TEXTMENUITEM *)&itemPtr;
	if(LPCmodSettings.OSsettings.TSOPcontrol & 0x01){			//If already active
		LPCmodSettings.OSsettings.TSOPcontrol &= 0xFE;	//Make sure to toggle only bit0.
	}
	else{
		LPCmodSettings.OSsettings.TSOPcontrol |= 0x01;	//Make sure to toggle only bit0.
	}
	sprintf(tempItemPtr->szParameter,"%s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x01? "Yes" : "No");
	sprintf(tempItemPtr->nextMenuItem->szParameter, "%s",
		(LPCmodSettings.OSsettings.TSOPcontrol) & 0x01?	((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "4-way" : "2-way") : "No");
}

void toggleTSOPSplit(void * itemStr){
	if((LPCmodSettings.OSsettings.TSOPcontrol & 0x02) || !(LPCmodSettings.OSsettings.TSOPcontrol & 0x01)){	//If TSOPControl bit1 is set or bit0 is not
		LPCmodSettings.OSsettings.TSOPcontrol &= 0xFD;	//Make sure to toggle only bit1.
	}
	else {
		LPCmodSettings.OSsettings.TSOPcontrol |= 0x02;	//Make sure to toggle only bit1.
	}
	sprintf(itemStr, "%s", (LPCmodSettings.OSsettings.TSOPcontrol) & 0x01?	((LPCmodSettings.OSsettings.TSOPcontrol) & 0x02? "4-way" : "2-way") : "No");
}
