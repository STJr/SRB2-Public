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
/// \brief  Zone Memory Allocation. Neat

#include "doomdef.h"
#include "z_zone.h"
#include "i_system.h"
#include "command.h"
#include "m_argv.h"
#include "i_video.h"
#include "doomstat.h"
#ifdef HWRENDER
#include "hardware/hw_main.h" // for hardware memory stats
#endif
#include "m_misc.h"

// =========================================================================
//                        ZONE MEMORY ALLOCATION
// =========================================================================
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

#define ZONEID 0x1d4a11

typedef struct memblock_s
{
	size_t size; // including the header and possibly tiny fragments
	void **user; // NULL if a free block
	int tag;     // purgelevel
	int id;      // should be ZONEID
#ifdef ZDEBUG
	const char *ownerfile;
	int ownerline;
#endif
	struct memblock_s *next, *prev;
} ATTRPACK memblock_t;

typedef struct
{
	// total bytes malloced, including header
	size_t size;

	// start/end cap for linked list
	memblock_t blocklist;

	memblock_t *rover;
} ATTRPACK memzone_t;

static memzone_t *mainzone;

static void Command_Memfree_f(void);

#if defined (_WIN32_WCE) || defined (DC)
static byte mb_used = 6; // sounds need 2.5 MB!
#elif defined (PSP)
static byte mb_used = 16;
#elif defined (HWRENDER)
static byte mb_used = 64;
#else
static byte mb_used = 32;
#endif

//
// Z_Init
//
void Z_Init(void)
{
	memblock_t *block;
	size_t size;

	if (M_CheckParm("-mb") && M_IsNextParm())
		mb_used = (byte)atoi(M_GetNextParm());
	else
	{
		ULONG total;
		ULONG memfree = I_GetFreeMem(&total)>>20;
		CONS_Printf("system memory %luMB free %luMB\n", total>>20, memfree);
		// we assume the system uses a lot of memory for disk cache
		if (memfree < 6)
			memfree = total>>21;
#ifdef _WIN32_WCE
		mb_used = (byte)min(max(memfree, mb_used), 10); // min 6MB max 10MB
#elif defined (DC)
//		if (!nofmod)
//			mb_used = (byte)min(max(memfree, mb_used), 8); // min 5MB max 8MB
#elif defined (SDL)
		if (!nosound)
			mb_used = (byte)min(max(memfree, mb_used), 48); // min 32MB max 48MB, for all the static sfxs
#endif
		// If not WinCE, and either SDL with no sfx, or non-SDL, then use 32 MB always.
		// Default is set above, no need to do anything here.
	}
	CONS_Printf("%d megabytes requested for Z_Init.\n", mb_used);
	size = mb_used<<20;
	mainzone = (memzone_t *)malloc(size);
	if (!mainzone)
		I_Error("Could not allocate %d megabytes.\n"
			"Please use the -mb parameter and specify a lower value.\n", mb_used);

	// touch memory to stop swapping
	memset(mainzone, 0, size);

	mainzone->size = size;

	// set the entire zone to one free block
	// block is the only free block in the zone
	mainzone->blocklist.next = mainzone->blocklist.prev = block =
		(memblock_t *)((byte *)mainzone + sizeof (memzone_t));

	mainzone->blocklist.user = (void *)mainzone;
	mainzone->blocklist.tag = PU_STATIC;
	mainzone->rover = block;

	block->prev = block->next = &mainzone->blocklist;

	// NULL indicates a free block.
	block->user = NULL;

	block->size = mainzone->size - sizeof (memzone_t);

	COM_AddCommand("memfree", Command_Memfree_f);
}

