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
/// \brief Handles WAD file header, directory, lump I/O

#ifdef __GNUC__
#include <unistd.h>
#endif

#define ZWAD

#ifdef ZWAD
#ifdef _WIN32_WCE
#define AVOID_ERRNO
#else
#include <errno.h>
#endif
#include "lzf.h"
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h" // rendermode
#include "d_netfil.h"
#include "dehacked.h"
#include "d_clisrv.h"
#include "r_defs.h"
#include "i_system.h"
#include "md5.h"

#ifdef HWRENDER
#include "r_data.h"
#include "hardware/hw_main.h"
#include "hardware/hw_glob.h"
#endif

#ifdef PC_DOS
#include <stdio.h> // for snprintf
int	snprintf(char *str, size_t n, const char *fmt, ...);
//int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

// Store lists of lumps for F_START/F_END etc.
typedef struct
{
	INT32 wadfile;
	INT32 firstlump;
	INT32 numlumps;
} lumplist_t;

#if defined(_MSC_VER)
#pragma pack(1)
#endif

// a raw entry of the wad directory
typedef struct
{
	UINT32 filepos; // file offset of the resource
	UINT32 size; // size of the resource
	char name[8]; // name of the resource
} ATTRPACK filelump_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

typedef struct
{
	const char *name;
	size_t len;
} lumpchecklist_t;

//===========================================================================
//                                                                    GLOBALS
//===========================================================================
UINT16 numwadfiles; // number of active wadfiles
wadfile_t *wadfiles[MAX_WADFILES]; // 0 to numwadfiles-1 are valid

// W_Shutdown
// Closes all of the WAD files before quitting
// If not done on a Mac then open wad files
// can prevent removable media they are on from
// being ejected
void W_Shutdown(void)
{
	while (numwadfiles--)
		fclose(wadfiles[numwadfiles]->handle);
}

//===========================================================================
//                                                        LUMP BASED ROUTINES
//===========================================================================

// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.

static char filenamebuf[MAX_WADPATH];

// search for all DEHACKED lump in all wads and load it
static inline void W_LoadDehackedLumps(UINT16 wadnum)
{
	UINT16 lump;

	// Check for MAINCFG
	for (lump = 0;lump != INT16_MAX;lump++)
	{
		lump = W_CheckNumForNamePwad("MAINCFG", wadnum, lump);
		if (lump == INT16_MAX)
			break;
		CONS_Printf("Loading main config from %s\n", wadfiles[wadnum]->filename);
		DEH_LoadDehackedLumpPwad(wadnum, lump);
	}

	// Check for OBJCTCFG
	for (lump = 0;lump < INT16_MAX;lump++)
	{
		lump = W_CheckNumForNamePwad("OBJCTCFG", wadnum, lump);
		if (lump == INT16_MAX)
			break;
		CONS_Printf("Loading object config from %s\n", wadfiles[wadnum]->filename);
		DEH_LoadDehackedLumpPwad(wadnum, lump);
	}
}

/** Compute MD5 message digest for bytes read from STREAM of this filname.
  *
  * The resulting message digest number will be written into the 16 bytes
  * beginning at RESBLOCK.
  *
  * \param filename path of file
  * \param resblock resulting MD5 checksum
  * \return 0 if MD5 checksum was made, and is at resblock, 1 if error was found
  */
static inline INT32 W_MakeFileMD5(const char *filename, void *resblock)
{
#ifdef NOMD5
	(void)filename;
	memset(resblock, 0x00, 16);
#else
	FILE *fhandle;

	if ((fhandle = fopen(filename, "rb")) != NULL)
	{
		tic_t t = I_GetTime();
#ifndef _arch_dreamcast
		if (devparm)
#endif
		CONS_Printf("Making MD5 for %s\n",filename);
		if (md5_stream(fhandle, resblock) == 1)
			return 1;
#ifndef _arch_dreamcast
		if (devparm)
#endif
		CONS_Printf("MD5 calc for %s took %f seconds\n",
			filename, (float)(I_GetTime() - t)/TICRATE);
		fclose(fhandle);
		return 0;
	}
#endif
	return 1;
}

