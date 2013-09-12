// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
/// \brief  Internally used data structures for virtually everything,
///	key definitions, lots of other stuff.

#ifndef __DOOMDEF__
#define __DOOMDEF__

#ifdef _WINDOWS
#if !defined (HWRENDER) && !defined (NOHW)
#define HWRENDER
#endif
// judgecutor: 3D sound support
#if !defined(HW3SOUND) && !defined (NOHS)
#define HW3SOUND
#endif
#endif

#if defined (_WIN32) || defined (_WIN32_WCE)
#define ASMCALL __cdecl
#else
#define ASMCALL
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4127 4152 4213 4514)
#ifdef _WIN64
#pragma warning(disable : 4306)
#endif
#endif
// warning level 4
// warning C4127: conditional expression is constant
// warning C4152: nonstandard extension, function/data pointer conversion in expression
// warning C4213: nonstandard extension used : cast on l-value

#if defined (_WIN32_WCE) && defined (DEBUG) && defined (ARM)
#if defined (ARMV4) || defined (ARMV4I)
//#pragma warning(disable : 1166)
// warning LNK1166: cannot adjust code at offset=
#endif
#endif


#include "doomtype.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#if !defined (_WIN32_WCE)
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <ctype.h>

#if ((defined (_WIN32) && !defined (_WIN32_WCE)) || defined (__DJGPP__)) && !defined (_XBOX)
#include <io.h>
#endif

#ifdef PC_DOS
#include <conio.h>
#endif

//#define NOMD5

// Uncheck this to compile debugging code
//#define RANGECHECK
//#ifndef PARANOIA
//#define PARANOIA // do some tests that never fail but maybe
// turn this on by make etc.. DEBUGMODE = 1 or use the Debug profile in the VC++ projects
//#endif
#if defined (_WIN32) || defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON) || defined (macintosh)
#define LOGMESSAGES // write message in log.txt
#endif

#if defined (LOGMESSAGES) && !defined (_WINDOWS)
extern FILE *logstream;
#endif

#define VERSION 200 // Game version
#define VERSIONSTRING " v2.0.7"

// HIGHLY IMPORTANT Modification Options, MUST be changed when creating a modification (or updating SRB2),
// else a lot of errors will occur when trying to access the Master Server.
// If you are just making clientside fixes for private use, that will not break netplay,
// these options are not important, but any builds not compatible with the normal SRB2 release
// must change these options accordingly.  If they are compatible with normal releases
// but you would still like to enable the updating feature, you must still change these values.

// For future use, the codebase is the version of SRB2 that the modification is based on,
// and should not be changed unless you have merged changes between versions of SRB2
// (such as 2.0.4 to 2.0.5, etc) into your working copy.
// Will always resemble the versionstring, 205 = 2.0.5, 210 = 2.1, etc.
#define CODEBASE 207

// The Modification ID (1 = Official Build), must be obtained from Inuyasha
// ( http://mb.srb2.org/private.php?do=newpm&u=2604 ).
// DO NOT try to set this otherwise, or your modification will be unplayable through the Master Server.
#define MODID 1

// The Modification Version, starting from 1. Do not follow your version string for this,
// it's only for detection of the version the player is using so the MS can alert them of an update.
// Only set it higher, not lower, obviously.
#define MODVERSION 207

// Comment out this line to completely disable update alerts (recommended for testing, but not for release)
#define UPDATE_ALERT

 // The string used in the alert that pops up in the event of an update being available.
 // Please change to apply to your modification (we don't want everyone asking where your mod is on SRB2.org!).
#define UPDATE_ALERT_STRING "New updates are available for SRB2\nPlease visit SRB2.org to download them.\n\nYou will not be able to connect to the\nMaster Server until you have updated to\nthe latest version!\n\nCurrent Version: %s\nLatest Version: %s"

// some tests, enable or disable it if it run or not
#define SPLITSCREEN

