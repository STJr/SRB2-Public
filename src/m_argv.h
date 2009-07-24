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
/// \brief Command line arguments

#ifndef __M_ARGV__
#define __M_ARGV__

//
// MISC
//
extern int myargc;
extern char **myargv;

// Returns the position of the given parameter in the arg list (0 if not found).
int M_CheckParm(const char *check);

// Pushes all parameters beginning with a +, ex: +map map01
void M_PushSpecialParameters(void);

// Returns true if there are available parameters.
boolean M_IsNextParm(void);

// Returns the next parameter after a M_CheckParm, NULL if not found.
char *M_GetNextParm(void);

// Finds and loads a response file (@moreargs.txt)
void M_FindResponseFile(void);

#endif //__M_ARGV__