//  Allocate a wadfile, setup the lumpinfo (directory) and
//  lumpcache, add the wadfile to the current active wadfiles
//
//  now returns index into wadfiles[], you can get wadfile_t *
//  with:
//       wadfiles[<return value>]
//
//  return -1 in case of problem
//
// Can now load dehacked files (.soc)
//
UINT16 W_LoadWadFile(const char *filename)
{
	FILE *handle;
	lumpinfo_t *lumpinfo;
	wadfile_t *wadfile;
	UINT32 numlumps;
	size_t i;
#ifdef HWRENDER
	GLPatch_t *grPatch;
#endif
	INT32 compressed = 0;
	size_t packetsize = 0;
	serverinfo_pak *dummycheck = NULL;

	// Shut the compiler up.
	(void)dummycheck;

	//CONS_Printf("Loading %s\n", filename);
	//
	// check if limit of active wadfiles
	//
	if (numwadfiles >= MAX_WADFILES)
	{
		CONS_Printf("Maximum wad files reached\n");
		return INT16_MAX;
	}

	strncpy(filenamebuf, filename, MAX_WADPATH);
	filenamebuf[MAX_WADPATH - 1] = '\0';
	filename = filenamebuf;

	// open wad file
	if ((handle = fopen(filename, "rb")) == NULL)
	{
		// If we failed to load the file with the path as specified by
		// the user, strip the directories and search for the file.
		nameonly(filenamebuf);

		// If findfile finds the file, the full path will be returned
		// in filenamebuf == filename.
		if (findfile(filenamebuf, NULL, true))
		{
			if ((handle = fopen(filename, "rb")) == NULL)
			{
				CONS_Printf("Can't open %s\n", filename);
				return INT16_MAX;
			}
		}
		else
		{
			CONS_Printf("File %s not found.\n", filename);
			return INT16_MAX;
		}
	}

	// Check if wad files will overflow fileneededbuffer. Only the filename part
	// is send in the packet; cf.
	for (i = 0; i < numwadfiles; i++)
	{
		packetsize += nameonlylength(wadfiles[i]->filename);
		packetsize += 22; // MD5, etc.
	}

	packetsize += nameonlylength(filename);
	packetsize += 22;

	if (packetsize > sizeof(dummycheck->fileneeded))
	{
		CONS_Printf("Maximum wad files reached\n");
		if (handle)
			fclose(handle);
		return INT16_MAX;
	}

	// detect dehacked file with the "soc" extension
	if (!stricmp(&filename[strlen(filename) - 4], ".soc"))
	{
		// This code emulates a wadfile with one lump name "OBJCTCFG"
		// at position 0 and size of the whole file.
		// This allows soc files to be like all wads, copied by network and loaded at the console.
		numlumps = 1;
		lumpinfo = Z_Calloc(sizeof (*lumpinfo), PU_STATIC, NULL);
		lumpinfo->position = 0;
		fseek(handle, 0, SEEK_END);
		lumpinfo->size = ftell(handle);
		fseek(handle, 0, SEEK_SET);
		strcpy(lumpinfo->name, "OBJCTCFG");
	}
	else
	{
		// assume wad file
		wadinfo_t header;
		lumpinfo_t *lump_p;
		filelump_t *fileinfo;
		void *fileinfov;

		// read the header
		if (fread(&header, 1, sizeof header, handle) < sizeof header)
		{
			CONS_Printf("Can't read wad header from %s because %s\n", filename, strerror(ferror(handle)));
			return INT16_MAX;
		}

		if (memcmp(header.identification, "ZWAD", 4) == 0)
			compressed = 1;
		else if (memcmp(header.identification, "IWAD", 4) != 0
			&& memcmp(header.identification, "PWAD", 4) != 0
			&& memcmp(header.identification, "SDLL", 4) != 0)
		{
			CONS_Printf("%s doesn't have IWAD or PWAD id\n", filename);
			return INT16_MAX;
		}

		header.numlumps = LONG(header.numlumps);
		header.infotableofs = LONG(header.infotableofs);

		// read wad file directory
		i = header.numlumps * sizeof (*fileinfo);
		fileinfov = fileinfo = malloc(i);
		if (fseek(handle, header.infotableofs, SEEK_SET) == -1
			|| fread(fileinfo, 1, i, handle) < i)
		{
			CONS_Printf("%s wadfile directory is corrupt; maybe %s\n", filename, strerror(ferror(handle)));
			free(fileinfov);
			return INT16_MAX;
		}

		numlumps = header.numlumps;

		// fill in lumpinfo for this wad
		lump_p = lumpinfo = Z_Malloc(numlumps * sizeof (*lumpinfo), PU_STATIC, NULL);
		for (i = 0; i < numlumps; i++, lump_p++, fileinfo++)
		{
			lump_p->position = LONG(fileinfo->filepos);
			lump_p->size = lump_p->disksize = LONG(fileinfo->size);
			if (compressed) // wad is compressed, lump might be
			{
				UINT32 realsize = 0;

				if (fseek(handle, lump_p->position, SEEK_SET)
					== -1 || fread(&realsize, 1, sizeof realsize,
					handle) < sizeof realsize)
				{
					I_Error("corrupt compressed file: %s; maybe %s",
						filename, strerror(ferror(handle)));
				}
				realsize = LONG(realsize);
				if (realsize != 0)
				{
					lump_p->size = realsize;
					lump_p->compressed = 1;
				}
				else
				{
					lump_p->size -= 4;
					lump_p->compressed = 0;
				}

				lump_p->position += 4;
				lump_p->disksize -= 4;
			}
			else lump_p->compressed = 0;
			strncpy(lump_p->name, fileinfo->name, 8);
		}
		free(fileinfov);
	}
	//
	// link wad file to search files
	//
	wadfile = Z_Malloc(sizeof (*wadfile), PU_STATIC, NULL);
	wadfile->filename = Z_StrDup(filename);
	wadfile->handle = handle;
	wadfile->numlumps = (UINT16)numlumps;
	wadfile->lumpinfo = lumpinfo;
	fseek(handle, 0, SEEK_END);
	wadfile->filesize = (unsigned)ftell(handle);

	//
	// generate md5sum
	//
	W_MakeFileMD5(filename, wadfile->md5sum);

	//
	// set up caching
	//
	Z_Calloc(numlumps * sizeof (*wadfile->lumpcache), PU_STATIC, &wadfile->lumpcache);

#ifdef HWRENDER
	// allocates GLPatch info structures STATIC from the start,
	// because these were causing a lot of fragmentation of the heap,
	// considering they are never freed.
	grPatch = Z_Calloc(numlumps * sizeof (*grPatch), PU_HWRPATCHINFO, &(wadfile->hwrcache)); // never freed
	for (i = 0; i < numlumps; i++)
	{
		// store the software patch lump number for each GLPatch
		grPatch[i].patchlump = (numwadfiles<<16) + (UINT16)i;
	}
#endif

	//
	// add the wadfile
	//
	CONS_Printf("Added file %s (%u lumps)\n", filename, numlumps);
	wadfiles[numwadfiles] = wadfile;
	W_LoadDehackedLumps(numwadfiles);

	numwadfiles++;
	return wadfile->numlumps;
}

