// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// This file is in the public domain.
// Written by Graue in 2006.
//
// This file does zone memory allocation. Each allocation is done with a
// tag, and this file keeps track of all the allocations made. Later, you
// can purge everything with a given tag.
//
// Some tags (PU_CACHE, for example) may be automatically purged whenever
// the space is needed, so memory allocated with these tags is no longer
// guaranteed to be valid after another call to Z_Malloc().
//
// The original implementation allocated a large block (48 MB, as of the
// last version of SRB2 that did this) upfront, and Z_Malloc() carved
// pieces out of that. Unfortunately, this had the effect of masking a
// lot of read/write past end of buffer type bugs which we have since
// caught with this direct-malloc version. We also suspected that SRB2's
// allocator was fragmenting badly. Finally, this version is a bit
// simpler (about half the lines of code).
//
//-----------------------------------------------------------------------------
/// \file
/// \brief Zone memory allocation.

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h" // I_GetFreeMem
#include "i_video.h" // rendermode
#include "z_zone.h"
#include "m_misc.h" // M_Memcpy

#ifdef HWRENDER
#include "hardware/hw_main.h" // For hardware memory info
#endif

#define ZONEID 0xa441d13d

struct memblock_s;

typedef struct
{
	struct memblock_s *block; // Describing this memory
	UINT32 id; // Should be ZONEID
} ATTRPACK memhdr_t;

// Some code might want aligned memory. Assume it wants memory n bytes
// aligned -- then we allocate n-1 extra bytes and return a pointer to
// the first byte aligned as requested.
// Thus, "real" is the pointer we get from malloc() and will free()
// later, but "hdr" is where the memhdr_t starts.
// For non-aligned allocations they will be the same.
typedef struct memblock_s
{
	void *real;
	memhdr_t *hdr;

	void **user;
	INT32 tag; // purgelevel

	size_t size; // including the header and blocks
	size_t realsize; // size of real data only

#ifdef ZDEBUG
	const char *ownerfile;
	INT32 ownerline;
#endif

	struct memblock_s *next, *prev;
} ATTRPACK memblock_t;

#ifdef ZDEBUG
#define Ptr2Memblock(s, f) Ptr2Memblock2(s, f, __FILE__, __LINE__)
static memblock_t *Ptr2Memblock2(void *ptr, const char* func, const char *file, INT32 line)
#else
static memblock_t *Ptr2Memblock(void *ptr, const char* func)
#endif
{
	memhdr_t *hdr;

	if (ptr == NULL)
		return NULL;

#ifdef ZDEBUG2
	CONS_Printf("%s %s:%d\n", func, file, line);
#endif

	hdr = (memhdr_t *)((UINT8 *)ptr - sizeof *hdr);
	if (hdr->id != ZONEID)
	{
#ifdef ZDEBUG
		I_Error("%s: wrong id from %s:%d", func, file, line);
#else
		I_Error("%s: wrong id", func);
#endif
	}
	return hdr->block;

}

static memblock_t head;

static void Command_Memfree_f(void);

void Z_Init(void)
{
	UINT32 total, memfree;

	head.next = head.prev = &head;

	memfree = I_GetFreeMem(&total)>>20;
	CONS_Printf("system memory %uMB free %uMB\n", total>>20, memfree);

	// Note: This allocates memory. Watch out.
	COM_AddCommand("memfree", Command_Memfree_f);
}

#ifdef ZDEBUG
void Z_Free2(void *ptr, const char *file, INT32 line)
#else
void Z_Free(void *ptr)
#endif
{
	memblock_t *block;

	if (ptr == NULL)
		return;

#ifdef ZDEBUG2
	CONS_Printf("Z_Free %s:%d\n", file, line);
#endif

#ifdef ZDEBUG
	block = Ptr2Memblock2(ptr, "Z_Free", file, line);
#else
	block = Ptr2Memblock(ptr, "Z_Free");
#endif

#ifdef ZDEBUG
	// Write every Z_Free call to a debug file.
	DEBFILE(va("Z_Free at %s:%d\n", file, line));
#endif

	// TODO: if zdebugging, make sure no other block has a user
	// that is about to be freed.

	// Clear the user's mark.
	if (block->user != NULL)
		*block->user = NULL;

	// Free the memory and get rid of the block.
	free(block->real);
	block->prev->next = block->next;
	block->next->prev = block->prev;
	free(block);
}

// malloc() that doesn't accept failure.
static void *xm(size_t size)
{
	void *p = malloc(size);
	if (p == NULL)
		I_Error("Out of memory allocating %"PRIdS" bytes",
			size);
	return p;
}

// Z_Malloc
// You can pass Z_Malloc() a NULL user if the tag is less than
// PU_PURGELEVEL.

#ifdef ZDEBUG
void *Z_Malloc2(size_t size, INT32 tag, void *user, INT32 alignbits,
	const char *file, INT32 line)
