// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2004 by Sonic Team Jr.
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
// DESCRIPTION:
//      stub and replacement "ANSI" C functions for use under OpenXDK
//
//-----------------------------------------------------------------------------

#include "../../doomdef.h"
#include "xboxhelp.h"
#ifdef __GNUC__
#include <unistd.h>
#else
#include <stdlib.h>
#endif

int access(const char *path, int amode)
{
	int accesshandle = 1;
	FILE *handle = NULL;
	if (amode == 6)
		handle = fopen(path, "r+");
	else if (amode == 4)
		handle = fopen(path, "r");
	else if (amode == 2)
		handle = fopen(path, "a+");
	else if (amode == 0)
		handle = fopen(path, "rb");
	if (handle)
	{
		accesshandle = 0;
		fclose(handle);
	}
	return accesshandle;
}

char *getcwd(char *_buf, size_t _size )
{
	_buf = NULL;
	_size = 0;
	return _buf;
}

#ifdef __GNUC__
int mkdir(const char *path, mode_t _mode)
{
	path = NULL;
	_mode = 0;
	return 0;
}
#else
int mkdir(const char *path)
{
	path = NULL;
	return 0;
}
#endif

int chdir (const char *__path )
{
	__path = NULL;
	return 0;
}

time_t time(time_t *T)
{
	long returntime = 0;
	T = NULL;
/*
	SYSTEMTIME st;
	FILETIME stft;
	INT64 ftli;
	if (!T) return returntime;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&stft);
	CopyMemory(&ftli,&stft,sizeof (LARGE_INTEGER));
	returntime = (long)ftli;
	*T = returntime;
*/
	return returntime;
}

#ifdef _MSC_VER
#include <RtcApi.h>
void __cdecl _RTC_Initialize(void)
{
}
char *getenv(const char *__env)
{
	__env = NULL;
	return NULL;
}

int putenv(const char *__env)
{
	__env = NULL;
	return 0;
}
#endif