void W_UnloadWadFile(UINT16 num)
{
	INT32 i;
	wadfile_t *delwad = wadfiles[num];
	lumpcache_t *lumpcache;
	if (num == 0)
	{
		CONS_Printf("You can't remove the IWAD %s!\n", wadfiles[0]->filename);
		return;
	}
	else
		CONS_Printf("Removing WAD %s...\n", wadfiles[num]->filename);

	DEH_UnloadDehackedWad(num);
	wadfiles[num] = NULL;
	lumpcache = delwad->lumpcache;
	numwadfiles--;
#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
		HWR_FreeTextureCache();
	Z_Free(delwad->hwrcache);
#endif
	if (*lumpcache)
	{
		for (i = 0;i < delwad->numlumps;i++)
			Z_ChangeTag(lumpcache[i], PU_PURGELEVEL);
	}
	Z_Free(lumpcache);
	fclose(delwad->handle);
	Z_Free(delwad->filename);
	Z_Free(delwad);
	CONS_Printf(" done unloading WAD\n");
}

/** Tries to load a series of files.
  * All files are wads unless they have an extension of ".soc".
  *
  * Each file is optional, but at least one file must be found or an error will
  * result. Lump names can appear multiple times. The name searcher looks
  * backwards, so a later file overrides all earlier ones.
  *
  * \param filenames A null-terminated list of files to use.
  * \return 1 if all files were loaded, 0 if at least one was missing or
  *           invalid.
  */