#else
void *Z_MallocAlign(size_t size, INT32 tag, void *user, INT32 alignbits)
#endif
{
	size_t extrabytes = (1<<alignbits) - 1;
	memblock_t *block;
	void *ptr;
	memhdr_t *hdr;
	void *given;

#ifdef ZDEBUG2
	CONS_Printf("Z_Malloc %s:%d\n", file, line);
#endif

	block = xm(sizeof *block);
	ptr = xm(extrabytes + sizeof *hdr + size);

#ifdef _NDS
	if ((size_t)ptr + extrabytes + sizeof *hdr + size > 0x2400000)
		I_Error("Out of heap!");
#endif

	// This horrible calculation makes sure that "given" is aligned
	// properly.
	given = (void *)((size_t)((UINT8 *)ptr + extrabytes + sizeof *hdr)
		& ~extrabytes);

	// The mem header lives 'sizeof (memhdr_t)' bytes before given.
	hdr = (memhdr_t *)((UINT8 *)given - sizeof *hdr);

	block->next = head.next;
	block->prev = &head;
	block->next->prev = head.next = block;

	block->real = ptr;
	block->hdr = hdr;
	block->tag = tag;
	block->user = NULL;
#ifdef ZDEBUG
	block->ownerline = line;
	block->ownerfile = file;
#endif
	block->size = size + extrabytes + sizeof *hdr;
	block->realsize = size;

	hdr->id = ZONEID;
	hdr->block = block;

	if (user != NULL)
	{
		block->user = user;
		*(void **)user = given;
	}
	else if (tag >= PU_PURGELEVEL)
		I_Error("Z_Malloc: attempted to allocate purgable block "
			"(size %"PRIdS") with no user", size);

	return given;
}

#ifdef ZDEBUG
void *Z_Calloc2(size_t size, INT32 tag, void *user, INT32 alignbits, const char *file, INT32 line)
#else
void *Z_CallocAlign(size_t size, INT32 tag, void *user, INT32 alignbits)
#endif
{
#ifdef ZDEBUG
	return memset(Z_Malloc2    (size, tag, user, alignbits, file, line), 0, size);
#else
	return memset(Z_MallocAlign(size, tag, user, alignbits            ), 0, size);
#endif
}

#ifdef ZDEBUG
void *Z_Realloc2(void *ptr, size_t size, INT32 tag, void *user, INT32 alignbits, const char *file, INT32 line)
#else
void *Z_ReallocAlign(void *ptr, size_t size,INT32 tag, void *user,  INT32 alignbits)
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

#ifdef ZDEBUG
	block = Ptr2Memblock2(ptr, "Z_Realloc", file, line);
#else
	block = Ptr2Memblock(ptr, "Z_Realloc");
#endif

	if (block == NULL)
		return NULL;

#ifdef ZDEBUG
	// Write every Z_Realloc call to a debug file.
	DEBFILE(va("Z_Realloc at %s:%d\n", file, line));
	rez = Z_MallocAlign2(size, tag, user, alignbits, file, line);
#else
	rez = Z_MallocAlign(size, tag, user, alignbits);
#endif

	if (size < block->realsize)
		copysize = size;
	else
		copysize = block->realsize;

	M_Memcpy(rez, ptr, copysize);

	Z_Free(ptr);

	if (size > copysize)
		memset((char*)rez+copysize, 0x00, size-copysize);

	return rez;
}

void Z_FreeTags(INT32 lowtag, INT32 hightag)
{
	memblock_t *block, *next;

	Z_CheckHeap(420);
	for (block = head.next; block != &head; block = next)
	{
		next = block->next; // get link before freeing

		if (block->tag >= lowtag && block->tag <= hightag)
			Z_Free((UINT8 *)block->hdr + sizeof *block->hdr);
	}
}

//
// Z_CheckMemCleanup
//
// TODO: Currently blocks >= PU_PURGELEVEL are freed every
// CLEANUPCOUNT. It might be better to keep track of
// the total size of all purgable memory and free it when the
// size exceeds some value.
//
// This was in Z_Malloc, but was freeing data at
// unsafe times. Now it is only called when it is safe
// to cleanup memory.

#define CLEANUPCOUNT 2000

static INT32 nextcleanup = CLEANUPCOUNT;

void Z_CheckMemCleanup(void)
{
	if (nextcleanup-- == 0)
	{
		nextcleanup = CLEANUPCOUNT;
		Z_FreeTags(PU_PURGELEVEL, INT32_MAX);
	}
}


/** Checks the heap, as well as the memhdr_ts, for any corruption or
  * other problems.
  * \param i Identifies from where in the code Z_CheckHeap was called.
  * \author Graue <graue@oceanbase.org>
  */
