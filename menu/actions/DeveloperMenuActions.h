#ifndef _DEVELOPERMENUACTIONS_H_
#define _DEVELOPERMENUACTIONS_H_
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

void LPCIOWrite(void * ignored);
void LPCIORead(void * ignored);

void SMBusRead(void * ignored);

void GPIORead(void * ignored);

void settingsPrintData(void * ignored);

void printBiosIdentifier(void * ignored);

#endif