INT32 W_InitMultipleFiles(char **filenames)
{
	INT32 rc = 1;

	// open all the files, load headers, and count lumps
	numwadfiles = 0;

	// will be realloced as lumps are added
	for (; *filenames; filenames++)
	{
		//CONS_Printf("Loading %s\n", *filenames);
		rc &= (W_LoadWadFile(*filenames) != INT16_MAX) ? 1 : 0;
	}

	if (!numwadfiles)
		I_Error("W_InitMultipleFiles: no files found");

	return rc;
}

/** Make sure a lump number is valid.
  * Compiles away to nothing if PARANOIA is not defined.
  */
static boolean TestValidLump(UINT16 wad, UINT16 lump)
{
	I_Assert(wad < MAX_WADFILES);
	if (!wadfiles[wad]) // make sure the wad file exists
		return false;

	I_Assert(lump < wadfiles[wad]->numlumps);
	if (lump >= wadfiles[wad]->numlumps) // make sure the lump exists
		return false;

	return true;
}


const char *W_CheckNameForNumPwad(UINT16 wad, UINT16 lump)
{
	if (lump >= wadfiles[wad]->numlumps || !TestValidLump(wad, 0))
		return NULL;

	return wadfiles[wad]->lumpinfo[lump].name;
}

const char *W_CheckNameForNum(lumpnum_t lumpnum)
{
	return W_CheckNameForNumPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum));
}

//
// Same as the original, but checks in one pwad only.
// wadid is a wad number
// (Used for sprites loading)
//
// 'startlump' is the lump number to start the search
//
UINT16 W_CheckNumForNamePwad(const char *name, UINT16 wad, UINT16 startlump)
{
	UINT16 i;
	static char uname[9];

	memset(uname, 0x00, sizeof uname);
	strncpy(uname, name, 8);
	uname[8] = 0;
	strupr(uname);

	if (!TestValidLump(wad,0))
		return INT16_MAX;

	//
	// scan forward
	// start at 'startlump', useful parameter when there are multiple
	//                       resources with the same name
	//
	if (startlump < wadfiles[wad]->numlumps)
	{
		lumpinfo_t *lump_p = wadfiles[wad]->lumpinfo + startlump;
		for (i = startlump; i < wadfiles[wad]->numlumps; i++, lump_p++)
		{
			if (memcmp(lump_p->name,uname,8) == 0)
				return i;
		}
	}

	// not found.
	return INT16_MAX;
}

//
// W_CheckNumForName
// Returns LUMPERROR if name not found.
//
lumpnum_t W_CheckNumForName(const char *name)
{
	INT32 i;
	lumpnum_t check = INT16_MAX;

	// scan wad files backwards so patch lump files take precedence
	for (i = numwadfiles - 1; i >= 0; i--)
	{
		check = W_CheckNumForNamePwad(name,(UINT16)i,0);
		if (check != INT16_MAX)
			break; //found it
	}
	if (check == INT16_MAX) return LUMPERROR;
	else return (i<<16)+check;
}

//
// W_GetNumForName
//
// Calls W_CheckNumForName, but bombs out if not found.
//
lumpnum_t W_GetNumForName(const char *name)
{
	lumpnum_t i;

	i = W_CheckNumForName(name);

	if (i == LUMPERROR)
		I_Error("W_GetNumForName: %s not found!\n", name);

	return i;
}

