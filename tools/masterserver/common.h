// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000 by DooM Legacy Team.
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
//
//-----------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __GNUC__
#include <unistd.h>
#elif defined ( _MSC_VER)
#include <wtypes.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined (_WIN32) || defined (__OS2__)
#ifdef __GNUC__
#define strcasecmp stricmp
#elif defined ( _MSC_VER)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#pragma warning(disable :  4244)
#endif
#endif

// ================================ DEFINITIONS ===============================


#ifdef _WIN32
#ifdef __GNUC__
#include <windef.h>
#include <stdarg.h>
#include <winbase.h>
#include <wincon.h>
#endif
#define DEFCOL     0xD0E0F0C0 // codes couleurs ANSI
#define BLACK      0
#define RED        FOREGROUND_RED
#define GREEN      FOREGROUND_GREEN
#define BROWN      FOREGROUND_GREEN|FOREGROUND_RED
#define BLUE       FOREGROUND_BLUE
#define PURPLE     FOREGROUND_RED|FOREGROUND_BLUE
#define CYAN       FOREGROUND_BLUE|FOREGROUND_GREEN
#define LIGHTGRAY  FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED
#define DARKGRAY   FOREGROUND_INTENSITY
#define LIGHTRED   FOREGROUND_RED|FOREGROUND_INTENSITY
#define LIGHTGREEN FOREGROUND_GREEN|FOREGROUND_INTENSITY
#define YELLOW     FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY
#define LIGHTBLUE  FOREGROUND_BLUE|FOREGROUND_INTENSITY
#define MAGENTA    FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY
#define LIGHTCYAN  FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY
#define WHITE      FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY

#else

#define DEFCOL     "\033[0m"   // codes couleurs ANSI
#define BLACK      "\033[0;30m"
#define RED        "\033[0;31m"
#define GREEN      "\033[0;32m"
#define BROWN      "\033[0;33m"
#define BLUE       "\033[0;34m"
#define PURPLE     "\033[0;35m"
#define CYAN       "\033[0;36m"
#define LIGHTGRAY  "\033[0;37m"
#define DARKGRAY   "\033[1;30m"
#define LIGHTRED   "\033[1;31m"
#define LIGHTGREEN "\033[1;32m"
#define YELLOW     "\033[1;33m"
#define LIGHTBLUE  "\033[1;34m"
#define MAGENTA    "\033[1;35m"
#define LIGHTCYAN  "\033[1;36m"
#define WHITE      "\033[1;37m"

#endif

#define NUM_ELEMENTS(arr)	(sizeof(arr)/sizeof(arr[0]))

// ================================== PROTOS ==================================

void clearScreen();
void logPrintf(FILE *, const char *, ...);
#ifdef _WIN32
void dbgPrintf(DWORDLONG col, const char *lpFmt, ...);
void conPrintf(DWORDLONG col, const char *lpFmt, ...);
#else
void dbgPrintf(const char *col, const char *lpFmt, ...);
void conPrintf(const char *col, const char *lpFmt, ...);
#endif
FILE *openFile(const char *filename);
const char *pCrypt(const char *pw, const char *salt);

// ================================== EXTERNS =================================

#endif