//
// Z_Free
//
#ifdef ZDEBUG
void Z_Free2(void *ptr, const char *file, int line)
#else
void Z_Free(void *ptr)
#endif
{
	memblock_t *block;
	memblock_t *other;

#ifdef ZDEBUG2
	CONS_Printf("Z_Free Current file: %s\n", file);
	CONS_Printf("Z_Free Current line: %d\n", line);
#endif

	if (!ptr || !mainzone)
		return;

	block = (memblock_t *)((byte *)ptr - sizeof (memblock_t));

#ifdef ZDEBUG
	// Write all Z_Free's to a debug file
	if (debugfile)
		fprintf(debugfile, "ZFREE@File: %s, line: %d\n", file, line);
	// check if there is not a user in this zone
	for (other = mainzone->blocklist.next; other->next != &mainzone->blocklist;
		other = other->next)
	{
		if ((other != block) && (other->user > (void **)0x100)
			&& ((other->user) >= (void **)block) && ((other->user) <= (void **)((byte *)block)
			+ block->size))
		{
			I_Error("Z_Free: Pointer %s:%d in zone at %s:%d", other->ownerfile, other->ownerline,
				file, line);
		}
	}
#endif

	if (block->id != ZONEID)
		I_Error("Internal memory management error");

//#ifdef PARANOIA
	// get direct a segv when using a pointer that isn't right
	memset(ptr, 0, block->size - sizeof (memblock_t));
//#endif
	if (block->user > (void **)0x100)
	{
		// smaller values are not pointers
		// Note: OS-dependent?

		// clear the user's mark
		*block->user = 0;
	}

	// mark as free
	block->user = NULL;
	block->tag = block->id = 0;

	other = block->prev;

	if (!other->user)
	{
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;

		if (block == mainzone->rover)
			mainzone->rover = other;

		block = other;
	}

	other = block->next;
	if (!other->user)
	{
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;

		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}

//
// Z_Malloc
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
#define MINFRAGMENT sizeof (memblock_t)

#ifdef ZDEBUG
void *Z_Malloc2(size_t size, int tag, void *user, int alignbits, const char *file, int line)
#else
void *Z_MallocAlign(size_t size, int tag, void *user, int alignbits)
#endif
{
	ULONG alignmask = (1<<alignbits) - 1;
#define ALIGN(a) (((size_t)a + alignmask) & ~alignmask)
	size_t extra, basedata;
	memblock_t *start;
	memblock_t *rover;
	memblock_t *newblock;
	memblock_t *base;

	if (!mainzone)
		return NULL;

#ifdef ZDEBUG2
	CONS_Printf("Z_Malloc2 Current file: %s\n", file);
	CONS_Printf("Z_Malloc2 Current line: %d\n", line);
#endif

	// scan through the block list, looking for the first free block of sufficient size,
	// throwing out any purgable blocks along the way.

	size = (size + 3) & ~3; // round up to 4's BEFORE accounting for block header

	// account for size of block header
	size += sizeof (memblock_t);

	// if there is a free block behind the rover, back up over them.
	// added comment: base is used to point at the begin of a region in case
	//                when there is two (or more) adjacent purgable block
	base = mainzone->rover;

	if (!base->prev->user)
		base = base->prev;

	rover = base;
	start = base->prev;

	if (base->id && base->id != ZONEID) // this shouldn't happen
		I_Error("WARNING: SRB2 may crash in a short time. This is a known bug, sorry.\n");

	do
	{
		if (rover == start)
		{
			// scanned all the way around the list
			// debug to see if problems of memory fragmentation..
			Command_Memfree_f();

			I_Error("Failed on allocation of %d bytes\n"
				"Try increasing memory size using the -mb parameter (currently using %d MB)",
				size, mb_used);
		}

		if (rover->user)
		{
			if (rover->tag < PU_PURGELEVEL)
			{
				// hit a block that can't be purged, so move base past it

				// FIXME: this is where the crashing problem seem to come from
				base = rover = rover->next;
			}
			else
			{
				// free the rover block (adding the size to base)

				// the rover can be the base block
				base = base->prev;
				Z_Free((byte *)rover + sizeof (memblock_t));
				base = base->next;
				rover = base->next;
			}
		}
		else
			rover = rover->next;
		basedata = ALIGN((size_t)base + sizeof (memblock_t));
	} while (base->user ||
		// crashed because base doesn't point to something valid (and it's not NULL)
		((size_t)base) + base->size < basedata + size - sizeof (memblock_t));

	// aligning can leave free space in current block so make it really free
	if (alignbits)
	{
		memblock_t *newbase = ((memblock_t *)basedata) - 1;
		size_t sizediff = (byte *)newbase - (byte *)base;

		if (sizediff > MINFRAGMENT)
		{
			newbase->prev = base;
			newbase->next = base->next;
			newbase->next->prev = newbase;

			newbase->size = base->size - sizediff;
			base->next = newbase;
			base->size = sizediff;
		}
		else
		{
			// adjust size of preview block if adjacent (not cycling)
			if (base->prev < base)
				base->prev->size += sizediff;
			base->prev->next = newbase;
			base->next->prev = newbase;
			base->size -= sizediff;
			memcpy(newbase, base, sizeof (memblock_t));
		}
		base = newbase;
	}

	// found a block big enough
	extra = base->size - size;

	if (extra > MINFRAGMENT)
	{
		// there will be a free fragment after the allocated block
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = extra;

		// NULL indicates free block.
		newblock->user = NULL;
		newblock->tag = 0;
		newblock->id = 0;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;

		base->next = newblock;
		base->size = size;
	}

	if (user)
	{
		// mark as an in use block
		base->user = user;
		*(void **)user = (void *)((byte *)base + sizeof (memblock_t));
	}
	else
	{
		// this shouldn't happen under normal circumstances
#if defined (PARANOIA) || defined (ZDEBUG)
		if (tag >= PU_PURGELEVEL)
			I_Error("Z_Malloc: attempted to allocate a purgable block with no owner");
#endif

		// mark as in use, but unowned
		base->user = (void *)2;
	}
	base->tag = tag;

#ifdef ZDEBUG
	base->ownerfile = file;
	base->ownerline = line;
#endif

	// next allocation will start looking here
	mainzone->rover = base->next;

	base->id = ZONEID;

	return (void *)((byte *)base + sizeof (memblock_t));
}

#ifdef ZDEBUG
void *Z_Calloc2(size_t size, int tag, void *user, int alignbits, const char *file, int line)
#else
void *Z_CallocAlign(size_t size, int tag, void *user, int alignbits)
#endif
{
#ifdef ZDEBUG
	return memset(Z_Malloc2    (size, tag, user, alignbits, file, line), 0, size);
#else
	return memset(Z_MallocAlign(size, tag, user, alignbits            ), 0, size);
#endif
}


#ifdef ZDEBUG
void *Z_Realloc2(void *ptr, size_t size, int tag, void *user, int alignbits, const char *file, int line)
#else
void *Z_ReallocAlign(void *ptr, size_t size,int tag, void *user,  int alignbits)
#endif
{
	void *rez;
	memblock_t *block;
	size_t copysize;

#ifdef ZDEBUG2
	CONS_Printf("Z_Realloc %s:%d\n", file, line);
#endif

	if (!size)
	{
		Z_Free(ptr);
		return NULL;
	}

	if (!ptr)
		return Z_CallocAlign(size, tag, user, alignbits);
	else
		rez = Z_MallocAlign(size, tag, user, alignbits);

	block = (memblock_t *)((byte *)ptr - sizeof *block);
	if (block->id != ZONEID)
	{
#ifdef ZDEBUG
		I_Error("Z_Realloc: wrong id from %s:%d", file, line);
#else
		I_Error("Z_Realloc: wrong id");
#endif
	}

#ifdef ZDEBUG
	// Write every Z_Realloc call to a debug file.
	DEBFILE(va("Z_Realloc at %s:%d\n", file, line));
#endif

	if (size < block->size)
		copysize = size;
	else
		copysize = block->size;

	M_Memcpy(rez, ptr, copysize);

	Z_Free(ptr);

	if (size > copysize)
		memset((char*)rez+copysize, 0x00, size-copysize);

	return rez;
}



//
// Z_FreeTags
//
void Z_FreeTags(int lowtag, int hightag)
{
	memblock_t *block;
	memblock_t *next;

	if (!mainzone)
		return;

	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = next)
	{
		// get link before freeing
		next = block->next;

		// free block?
		if (!block->user)
			continue;

		if (block->tag >= lowtag && block->tag <= hightag)
			Z_Free((byte *)block + sizeof (memblock_t));
	}
}