size_t W_LumpLengthPwad(UINT16 wad, UINT16 lump)
{
	if (!TestValidLump(wad, lump))
		return 0;
	return wadfiles[wad]->lumpinfo[lump].size;
}

/** Returns the buffer size needed to load the given lump.
  *
  * \param lump Lump number to look at.
  * \return Buffer size needed, in bytes.
  */
size_t W_LumpLength(lumpnum_t lumpnum)
{
	return W_LumpLengthPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum));
}

/** Reads bytes from the head of a lump, without doing decompression.
  *
  * \param wad Wad number to read from.
  * \param lump Lump number to read from, within wad.
  * \param dest Buffer in memory to serve as destination.
  * \param size Number of bytes to read.
  * \param offest Number of bytes to offset.
  * \return Number of bytes read (should equal size).
  * \sa W_ReadLumpHeader
  */
static size_t W_RawReadLumpHeader(UINT16 wad, UINT16 lump, void *dest, size_t size, size_t offset)
{
	size_t bytesread;
	lumpinfo_t *l;
	FILE *handle;

	l = wadfiles[wad]->lumpinfo + lump;

	handle = wadfiles[wad]->handle;

	fseek(handle, (long)(l->position + offset), SEEK_SET);
	bytesread = fread(dest, 1, size, handle);

	return bytesread;
}

// Read a compressed lump; return it in newly Z_Malloc'd memory.
// wad is number of wad file, lump is number of lump in wad.
static void *W_ReadCompressedLump(UINT16 wad, UINT16 lump)
{
#ifdef ZWAD
	char *compressed, *data;
	const lumpinfo_t *l = &wadfiles[wad]->lumpinfo[lump];
	size_t retval;

	compressed = Z_Malloc(l->disksize, PU_STATIC, NULL);
	data = Z_Malloc(l->size, PU_STATIC, NULL);
	if (W_RawReadLumpHeader(wad, lump, compressed, l->disksize, 0)
		< l->disksize)
	{
		I_Error("wad %d, lump %d: cannot read compressed data",
			wad, lump);
	}

	retval = lzf_decompress(compressed, l->disksize, data, l->size);
#ifndef AVOID_ERRNO
	if (retval == 0 && errno == E2BIG)
	{
		I_Error("wad %d, lump %d: compressed data too big "
			"(bigger than %"PRIdS")", wad, lump, l->size);
	}
	else if (retval == 0 && errno == EINVAL)
		I_Error("wad %d, lump %d: invalid compressed data", wad, lump);
	else
#endif
	if (retval != l->size)
	{
		I_Error("wad %d, lump %d: decompressed to wrong number of "
			"bytes (expected %"PRIdS", got %"PRIdS")", wad, lump,
			l->size, retval);
	}
	Z_Free(compressed);
	return data;
#else
	(void)wad;
	(void)lump;
	//I_Error("ZWAD files not supported on this platform.");
	return NULL;
#endif
}

/** Reads bytes from the head of a lump.
  * Note: If the lump is compressed, the whole thing has to be read anyway.
  *
  * \param lump Lump number to read from.
  * \param dest Buffer in memory to serve as destination.
  * \param size Number of bytes to read.
  * \param offest Number of bytes to offset.
  * \return Number of bytes read (should equal size).
  * \sa W_ReadLump, W_RawReadLumpHeader
  */
size_t W_ReadLumpHeaderPwad(UINT16 wad, UINT16 lump, void *dest, size_t size, size_t offset)
{
	size_t lumpsize;

	if (!TestValidLump(wad,lump))
		return 0;

	lumpsize = wadfiles[wad]->lumpinfo[lump].size;
	// empty resource (usually markers like S_START, F_END ..)
	if (!lumpsize || lumpsize<offset)
		return 0;

	// zero size means read all the lump
	if (!size || size+offset > lumpsize)
		size = lumpsize - offset;

	if (wadfiles[wad]->lumpinfo[lump].compressed)
	{
		UINT8 *data;
		data = W_ReadCompressedLump(wad, lump);
		if (!data) return 0;
		M_Memcpy(dest, data+offset, size);
		Z_Free(data);
		return size;
	}
	else
		return W_RawReadLumpHeader(wad, lump, dest, size, offset);
}

