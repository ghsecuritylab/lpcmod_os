#ifndef _MENUACTIONS_H_
#define _MENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//For the icon->text menu jump
void AdvancedMenu(void *menu);

//Used to display child menus
void DrawChildTextMenu(void *menu);

void SetWidescreen(void *);
void SetVideoStandard(void *);
void SetLEDColor(void *);


void DrawBootMenu(void *entry);
void BootMenuEntry(void *entry);

//Ick, this needs to be removed.
void BootFromCD(void *driveId);
void BootFromNet(void *whatever);

#ifdef FLASH
//void FlashBios(void *);
#endif

#ifdef ETHERBOOT
void BootFromEtherboot(void *);
#endif

// Booting Original Bios
void BootOriginalBios(void * data);

// Booting 256k Modbios
void BootModBios(void * data);

// Booting 512k Modbios
void BootModBios2(void * data);

#endif
