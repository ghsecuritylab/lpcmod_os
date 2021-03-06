/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "MenuInits.h"
#include "boot.h"
#include "BootIde.h"
#include "memory_layout.h"
#include "FatFSAccessor.h"
#include "BFMBootMenuActions.h"
#include "string.h"

TEXTMENU* BFMBootMenuInit(void)
{
    TEXTMENUITEM* itemPtr;
    TEXTMENU* menuPtr;
    FATXFILEINFO fileinfo;
    FATXPartition* partition;

    char* fnames[4096]; //Because Each dir can have up to 4096 files when not in root of partition.
    short n = 0, i = 0;
    int bioses = 0;
    int res;
    int dcluster;
    char* path="\\XBlast\\BFM\\";      //And we're not in root.
    char fullPath[20];
    char* fullPathptr = fullPath;
    for(i = 0; i < 4096; i++)   //Not really useful but good practice.
    {
        fnames[i] = NULL;
    }
    memset(fullPath, 0, 20);

    // Generate the menu title.
    strcpy(fullPath, "'C:");
    fullPathptr += 3;
    strcpy(fullPathptr, path);
    fullPathptr += strlen(path);
    strcpy(fullPathptr, "'");
    fullPathptr = NULL;

    //Only supports BIOS file fetch from Master HDD.
    partition = OpenFATXPartition(0, SECTOR_SYSTEM, SYSTEM_SIZE);

    menuPtr = calloc(1, sizeof(TEXTMENU));

    strcpy(menuPtr->szCaption, fullPath);

    if(partition != NULL)
    {
        dcluster = FATXFindDir(partition, FATX_ROOT_FAT_CLUSTER, "XBlast");

        if((dcluster != -1) && (dcluster != 1))
        {
            dcluster = FATXFindDir(partition, dcluster, "BFM");
        }

        if((dcluster != -1) && (dcluster != 1))
        {
            n = FATXListDir(partition, dcluster, &fnames[0], 4096, path);
            for (i = 0; i < n; i++)
            {
                // Check the file.
                res = FATXFindFile(partition, fnames[i], FATX_ROOT_FAT_CLUSTER, &fileinfo);

                if(res && (fileinfo.fileSize % (256 * 1024) == 0))
                {
                    // If it's a (readable) file - i.e. not a directory.
                    // AND it's filesize is divisible by 256k.
                    itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                    strcpy(itemPtr->szCaption, fnames[i]+strlen(path));
                    itemPtr->functionPtr = bootBFMBios;
                    itemPtr->functionDataPtr = fnames[i];       //allocating char* pointer contained in char **fnames so char **fnames can be destroyed
                    TextMenuAddItem(menuPtr, itemPtr);
                    bioses++;
                }
            }
            if(n < 1)
            {
                // If there were no directories and no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No files in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
            else if(bioses == 0)
            {
                // If there were directories, but no files.
                itemPtr = calloc(1, sizeof(TEXTMENUITEM));
                sprintf(itemPtr->szCaption, "No BIOS in %s.", fullPath);
                itemPtr->functionPtr = NULL;
                TextMenuAddItem(menuPtr, itemPtr);
            }
        }
        else
        {
            // If C:\BIOS doesnt exist.
            itemPtr = calloc(1, sizeof(TEXTMENUITEM));
            sprintf(itemPtr->szCaption, "%s does not exist.", fullPath);
            itemPtr->functionPtr = NULL;
            TextMenuAddItem(menuPtr, itemPtr);
        }

        CloseFATXPartition(partition);
    }
    else
    {
        // If the partition couldn't be opened at all.
        itemPtr = calloc(1, sizeof(TEXTMENUITEM));
        sprintf(itemPtr->szCaption, "Error reading C:\\ partition.");
        itemPtr->functionPtr = NULL;
        TextMenuAddItem(menuPtr, itemPtr);
    }

    return menuPtr;
}