size_t W_ReadLumpHeader(lumpnum_t lumpnum, void *dest, size_t size, size_t offset)
{
	return W_ReadLumpHeaderPwad(WADFILENUM(lumpnum), LUMPNUM(lumpnum), dest, size, offset);
}

/** Reads a lump into memory.
  *
  * \param lump Lump number to read from.
  * \param dest Buffer in memory to serve as destination. Size must be >=
  *             W_LumpLength().
  * \sa W_ReadLumpHeader
  */
void W_ReadLump(lumpnum_t lumpnum, void *dest)
{
	W_ReadLumpHeaderPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum),dest,0,0);
}

void W_ReadLumpPwad(UINT16 wad, UINT16 lump, void *dest)
{
	W_ReadLumpHeaderPwad(wad, lump, dest, 0, 0);
}

// ==========================================================================
// W_CacheLumpNum
// ==========================================================================
void *W_CacheLumpNumPwad(UINT16 wad, UINT16 lump, INT32 tag)
{
	lumpcache_t *lumpcache;

	if (!TestValidLump(wad,lump))
		return NULL;

	lumpcache = wadfiles[wad]->lumpcache;
	if (!lumpcache[lump])
	{
		void *ptr = Z_Malloc(W_LumpLengthPwad(wad, lump), tag, &lumpcache[lump]);
		W_ReadLumpHeaderPwad(wad, lump, ptr, 0, 0);  // read the lump in full
	}
	else
		Z_ChangeTag(lumpcache[lump], tag);

	return lumpcache[lump];
}

void *W_CacheLumpNum(lumpnum_t lumpnum, INT32 tag)
{

	return W_CacheLumpNumPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum),tag);
}

//
// W_CacheLumpNumForce
//
// Forces the lump to be loaded, even if it already is!
//
void *W_CacheLumpNumForce(lumpnum_t lumpnum, INT32 tag)
{
	UINT16 wad, lump;
	void *ptr;

	wad = WADFILENUM(lumpnum);
	lump = LUMPNUM(lumpnum);

	if (!TestValidLump(wad,lump))
		return NULL;

	ptr = Z_Malloc(W_LumpLengthPwad(wad, lump), tag, NULL);
	W_ReadLumpHeaderPwad(wad, lump, ptr, 0, 0);  // read the lump in full

	return ptr;
}

//
// W_IsLumpCached
//
// If a lump is already cached return true, otherwise
// return false.
//
// no outside code uses the PWAD form, for now
static inline boolean W_IsLumpCachedPWAD(UINT16 wad, UINT16 lump, void *ptr)
{
	void *lcache;

	if (!TestValidLump(wad, lump))
		return false;

	lcache = wadfiles[wad]->lumpcache[lump];

	if (ptr)
	{
		if (ptr == lcache)
			return true;
	}
	else if (lcache)
		return true;

	return false;
}

boolean W_IsLumpCached(lumpnum_t lumpnum, void *ptr)
{
	return W_IsLumpCachedPWAD(WADFILENUM(lumpnum),LUMPNUM(lumpnum), ptr);
}

// ==========================================================================
// W_CacheLumpName
// ==========================================================================
void *W_CacheLumpName(const char *name, INT32 tag)
{
	return W_CacheLumpNum(W_GetNumForName(name), tag);
}

// ==========================================================================
//                                         CACHING OF GRAPHIC PATCH RESOURCES
// ==========================================================================

// Graphic 'patches' are loaded, and if necessary, converted into the format
// the most useful for the current rendermode. For software renderer, the
// graphic patches are kept as is. For the hardware renderer, graphic patches
// are 'unpacked', and are kept into the cache in that unpacked format, and
// the heap memory cache then acts as a 'level 2' cache just after the
// graphics card memory.

//
// Cache a patch into heap memory, convert the patch format as necessary
//