//
// Z_DumpHeap
// Note: TFileDumpHeap(stdout)?
//
static inline void Z_DumpHeap(int lowtag, int hightag)
{
	memblock_t *block;

	if (!mainzone)
	{
		CONS_Printf("There no ZMem Heap to dump\n");
		return;
	}

	CONS_Printf("zone size: %u  location: %p\n", (unsigned int)mainzone->size, mainzone);

	CONS_Printf("tag range: %d to %d\n", lowtag, hightag);

	for (block = mainzone->blocklist.next; ; block = block->next)
	{
		// give more info if ZDEBUG on
#ifndef ZDEBUG
		if (block->tag >= lowtag && block->tag <= hightag)
			CONS_Printf("block:%p    size:%u    user:%p    tag:%3d prev:%p next:%p\n",
				block, (unsigned int)block->size, block->user, block->tag, block->next, block->prev);
#else
		if (block->tag >= lowtag && block->tag <= hightag)
			CONS_Printf("block:%p    size:%u    user:%p    tag:%3d prev:%p next:%p id:%d "
				"ownerline:%6d ownerfile:%s\n", block, (unsigned int)block->size, block->user,
				block->tag,	block->next, block->prev, block->id, block->ownerline, block->ownerfile);
#endif

		if (block->next == &mainzone->blocklist)
			break; // all blocks have been hit

		if ((byte *)block + block->size != (byte *)block->next)
			CONS_Printf("ERROR: block size does not touch the next block\n");

		if (block->next->prev != block)
			CONS_Printf("ERROR: next block doesn't have proper back link\n");

		if (!block->user && !block->next->user)
			CONS_Printf("ERROR: two consecutive free blocks\n");
	}
}

