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

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif
#include "common.h"

// ================================== GLOBALS =================================

// used by xxxPrintf() functions as temporary variable
static char str[1024] ="";
static va_list arglist;
static int len = 0;
#ifdef _WIN32
static HANDLE co = INVALID_HANDLE_VALUE;
static DWORD bytesWritten = 0;
static CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
#endif

// ================================= FUNCTIONS ================================

/*
 * clearScreen():
 */
void clearScreen()
{
#ifdef _WIN32
#else
	printf("\033[0m \033[2J \033[0;0H");
#endif
}

/*
 * dbgPrintf():
 */
#ifdef _WIN32
void dbgPrintf(DWORDLONG col, const char *lpFmt, ...)
#else
void dbgPrintf(const char *col, const char *lpFmt, ...)
#endif
{
#if defined (__DEBUG__)
	va_start(arglist, lpFmt);
	len = vsnprintf(str, sizeof str, lpFmt, arglist);
	va_end(arglist);
	if (len <= 0)
		return;

#ifdef _WIN32
	co = GetStdHandle(STD_OUTPUT_HANDLE);

	if (co == INVALID_HANDLE_VALUE)
	return;

	if (col == DEFCOL)
	{
		if (!GetConsoleScreenBufferInfo(co, &ConsoleScreenBufferInfo))
			ConsoleScreenBufferInfo.wAttributes =
				FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

		SetConsoleTextAttribute(co, (DWORD)col);
	}

	if (GetFileType(co) == FILE_TYPE_CHAR)
		WriteConsoleA(co, str, (DWORD)len, &bytesWritten, NULL);
	else
		WriteFile(co, str, (DWORD)len, &bytesWritten, NULL);

	if (col != DEFCOL)
		SetConsoleTextAttribute(co,
			ConsoleScreenBufferInfo.wAttributes);
#else
	printf("%s%s", col, str);
	fflush(stdout);
#endif
#else
	(void)col;
	(void)lpFmt;
#endif
}

/*
 * conPrintf()
 */
#ifdef _WIN32
void conPrintf(DWORDLONG col, const char *lpFmt, ...)
#else
void conPrintf(const char *col, const char *lpFmt, ...)
#endif
{
	va_start(arglist, lpFmt);
	len = vsnprintf(str, sizeof str, lpFmt, arglist);
	va_end(arglist);
	if (len <= 0)
		return;

#ifdef _WIN32
	co = GetStdHandle(STD_OUTPUT_HANDLE);

	if (col == DEFCOL)
	{
		if (!GetConsoleScreenBufferInfo(co, &ConsoleScreenBufferInfo))
			ConsoleScreenBufferInfo.wAttributes =
				FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;

		SetConsoleTextAttribute(co, (WORD)col);
	}

	if (co == INVALID_HANDLE_VALUE)
		return;

	if (GetFileType(co) == FILE_TYPE_CHAR)
		WriteConsoleA(co, str, (DWORD)len, &bytesWritten, NULL);
	else
		WriteFile(co, str, (DWORD)len, &bytesWritten, NULL);

	if (col != DEFCOL)
		SetConsoleTextAttribute(co,
			ConsoleScreenBufferInfo.wAttributes);
#else
	printf("%s%s", col, str);
	fflush(stdout);
#endif
}

/*
 * logPrintf():
 */
void logPrintf(FILE *f, const char *lpFmt, ...)
{
	va_start(arglist, lpFmt);
	len = vsnprintf(str, sizeof str, lpFmt, arglist);
	va_end(arglist);
	if (len <= 0)
		return;

	if (f)
	{
		time_t t = time(NULL);
		char *ct = ctime(&t);
		ct[strlen(ct)-1] = '\0';
		fprintf(f, "%s: %s", ct, str);
		fflush(f);
	}
#ifdef HAVE_SYSLOG
	syslog(LOG_INFO, "%s", str);
#endif
#if defined (__DEBUG__)
#ifdef _WIN32
	printf("%s", str);
#else
	printf("%s%s", DEFCOL, str);
#endif
	fflush(stdout);
#endif
}

/*
 * openFile():
 */
FILE *openFile(const char *filename)
{
	return fopen(filename, "a+t");
}