// Software-only compile cache the data without conversion
#ifdef HWRENDER
static inline void *W_CachePatchNumPwad(UINT16 wad, UINT16 lump, INT32 tag)
{
	GLPatch_t *grPatch;

	if (rendermode == render_soft || rendermode == render_none)
		return W_CacheLumpNumPwad(wad, lump, tag);

	if (!TestValidLump(wad, lump))
		return NULL;

	grPatch = &(wadfiles[wad]->hwrcache[lump]);

	if (grPatch->mipmap.grInfo.data)
	{
		if (tag == PU_CACHE)
			tag = PU_HWRCACHE;
		Z_ChangeTag(grPatch->mipmap.grInfo.data, tag);
	}
	else
	{
		// first time init grPatch fields
		// we need patch w,h,offset,...
		// this code will be executed latter in GetPatch, anyway
		// do it now
		patch_t *ptr = W_CacheLumpNum(grPatch->patchlump, PU_STATIC);
		HWR_MakePatch(ptr, grPatch, &grPatch->mipmap);
		Z_Free(ptr);
	}

	// return GLPatch_t, which can be casted to (patch_t) with valid patch header info
	return (void *)grPatch;
}

void *W_CachePatchNum(lumpnum_t lumpnum, INT32 tag)
{
	return W_CachePatchNumPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum),tag);
}

#endif // HWRENDER

void *W_CachePatchName(const char *name, INT32 tag)
{
	lumpnum_t num;

	num = W_CheckNumForName(name);

	if (num == LUMPERROR)
		return W_CachePatchNum(W_GetNumForName("BRDR_MM"), tag);
	return W_CachePatchNum(num, tag);
}
#ifndef NOMD5
#define MD5_LEN 16

/**
  * Prints an MD5 string into a human-readable textual format.
  *
  * \param md5 The md5 in binary form -- MD5_LEN (16) bytes.
  * \param buf Where to print the textual form. Needs 2*MD5_LEN+1 (33) bytes.
  * \author Graue <graue@oceanbase.org>
  */
#define MD5_FORMAT \
	"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
static void PrintMD5String(const UINT8 *md5, char *buf)
{
	snprintf(buf, 2*MD5_LEN+1, MD5_FORMAT,
		md5[0], md5[1], md5[2], md5[3],
		md5[4], md5[5], md5[6], md5[7],
		md5[8], md5[9], md5[10], md5[11],
		md5[12], md5[13], md5[14], md5[15]);
}
#endif
/** Verifies a file's MD5 is as it should be.
  * For releases, used as cheat prevention -- if the MD5 doesn't match, a
  * fatal error is thrown. In debug mode, an MD5 mismatch only triggers a
  * warning.
  *
  * \param wadfilenum Number of the loaded wad file to check.
  * \param matchmd5   The MD5 sum this wad should have, expressed as a
  *                   textual string.
  * \author Graue <graue@oceanbase.org>
  */
void W_VerifyFileMD5(UINT16 wadfilenum, const char *matchmd5)
{
#ifdef NOMD5
	(void)wadfilenum;
	(void)matchmd5;
#else
	UINT8 realmd5[MD5_LEN];
	INT32 ix;

	I_Assert(strlen(matchmd5) == 2*MD5_LEN);
	I_Assert(wadfilenum < numwadfiles);
	// Convert an md5 string like "7d355827fa8f981482246d6c95f9bd48"
	// into a real md5.
	for (ix = 0; ix < 2*MD5_LEN; ix++)
	{
		INT32 n, c = matchmd5[ix];
		if (isdigit(c))
			n = c - '0';
		else
		{
			I_Assert(isxdigit(c));
			if (isupper(c)) n = c - 'A' + 10;
			else n = c - 'a' + 10;
		}
		if (ix & 1) realmd5[ix>>1] = (UINT8)(realmd5[ix>>1]+n);
		else realmd5[ix>>1] = (UINT8)(n<<4);
	}

	if (memcmp(realmd5, wadfiles[wadfilenum]->md5sum, 16))
	{
		char actualmd5text[2*MD5_LEN+1];
		PrintMD5String(wadfiles[wadfilenum]->md5sum, actualmd5text);
#ifdef _DEBUG
		CONS_Printf
#else
		I_Error
#endif
			("File is corrupt or has been modified: %s "
			"(found md5: %s, wanted: %s)\n",
			wadfiles[wadfilenum]->filename,
			actualmd5text, matchmd5);
	}
#endif
}