//
// Z_FileDumpHeap
//
static inline void Z_FileDumpHeap(FILE *f)
{
	memblock_t *block;
	unsigned int totalblocks = 0;

	if (!mainzone)
	{
		fprintf(f, "There no ZMem Heap to dump to file\n");
		return;
	}

	fprintf(f, "zone size: %u     location: %p\n", (unsigned int)mainzone->size, mainzone);

	for (block = mainzone->blocklist.next; ; block = block->next)
	{
		totalblocks++;
		fprintf(f, "block:%p size:%u user:%p tag:%3d prev:%p next:%p id:%7d\n",
			block, (unsigned int)block->size, block->user, block->tag, block->prev, block->next,
			block->id);

		if (block->next == &mainzone->blocklist)
            break; // all blocks have been hit

		if ((block->user > (void **)0x100) && ((size_t)(*(block->user)) != ((size_t)block)
			+ sizeof (memblock_t)))
		{
			fprintf(f, "ERROR: block don't have a proper user\n");
		}

		if ((byte *)block + block->size != (byte *)block->next)
			fprintf(f, "ERROR: block size does not touch the next block\n");

		if (block->next->prev != block)
			fprintf(f, "ERROR: next block doesn't have proper back link\n");

		if (!block->user && !block->next->user)
			fprintf(f, "ERROR: two consecutive free blocks\n");
	}
	fprintf(f, "Total: %d blocks\n"
		"===============================================================================\n\n",totalblocks);
}

//
// Z_CheckHeap
//
void Z_CheckHeap(int i)
{
	memblock_t *block;
	unsigned int blocknumon = 0;

	if (!mainzone)
	{
		CONS_Printf("There no ZMem Heap to check\n");
		return;
	}

	for (block = mainzone->blocklist.next; ; block = block->next)
	{
		blocknumon++;
		if (block->next == &mainzone->blocklist)
			break; // all blocks have been hit

		if ((block->user > (void **)0x100) && ((size_t)(*(block->user)) != ((size_t)block)
			+ sizeof (memblock_t)))
		{
			I_Error("Z_CheckHeap: block %u doesn't have a proper user %d\n", blocknumon, i);
		}

		if ((byte *)block + block->size != (byte *)block->next)
			I_Error("Z_CheckHeap: block %u size does not touch the next block %d\n", blocknumon, i);

		if (block->next->prev != block)
			I_Error("Z_CheckHeap: next block from %u doesn't have proper back link %d\n"
				"next->tag is %d\nnext->size is %u\nblock->tag is %d\nblock->size is %u\n",
				blocknumon, i, block->next->tag, block->next->size, block->tag, block->size);

		if (!block->user && !block->next->user)
			I_Error("Z_CheckHeap: two consecutive free blocks (starting with %u) %d\n",
				blocknumon, i);
	}
}

