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
/// \brief Main program, simply calls D_SRB2Main and D_SRB2Loop, the high level loop.

#include "../doomdef.h"

#include "../m_argv.h"
#include "../d_main.h"
#include "../i_system.h"

#ifdef __GNUC__
#include <unistd.h>
#endif

#ifdef SDL

#ifdef SDLMAIN
#include "SDL_main.h"
#elif defined(FORCESDLMAIN)
extern int SDL_main(int argc, char *argv[]);
#endif

#ifdef LOGMESSAGES
FILE *logstream = NULL;
#endif

#ifndef DOXYGEN
#ifndef O_TEXT
#define O_TEXT 0
#endif

#ifndef O_SEQUENTIAL
#define O_SEQUENTIAL 0
#endif
#endif

#if defined (_WIN32) && !defined (_XBOX)
#include "../win32/win_dbg.h"
typedef BOOL (WINAPI *p_IsDebuggerPresent)(VOID);
#endif

#ifdef _arch_dreamcast
#include <arch/arch.h>
KOS_INIT_FLAGS(INIT_DEFAULT
//| INIT_NET
//| INIT_MALLOCSTATS
//| INIT_QUIET
//| INIT_OCRAM
//| INIT_NO_DCLOAD
);
#endif

#if defined (_WIN32) && !defined (_XBOX) && !defined (_WIN32_WCE)
static inline VOID MakeCodeWritable(VOID)
{
#ifdef USEASM // Disable write-protection of code segment
	DWORD OldRights;
	const DWORD NewRights = PAGE_EXECUTE_READWRITE;
	PBYTE pBaseOfImage = (PBYTE)GetModuleHandle(NULL);
	PIMAGE_DOS_HEADER dosH =(PIMAGE_DOS_HEADER)pBaseOfImage;
	PIMAGE_NT_HEADERS ntH = (PIMAGE_NT_HEADERS)(pBaseOfImage + dosH->e_lfanew);
	PIMAGE_OPTIONAL_HEADER oH = (PIMAGE_OPTIONAL_HEADER)
		((PBYTE)ntH + sizeof (IMAGE_NT_SIGNATURE) + sizeof (IMAGE_FILE_HEADER));
	LPVOID pA = pBaseOfImage+oH->BaseOfCode;
	SIZE_T pS = oH->SizeOfCode;
#if 1 // try to find the text section
	PIMAGE_SECTION_HEADER ntS = IMAGE_FIRST_SECTION (ntH);
	WORD s;
	for (s = 0; s < ntH->FileHeader.NumberOfSections; s++)
	{
		if (memcmp (ntS[s].Name, ".text\0\0", 8) == 0)
		{
			pA = pBaseOfImage+ntS[s].VirtualAddress;
			pS = ntS[s].Misc.VirtualSize;
			break;
		}
	}
#endif

	if (!VirtualProtect(pA,pS,NewRights,&OldRights))
		I_Error("Could not make code writable\n");
#endif
}
#endif


/**	\brief	The main function

	\param	argc	number of arg
	\param	*argv	string table

	\return	int
*/
FUNCNORETURN
#if defined (_XBOX) && defined (__GNUC__)
void XBoxStartup()
{
	const char *logdir = NULL;
	myargc = -1;
	myargv = NULL;
#else
#ifdef FORCESDLMAIN
int SDL_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
	const char *logdir = NULL;
	myargc = argc;
	myargv = argv; /// \todo pull out path to exe from this string
#endif

	logdir = D_Home();

#ifdef LOGMESSAGES
#if defined(_WIN32_WCE) || defined(GP2X)
	logstream = fopen(va("%s.log",argv[0]), "a");
#elif defined(DEFAULTDIR)
	if (logdir)
		logstream = fopen(va("%s/"DEFAULTDIR"/srb2log.txt",logdir), "a");
	else
#endif
		logstream = fopen("./srb2log.txt", "a");
#endif

	//CONS_Printf ("I_StartupSystem() ...\n");
	I_StartupSystem();
#if defined (_WIN32) && !defined (_XBOX)
#ifndef _WIN32_WCE
	{
		p_IsDebuggerPresent pfnIsDebuggerPresent = (p_IsDebuggerPresent)GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsDebuggerPresent");
		if ((!pfnIsDebuggerPresent || !pfnIsDebuggerPresent())
#ifdef BUGTRAP
			&& !InitBugTrap()
#endif
			)
		{
			LoadLibraryA("exchndl.dll");
		}
	}
#endif
	prevExceptionFilter = SetUnhandledExceptionFilter(RecordExceptionInfo);
#ifndef _WIN32_WCE
	MakeCodeWritable();
#endif
#endif
	// startup SRB2
	CONS_Printf ("Setting up SRB2...\n");
	D_SRB2Main();
	CONS_Printf ("Entering main game loop...\n");
	// never return
	D_SRB2Loop();

#ifdef BUGTRAP
	// This is safe even if BT didn't start.
	ShutdownBugTrap();
#endif

	// return to OS
#ifndef __GNUC__
	return 0;
#endif
}
#endif
