// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief VESA extra modes.

#include "../doomdef.h"
#include "../screen.h"



#define MODE_SUPPORTED_IN_HW    0x0001
#define COLOR_MODE              0x0008
#define GRAPHICS_MODE           0x0010
#define VGA_INCOMPATIBLE        0x0020
#define LINEAR_FRAME_BUFFER     0x0080
#define LINEAR_MODE             0x4000

#define MAX_VESA_MODES          30  // we'll just take the first 30 if there


// VESA information block structure
typedef struct vbeinfoblock_s
{
	unsigned char  VESASignature[4]     ATTRPACK;
	unsigned short VESAVersion          /*ATTRPACK*/;
	unsigned long  OemStringPtr         ATTRPACK;
	byte           Capabilities[4];
	unsigned long  VideoModePtr         ATTRPACK;
	unsigned short TotalMemory          ATTRPACK;
	byte           OemSoftwareRev[2];
	byte           OemVendorNamePtr[4];
	byte           OemProductNamePtr[4];
	byte           OemProductRevPtr[4];
	byte           Reserved[222];
	byte           OemData[256];
} vbeinfoblock_t;


// VESA information for a specific mode
typedef struct vesamodeinfo_s
{
	unsigned short ModeAttributes       /*ATTRPACK*/;
	unsigned char  WinAAttributes       ATTRPACK;
	unsigned char  WinBAttributes       ATTRPACK;
	unsigned short WinGranularity       /*ATTRPACK*/;
	unsigned short WinSize              ATTRPACK;
	unsigned short WinASegment          /*ATTRPACK*/;
	unsigned short WinBSegment          ATTRPACK;
	unsigned long  WinFuncPtr           ATTRPACK;
	unsigned short BytesPerScanLine     /*ATTRPACK*/;
	unsigned short XResolution          ATTRPACK;
	unsigned short YResolution          /*ATTRPACK*/;
	unsigned char  XCharSize            ATTRPACK;
	unsigned char  YCharSize            ATTRPACK;
	unsigned char  NumberOfPlanes       ATTRPACK;
	unsigned char  BitsPerPixel         ATTRPACK;
	unsigned char  NumberOfBanks        ATTRPACK;
	unsigned char  MemoryModel          ATTRPACK;
	unsigned char  BankSize             ATTRPACK;
	unsigned char  NumberOfImagePages   ATTRPACK;
	unsigned char  Reserved_page        ATTRPACK;
	unsigned char  RedMaskSize          ATTRPACK;
	unsigned char  RedMaskPos           ATTRPACK;
	unsigned char  GreenMaskSize        ATTRPACK;
	unsigned char  GreenMaskPos         ATTRPACK;
	unsigned char  BlueMaskSize         ATTRPACK;
	unsigned char  BlueMaskPos          ATTRPACK;
	unsigned char  ReservedMaskSize     ATTRPACK;
	unsigned char  ReservedMaskPos      ATTRPACK;
	unsigned char  DirectColorModeInfo  ATTRPACK;

	/* VBE 2.0 extensions */
	unsigned long  PhysBasePtr          /*ATTRPACK*/;
	unsigned long  OffScreenMemOffset   ATTRPACK;
	unsigned short OffScreenMemSize     /*ATTRPACK*/;

	/* VBE 3.0 extensions */
	unsigned short LinBytesPerScanLine  ATTRPACK;
	unsigned char  BnkNumberOfPages     ATTRPACK;
	unsigned char  LinNumberOfPages     ATTRPACK;
	unsigned char  LinRedMaskSize       ATTRPACK;
	unsigned char  LinRedFieldPos       ATTRPACK;
	unsigned char  LinGreenMaskSize     ATTRPACK;
	unsigned char  LinGreenFieldPos     ATTRPACK;
	unsigned char  LinBlueMaskSize      ATTRPACK;
	unsigned char  LinBlueFieldPos      ATTRPACK;
	unsigned char  LinRsvdMaskSize      ATTRPACK;
	unsigned char  LinRsvdFieldPos      ATTRPACK;
	unsigned long  MaxPixelClock        ATTRPACK;

	unsigned char  Reserved[190]        ATTRPACK;
} vesamodeinfo_t;


// setup standard VGA + VESA modes list, activate the default video mode.
void VID_Init (void);