void Z_CheckHeap(INT32 i)
{
	memblock_t *block;
	UINT32 blocknumon = 0;
	void *given;

	for (block = head.next; block != &head; block = block->next)
	{
		blocknumon++;
		given = (UINT8 *)block->hdr + sizeof *(block->hdr);
#ifdef ZDEBUG
		CONS_Printf("block %u owned by %s:%d\n",
			blocknumon, block->ownerfile, block->ownerline);
#endif
		if (block->user != NULL && *(block->user) != given)
		{
			I_Error("Z_CheckHeap %d: block %u doesn't have a "
				"proper user", i, blocknumon);
		}
		if (block->next->prev != block)
		{
			I_Error("Z_CheckHeap %d: block %u lacks proper "
				"backlink", i, blocknumon);
		}
		if (block->prev->next != block)
		{
			I_Error("Z_CheckHeap %d: block %u lacks proper "
				"forward link", i, blocknumon);
		}
		if (block->hdr->block != block)
		{
			I_Error("Z_CheckHeap %d: block %u doesn't have "
				"linkback from allocated memory",
				i, blocknumon);
		}
		if (block->hdr->id != ZONEID)
		{
			I_Error("Z_CheckHeap %d: block %u's memory has "
				"wrong ID", i, blocknumon);
		}
	}
}

#ifdef PARANOIA
void Z_ChangeTag2(void *ptr, INT32 tag, const char *file, INT32 line)
#else
void Z_ChangeTag2(void *ptr, INT32 tag)
#endif
{
	memblock_t *block;
	memhdr_t *hdr;

	if (ptr == NULL)
		return;

	hdr = (memhdr_t *)((UINT8 *)ptr - sizeof *hdr);

#ifdef PARANOIA
	if (hdr->id != ZONEID) I_Error("Z_CT at %s:%d: wrong id", file, line);
#endif

	block = hdr->block;

	if (tag >= PU_PURGELEVEL && block->user == NULL)
		I_Error("Internal memory management error: "
			"tried to make block purgable but it has no owner");

	block->tag = tag;
}

/** Calculates memory usage for a given set of tags.
  * \param lowtag The lowest tag to consider.
  * \param hightag The highest tag to consider.
  * \return Number of bytes currently allocated in the heap for the
  *         given tags.
  * \sa Z_TagUsage
  */
static size_t Z_TagsUsage(INT32 lowtag, INT32 hightag)
{
	size_t cnt = 0;
	memblock_t *rover;

	for (rover = head.next; rover != &head; rover = rover->next)
	{
		if (rover->tag < lowtag || rover->tag > hightag)
			continue;
		cnt += rover->size + sizeof *rover;
	}

	return cnt;
}

size_t Z_TagUsage(INT32 tagnum)
{
	return Z_TagsUsage(tagnum, tagnum);
}

void Command_Memfree_f(void)
{
	UINT32 freebytes, totalbytes;

	Z_CheckHeap(-1);
	CONS_Printf("\2Memory Info\n");
	CONS_Printf("Total heap used   : %7"PRIdS" KB\n", Z_TagsUsage(0, INT32_MAX)>>10);
	CONS_Printf("Static            : %7"PRIdS" KB\n", Z_TagUsage(PU_STATIC)>>10);
	CONS_Printf("Static (sound)    : %7"PRIdS" KB\n", Z_TagUsage(PU_SOUND)>>10);
	CONS_Printf("Static (music)    : %7"PRIdS" KB\n", Z_TagUsage(PU_MUSIC)>>10);
	CONS_Printf("Level             : %7"PRIdS" KB\n", Z_TagUsage(PU_LEVEL)>>10);
	CONS_Printf("Special thinker   : %7"PRIdS" KB\n", Z_TagUsage(PU_LEVSPEC)>>10);
	CONS_Printf("All purgable      : %7"PRIdS" KB\n",
		Z_TagsUsage(PU_PURGELEVEL, INT32_MAX)>>10);

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
	{
		CONS_Printf("Patch info headers: %7"PRIdS" KB\n",
			Z_TagUsage(PU_HWRPATCHINFO)>>10);
		CONS_Printf("Mipmap patches    : %7"PRIdS" KB\n",
			Z_TagUsage(PU_HWRPATCHCOLMIPMAP)>>10);
		CONS_Printf("HW Texture cache  : %7"PRIdS" KB\n",
			Z_TagUsage(PU_HWRCACHE)>>10);
		CONS_Printf("Plane polygons    : %7"PRIdS" KB\n",
			Z_TagUsage(PU_HWRPLANE)>>10);
		CONS_Printf("HW Texture used   : %7d KB\n",
			HWR_GetTextureUsed()>>10);
	}
#endif

	CONS_Printf("\2System Memory Info\n");
	freebytes = I_GetFreeMem(&totalbytes);
	CONS_Printf("    Total physical memory: %7u KB\n", totalbytes>>10);
	CONS_Printf("Available physical memory: %7u KB\n", freebytes>>10);
}

// Creates a copy of a string.
char *Z_StrDup(const char *s)
{
	return strcpy(ZZ_Alloc(strlen(s) + 1), s);
}
