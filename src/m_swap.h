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
/// \brief Endianess handling, swapping 16bit and 32bit

#ifndef __M_SWAP__
#define __M_SWAP__

#ifdef __GNUG__
#pragma interface
#endif

// Endianess handling.
// WAD files are stored little endian.
#ifdef __BIG_ENDIAN__

#define SHORT(x) ((short)(\
(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) \
| \
(((unsigned short)(x) & (unsigned short)0xff00U) >> 8))) \

#define LONG(x) ((int)(\
(((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) \
| \
(((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) \
| \
(((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) \
| \
(((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24)))

#else
#define SHORT(x) ((short)(x))
#define LONG(x)	((int)(x))
#endif

#endif
