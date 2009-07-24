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
/// \brief Dehacked files.

#ifndef __DEHACKED_H__
#define __DEHACKED_H__

typedef enum
{
	UNDO_NONE    = 0x00,
	UNDO_NEWLINE = 0x01,
	UNDO_SPACE   = 0x02,
	UNDO_CUTLINE = 0x04,
	UNDO_HEADER  = 0x07,
	UNDO_ENDTEXT = 0x08,
	UNDO_TODO = 0,
	UNDO_DONE = 0,
} undotype_f;

void DEH_WriteUndoline(const char *value, const char *data, undotype_f flags);
void DEH_UnloadDehackedWad(USHORT wad);

void DEH_LoadDehackedLump(lumpnum_t lumpnum);
void DEH_LoadDehackedLumpPwad(USHORT wad, USHORT lump);

extern boolean deh_loaded, modcredits;

#define MAXLINELEN 1024

// the code was first write for a file
// converted to use memory with this functions
typedef struct
{
	char *data;
	char *curpos;
	size_t size;
} MYFILE;
#define myfeof(a) (a->data + a->size <= a->curpos)
char *myfgets(char *buf, int bufsize, MYFILE *f);
#endif