// Note: This never opens lumps themselves and therefore doesn't have to
// deal with compressed lumps.
static int W_VerifyFile(const char *filename, lumpchecklist_t *checklist,
	boolean status)
{
	FILE *handle;
	size_t i, j;
	int goodfile = false;

	if (!checklist) I_Error("No checklist for %s\n", filename);
	strlcpy(filenamebuf, filename, MAX_WADPATH);
	filename = filenamebuf;
	// open wad file
	if ((handle = fopen(filename, "rb")) == NULL)
	{
		nameonly(filenamebuf); // leave full path here
		if (findfile(filenamebuf, NULL, true))
		{
			if ((handle = fopen(filename, "rb")) == NULL)
				return -1;
		}
		else
			return -1;
	}

	// detect dehacked file with the "soc" extension
	if (stricmp(&filename[strlen(filename) - 4], ".soc") != 0)
	{
		// assume wad file
		wadinfo_t header;
		filelump_t lumpinfo;

		// read the header
		if (fread(&header, 1, sizeof header, handle) == sizeof header
			&& header.numlumps < INT16_MAX
			&& strncmp(header.identification, "ZWAD", 4)
			&& strncmp(header.identification, "IWAD", 4)
			&& strncmp(header.identification, "PWAD", 4)
			&& strncmp(header.identification, "SDLL", 4))
		{
			fclose(handle);
			return true;
		}

		header.numlumps = LONG(header.numlumps);
		header.infotableofs = LONG(header.infotableofs);

		// let seek to the lumpinfo list
		if (fseek(handle, header.infotableofs, SEEK_SET) == -1)
		{
			fclose(handle);
			return false;
		}

		goodfile = true;
		for (i = 0; i < header.numlumps; i++)
		{
			// fill in lumpinfo for this wad file directory
			if (fread(&lumpinfo, sizeof (lumpinfo), 1 , handle) != 1)
			{
				fclose(handle);
				return -1;
			}

			lumpinfo.filepos = LONG(lumpinfo.filepos);
			lumpinfo.size = LONG(lumpinfo.size);

			if (lumpinfo.size == 0)
				continue;

			for (j = 0; j < NUMSPRITES; j++)
				if (sprnames[j] && !strncmp(lumpinfo.name, sprnames[j], 4)) // Sprites
					continue;

			goodfile = false;
			for (j = 0; checklist[j].len && checklist[j].name && !goodfile; j++)
				if ((strncmp(lumpinfo.name, checklist[j].name, checklist[j].len) != false) == status)
					goodfile = true;

			if (!goodfile)
				break;
		}
	}
	fclose(handle);
	return goodfile;
}


/** Checks a wad for lumps other than music and sound.
  * Used during game load to verify music.dta is a good file and during a
  * netgame join (on the server side) to see if a wad is important enough to
  * be sent.
  *
  * \param filename Filename of the wad to check.
  * \return 1 if file contains only music/sound lumps, 0 if it contains other
  *         stuff (maps, sprites, dehacked lumps, and so on). -1 if there no
  *         file exists with that filename
  * \author Alam Arias
  */
int W_VerifyNMUSlumps(const char *filename)
{
	// MIDI, MOD/S3M/IT/XM/OGG/MP3/WAV, WAVE SFX
	// ENDOOM text and palette lumps
	lumpchecklist_t NMUSlist[] =
	{
		{"D_", 2},
		{"O_", 2},
		{"DS", 2},
		{"ENDOOM", 6},
		{"PLAYPAL", 7},
		{"COLORMAP", 8},
		{"PAL", 3},
		{"CLM", 3},
		{NULL, 0},
	};
	return W_VerifyFile(filename, NMUSlist, false);
}