// =========================================================================

// The maximum number of players, multiplayer/networking.
// NOTE: it needs more than this to increase the number of players...

#define MAXPLAYERS 32
#define MAXSKINS MAXPLAYERS
#define PLAYERSMASK (MAXPLAYERS-1)
#define MAXPLAYERNAME 21
#define MAXSKINCOLORS 16
#define MAXROOMS 32

// State updates, number of tics / second.
// NOTE: used to setup the timer rate, see I_StartupTimer().
#define OLDTICRATE 35
#define NEWTICRATERATIO 1 // try 4 for 140 fps :)
#define TICRATE (OLDTICRATE*NEWTICRATERATIO)

#define RING_DIST 512*FRACUNIT // how close you need to be to a ring to attract it
#define LIMIT_DRAW_DIST (2048*FRACUNIT)

#define PUSHACCEL (2*FRACUNIT) // Acceleration for MF2_SLIDEPUSH items.

// Name of local directory for config files and savegames
#if !defined(_arch_dreamcast) && !defined(_WIN32_WCE) && !defined(GP2X)
#if ((defined (__unix__) || defined (UNIXCOMMON)) && !defined (__CYGWIN__)) && !defined (__APPLE__)
#define DEFAULTDIR ".srb2"
#else
#define DEFAULTDIR "srb2"
#endif
#endif

#include "g_state.h"

// commonly used routines - moved here for include convenience

/**	\brief	The I_Error function

	\param	error	the error message

	\return	void


*/
void I_Error(const char *error, ...) FUNCIERROR;

// console.h
void CONS_Printf(const char *fmt, ...) FUNCPRINTF;

#include "m_swap.h"

// m_misc.h
extern void *(*M_Memcpy)(void* dest, const void* src, size_t n) FUNCNONNULL;
char *va(const char *format, ...) FUNCPRINTF;

// d_main.c
extern boolean devparm; // development mode (-debug)

// =======================
// Misc stuff for later...
// =======================

// if we ever make our alloc stuff...
#define ZZ_Alloc(x) Z_Malloc(x, PU_STATIC, NULL)

// i_system.c, replace getchar() once the keyboard has been appropriated
INT32 I_GetKey(void);

#ifndef min // Double-Check with WATTCP-32's cdefs.h
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef max // Double-Check with WATTCP-32's cdefs.h
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif

// An assert-type mechanism.
#ifdef PARANOIA
#define I_Assert(e) ((e) ? (void)0 : I_Error("assert failed: %s, file %s, line %d", #e, __FILE__, __LINE__))
#else
#define I_Assert(e) ((void)0)
#endif

// The character that separates pathnames. Forward slash on
// most systems, but reverse solidus (\) on Windows and DOS.
#if defined (PC_DOS) || defined (_WIN32)
	#define PATHSEP "\\"
#else
	#define PATHSEP "/"
#endif

// Compile date and time and revision.
extern const char *compdate, *comptime, *comprevision;

// Disabled code and code under testing

//#define SLOPENESS // Fun experimental slope stuff!
//#define WEAPON_SFX
//#define FISHCAKE /// \todo Remove this to disable cheating. Remove for release!
//#define JOHNNYFUNCODE
#define SHUFFLE //Incomplete OpenGL sorting code
//#define CHAOSISNOTDEADYET // Pre-1.08 Chaos gametype code
//#define POLYOBJECTS_PLANES // Polyobject fake flat code
//#define BLUE_SPHERES // Blue spheres for future use.
#define NEWPING //Improved way of dealing with ping values and a ping limit.
#define SEENAMES // See name of player in your crosshair
#define TRANSFIX //Gets translation table from mobj->skin rather than player->skin. Ported from SRB2JTE.

#ifdef SHUFFLE
#define HARDWAREFIX //"New" hardware lighting, colormapping and fog. Ported from SRB2JTE.
#endif

#endif // __DOOMDEF__
