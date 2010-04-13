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
	unsigned char  VESASignature[4];
	UINT16         VESAVersion;
	unsigned long  OemStringPtr;
	UINT8          Capabilities[4];
	unsigned long  VideoModePtr;
	UINT16         TotalMemory;
	UINT8          OemSoftwareRev[2];
	UINT8          OemVendorNamePtr[4];
	UINT8          OemProductNamePtr[4];
	UINT8          OemProductRevPtr[4];
	UINT8          Reserved[222];
	UINT8          OemData[256];
}  ATTTPACK vbeinfoblock_t;


// VESA information for a specific mode
typedef struct vesamodeinfo_s
{
	UINT16         ModeAttributes;
	unsigned char  WinAAttributes;
	unsigned char  WinBAttributes;
	UINT16         WinGranularity;
	UINT16         WinSize;
	UINT16         WinASegment;
	UINT16         WinBSegment;
	unsigned long  WinFuncPtr;
	UINT16         BytesPerScanLine;
	UINT16         XResolution;
	UINT16         YResolution;
	unsigned char  XCharSize;
	unsigned char  YCharSize;
	unsigned char  NumberOfPlanes;
	unsigned char  BitsPerPixel;
	unsigned char  NumberOfBanks;
	unsigned char  MemoryModel;
	unsigned char  BankSize;
	unsigned char  NumberOfImagePages;
	unsigned char  Reserved_page;
	unsigned char  RedMaskSize;
	unsigned char  RedMaskPos;
	unsigned char  GreenMaskSize;
	unsigned char  GreenMaskPos;
	unsigned char  BlueMaskSize;
	unsigned char  BlueMaskPos;
	unsigned char  ReservedMaskSize;
	unsigned char  ReservedMaskPos;
	unsigned char  DirectColorModeInfo;

	/* VBE 2.0 extensions */
	unsigned long  PhysBasePtr;
	unsigned long  OffScreenMemOffset;
	UINT16         OffScreenMemSize;

	/* VBE 3.0 extensions */
	UINT16         LinBytesPerScanLine;
	unsigned char  BnkNumberOfPages;
	unsigned char  LinNumberOfPages;
	unsigned char  LinRedMaskSize;
	unsigned char  LinRedFieldPos;
	unsigned char  LinGreenMaskSize;
	unsigned char  LinGreenFieldPos;
	unsigned char  LinBlueMaskSize;
	unsigned char  LinBlueFieldPos;
	unsigned char  LinRsvdMaskSize;
	unsigned char  LinRsvdFieldPos;
	unsigned long  MaxPixelClock;

	unsigned char  Reserved[190];
} ATTRPACK vesamodeinfo_t;


// setup standard VGA + VESA modes list, activate the default video mode.
void VID_Init (void);