//
// Z_ChangeTag
//
#ifdef PARANOIA
void Z_ChangeTag2(void *ptr, int tag, const char *file, int line)
#else
void Z_ChangeTag2(void *ptr, int tag)
#endif
{
	memblock_t *block;

#ifdef PARANOIA
	if (((memblock_t *)((byte *)ptr - sizeof (memblock_t)))->id != ZONEID)
		I_Error("Z_CT at %s:%d", file, line);
#endif

	if (!ptr)
		return;

	if (!mainzone)
		I_Error("Internal memory management error found while changing tag of nonething");

	block = (memblock_t *)((byte *)ptr - sizeof (memblock_t));

	if (block->id != ZONEID)
		I_Error("Internal memory management error found while changing tag");

	if (tag >= PU_PURGELEVEL && (size_t)block->user < 0x100)
		I_Error("Internal memory management error: an owner is required for purgable blocks");

	block->tag = tag;
}

//
// Z_FreeMemory
//
static inline void Z_FreeMemory(size_t *realfree, size_t *cachemem, size_t *usedmem, size_t *largefreeblock)
{
	memblock_t *block;
	size_t freeblock = 0;

	*realfree = 0;
	*cachemem = 0;
	*usedmem = 0;
	*largefreeblock = 0;

	if (!mainzone)
	{
		return;
	}

	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next)
	{
		if (!block->user)
		{
			// free memory
			*realfree += block->size;
			freeblock += block->size;
			if (freeblock > *largefreeblock)
				*largefreeblock = freeblock;
		}
		else if (block->tag >= PU_PURGELEVEL)
		{
			// purgable memory (cache)
			*cachemem += block->size;
			freeblock += block->size;
			if (freeblock>*largefreeblock)
				*largefreeblock = freeblock;
		}
		else
		{
			// used block
			*usedmem += block->size;
			freeblock = 0;
		}
	}
}

/** Calculates memory usage for a given tag.
  *
  * \param tagnum The tag.
  * \return Number of bytes currently allocated in the heap for the given tag.
  */
size_t Z_TagUsage(int tagnum)
{
	memblock_t *block;
	size_t bytes = 0;

	if (mainzone)
		for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next)
			if (block->user && block->tag == tagnum)
				bytes += block->size;

	return bytes;
}

/** Prints memory statistics to the console.
  */
static void Command_Memfree_f(void)
{
	size_t memfree, cache, used, largefreeblock;
	ULONG freebytes, totalbytes;

	Z_CheckHeap(-1);
	Z_FreeMemory(&memfree, &cache, &used, &largefreeblock);
	CONS_Printf("\2Memory Heap Info\n");
	CONS_Printf("Total heap size   : %7d KB\n", mb_used<<10);
	CONS_Printf("used memory       : %7d KB\n", used>>10);
	CONS_Printf("free memory       : %7d KB\n", memfree>>10);
	CONS_Printf("cache memory      : %7d KB\n", cache>>10);
	CONS_Printf("largest free block: %7d KB\n", largefreeblock>>10);
#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
	{
		CONS_Printf("Patch info headers: %7d KB\n", Z_TagUsage(PU_HWRPATCHINFO)>>10);
		CONS_Printf("HW Texture cache  : %7d KB\n", Z_TagUsage(PU_HWRCACHE)>>10);
		CONS_Printf("Plane polygone    : %7d KB\n", Z_TagUsage(PU_HWRPLANE)>>10);
		CONS_Printf("HW Texture used   : %7d KB\n", HWR_GetTextureUsed()>>10);
	}
#endif

	CONS_Printf("\2System Memory Info\n");
	freebytes = I_GetFreeMem(&totalbytes);
	CONS_Printf("    Total physical memory: %7lu KB\n", totalbytes>>10);
	CONS_Printf("Available physical memory: %7lu KB\n", freebytes>>10);
}

// creates a copy of a string, null-terminated
// returns ptr to the new duplicate string
//
char *Z_StrDup(const char *in)
{
	return strcpy(ZZ_Alloc(strlen(in) + 1), in);
}
