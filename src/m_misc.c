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
/// \brief Commonly used routines
///
///	Default Config File.
///	PCX Screenshots.
///	File i/o

#ifdef __GNUC__
#include <unistd.h>
#endif
// Extended map support.
#include <ctype.h>

#include "doomdef.h"
#include "g_game.h"
#include "m_misc.h"
#include "hu_stuff.h"
#include "v_video.h"
#include "z_zone.h"
#include "g_input.h"
#include "i_video.h"
#include "d_main.h"
#include "m_argv.h"
#include "i_system.h"

#ifdef _WIN32_WCE
#include "sdl/SRB2CE/cehelp.h"
#endif

#ifdef _XBOX
#include "sdl/SRB2XBOX/xboxhelp.h"
#endif

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#ifdef SDL
#include "sdl/hwsym_sdl.h"
#endif

#ifdef HAVE_PNG

#ifndef _MSC_VER
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#endif

#ifndef _LFS64_LARGEFILE
#define _LFS64_LARGEFILE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 0
#endif

 #include "png.h"
 #ifdef PNG_WRITE_SUPPORTED
  #define USE_PNG // Only actually use PNG if write is supported.
  #if defined (PNG_WRITE_APNG_SUPPORTED) //|| !defined(PNG_STATIC)
   #define USE_APNG
  #endif
  // See hardware/hw_draw.c for a similar check to this one.
 #endif
#endif

static CV_PossibleValue_t screenshot_cons_t[] = {{0, "Default"}, {1, "HOME"}, {2, "SRB2"}, {3, "CUSTOM"}, {0, NULL}};
consvar_t cv_screenshot_option = {"screenshot_option", "Default", CV_SAVE, screenshot_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_screenshot_folder = {"screenshot_folder", "", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

static CV_PossibleValue_t zlib_level_t[] = {
	{-1, "Default"}, //Z_DEFAULT_COMPRESSION
	{0, "None"},  //Z_NO_COMPRESSION
	{1, "Best Speed"}, //Z_BEST_SPEED
	{2, "Level 2"}, {3, "Level 3"}, {4, "Level 4"}, {5, "Level 5"},
	{6, "Normal"}, //Zlib Default
	{7, "Level 7"}, {8, "Level 8"},
	{9, "Best Compression"}, //Z_BEST_COMPRESSION
	{0, NULL}};

static CV_PossibleValue_t zlib_mem_level_t[] = {
	{-1, "Default"}, //do not set mem_level
	{0, "Level 0"}, {1, "Level 1"}, {2, "Level 2"}, {3, "Level 3"},
	{4, "Level 4"}, {5, "Level 5"}, {6, "Level 6"}, {7, "Level 7"},
	{8, "Normal"}, //libpng Default
	{9, "Level 9"}, {0, NULL}};

static CV_PossibleValue_t zlib_strategy_t[] = {
	{-1, "Default"},
	{0, "Normal"}, //Z_DEFAULT_STRATEGY
	{1, "Filtered"}, //Z_FILTERED
	{2, "Huffman Only"}, //Z_HUFFMAN_ONLY
	{3, "RLE"}, //Z_RLE
	{4, "Fixed"}, //Z_FIXED
	{0, NULL}};

static CV_PossibleValue_t zlib_window_bits_t[] = {
	{-1, "Default"},
#ifdef WBITS_8_OK
	{8, "256"},
#endif
	{9, "512"}, {10, "1k"}, {11, "2k"}, {12, "4k"}, {13, "8k"},
	{14, "16k"}, {15, "32k"},
	{0, NULL}};

consvar_t cv_zlib_level = {"png_z_compression_level", "Default", CV_SAVE, zlib_level_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_memory = {"png_z_memory_level", "Level 7", CV_SAVE, zlib_mem_level_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_strategy = {"png_z_strategy", "Default", CV_SAVE, zlib_strategy_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_window_bits = {"png_z_window_bits", "Default", CV_SAVE, zlib_window_bits_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_zlib_levela = {"apng_z_compression_level", "Default", CV_SAVE, zlib_level_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_memorya = {"apng_z_memory_level", "Level 7", CV_SAVE, zlib_mem_level_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_strategya = {"apng_z_strategy", "RLE", CV_SAVE, zlib_strategy_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_zlib_window_bitsa = {"apng_z_window_bits", "Default", CV_SAVE, zlib_window_bits_t, NULL, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_apng_disable = {"apng_disable", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

boolean takescreenshot = false; // Take a screenshot this tic
boolean moviemode = false; // disable screenshot message in Movie mode

/** Returns the map number for a map identified by the last two characters in
  * its name.
  *
  * \param first  The first character after MAP.
  * \param second The second character after MAP.
  * \return The map number, or 0 if no map corresponds to these characters.
  * \sa G_BuildMapName
  */
INT32 M_MapNumber(char first, char second)
{
	if (isdigit(first))
	{
		if (isdigit(second))
			return ((INT32)first - '0') * 10 + ((INT32)second - '0');
		return 0;
	}

	if (!isalpha(first))
		return 0;
	if (!isalnum(second))
		return 0;

	return 100 + ((INT32)tolower(first) - 'a') * 36 + (isdigit(second) ? ((INT32)second - '0') :
		((INT32)tolower(second) - 'a') + 10);
}

// ==========================================================================
//                         FILE INPUT / OUTPUT
// ==========================================================================

//
// FIL_WriteFile
//
#ifndef O_BINARY
#define O_BINARY 0
#endif

/** Writes out a file.
  *
  * \param name   Name of the file to write.
  * \param source Memory location to write from.
  * \param length How many bytes to write.
  * \return True on success, false on failure.
  */
boolean FIL_WriteFile(char const *name, const void *source, size_t length)
{
	FILE *handle = NULL;
	size_t count;

	//if (FIL_WriteFileOK(name))
		handle = fopen(name, "w+b");

	if (!handle)
		return false;

	count = fwrite(source, 1, length, handle);
	fclose(handle);

	if (count < length)
		return false;

	return true;
}

/** Reads in a file, appending a zero byte at the end.
  *
  * \param name   Filename to read.
  * \param buffer Pointer to a pointer, which will be set to the location of a
  *               newly allocated buffer holding the file's contents.
  * \return Number of bytes read, not counting the zero byte added to the end,
  *         or 0 on error.
  */
size_t FIL_ReadFile(char const *name, UINT8 **buffer)
{
	FILE *handle = NULL;
	size_t count, length;
	UINT8 *buf;

	if (FIL_ReadFileOK(name))
		handle = fopen(name, "rb");

	if (!handle)
		return 0;

	fseek(handle,0,SEEK_END);
	length = ftell(handle);
	fseek(handle,0,SEEK_SET);

	buf = Z_Malloc(length + 1, PU_STATIC, NULL);
	count = fread(buf, 1, length, handle);
	fclose(handle);

	if (count < length)
	{
		Z_Free(buf);
		return 0;
	}

	// append 0 byte for script text files
	buf[length] = 0;

	*buffer = buf;
	return length;
}

/** Check if the filename exists
  *
  * \param name   Filename to check.
  * \return true if file exists, false if it doesn't.
  */
boolean FIL_FileExists(char const *name)
{
	return access(name,0)+1;
}


/** Check if the filename OK to write
  *
  * \param name   Filename to check.
  * \return true if file write-able, false if it doesn't.
  */
boolean FIL_WriteFileOK(char const *name)
{
	return access(name,2)+1;
}


/** Check if the filename OK to read
  *
  * \param name   Filename to check.
  * \return true if file read-able, false if it doesn't.
  */
boolean FIL_ReadFileOK(char const *name)
{
	return access(name,4)+1;
}

/** Check if the filename OK to read/write
  *
  * \param name   Filename to check.
  * \return true if file (read/write)-able, false if it doesn't.
  */
boolean FIL_FileOK(char const *name)
{
	return access(name,6)+1;
}


/** Checks if a pathname has a file extension and adds the extension provided
  * if not.
  *
  * \param path      Pathname to check.
  * \param extension Extension to add if no extension is there.
  */
void FIL_DefaultExtension(char *path, const char *extension)
{
	char *src;

	// search for '.' from end to begin, add .EXT only when not found
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
			return; // it has an extension
		src--;
	}

	strcat(path, extension);
}

void FIL_ForceExtension(char *path, const char *extension)
{
	char *src;

	// search for '.' from end to begin, add .EXT only when not found
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path)
	{
		if (*src == '.')
		{
			*src = '\0';
			break; // it has an extension
		}
		src--;
	}

	strcat(path, extension);
}

/** Checks if a filename extension is found.
  * Lump names do not contain dots.
  *
  * \param in String to check.
  * \return True if an extension is found, otherwise false.
  */
boolean FIL_CheckExtension(const char *in)
{
	while (*in++)
		if (*in == '.')
			return true;

	return false;
}

// ==========================================================================
//                        CONFIGURATION FILE
// ==========================================================================

//
// DEFAULTS
//

char configfile[MAX_WADPATH];

// ==========================================================================
//                          CONFIGURATION
// ==========================================================================
static boolean gameconfig_loaded = false; // true once config.cfg loaded AND executed

/** Saves a player's config, possibly to a particular file.
  *
  * \sa Command_LoadConfig_f
  */
void Command_SaveConfig_f(void)
{
	char tmpstr[MAX_WADPATH];

	if (COM_Argc() < 2)
	{
		CONS_Printf("saveconfig <filename[.cfg]> [-silent] : save config to a file\n");
		return;
	}
	strcpy(tmpstr, COM_Argv(1));
	FIL_ForceExtension(tmpstr, ".cfg");

	M_SaveConfig(tmpstr);
	if (stricmp(COM_Argv(2), "-silent"))
		CONS_Printf("config saved as %s\n", configfile);
}

/** Loads a game config, possibly from a particular file.
  *
  * \sa Command_SaveConfig_f, Command_ChangeConfig_f
  */
void Command_LoadConfig_f(void)
{
	if (COM_Argc() != 2)
	{
		CONS_Printf("loadconfig <filename[.cfg]> : load config from a file\n");
		return;
	}

	strcpy(configfile, COM_Argv(1));
	FIL_ForceExtension(configfile, ".cfg");
	COM_BufInsertText(va("exec \"%s\"\n", configfile));
}

/** Saves the current configuration and loads another.
  *
  * \sa Command_LoadConfig_f, Command_SaveConfig_f
  */
void Command_ChangeConfig_f(void)
{
	if (COM_Argc() != 2)
	{
		CONS_Printf("changeconfig <filename[.cfg]> : save current config and load another\n");
		return;
	}

	COM_BufAddText(va("saveconfig \"%s\"\n", configfile));
	COM_BufAddText(va("loadconfig \"%s\"\n", COM_Argv(1)));
}

/** Loads the default config file.
  *
  * \sa Command_LoadConfig_f
  */
void M_FirstLoadConfig(void)
{
	// configfile is initialised by d_main when searching for the wad?

	// check for a custom config file
	if (M_CheckParm("-config") && M_IsNextParm())
	{
		strcpy(configfile, M_GetNextParm());
		CONS_Printf("config file: %s\n",configfile);
	}

	// load default control
	G_Controldefault();

	// load config, make sure those commands doesnt require the screen..
	CONS_Printf("\n");
	COM_BufInsertText(va("exec \"%s\"\n", configfile));
	// no COM_BufExecute() needed; that does it right away

	// make sure I_Quit() will write back the correct config
	// (do not write back the config if it crash before)
	gameconfig_loaded = true;
}

/** Saves the game configuration.
  *
  * \sa Command_SaveConfig_f
  */
void M_SaveConfig(const char *filename)
{
	FILE *f;

	// make sure not to write back the config until it's been correctly loaded
	if (!gameconfig_loaded)
		return;

	// can change the file name
	if (filename)
	{
		if (!strstr(filename, ".cfg"))
		{
			CONS_Printf("M_SaveConfig(): filename is not .cfg\n");
			return;
		}

		f = fopen(filename, "w");
		// change it only if valid
		if (f)
			strcpy(configfile, filename);
		else
		{
			CONS_Printf("Couldn't save game config file %s\n", filename);
			return;
		}
	}
	else
	{
		if (!strstr(configfile, ".cfg"))
		{
			CONS_Printf("M_SaveConfig(): filename is not .cfg\n");
			return;
		}

		f = fopen(configfile, "w");
		if (!f)
		{
			CONS_Printf("Couldn't save game config file %s\n", configfile);
			return;
		}
	}

	// header message
	fprintf(f, "// SRB2 configuration file.\n");

	// FIXME: save key aliases if ever implemented..

	CV_SaveVariables(f);
	if (!dedicated) G_SaveKeySetting(f);

	fclose(f);
}


#if NUMSCREENS > 2
static const char *Newsnapshotfile(const char *pathname, const char *ext)
{
	static char freename[13] = "srb2XXXX.ext";
	int i = 5000; // start in the middle: num screenshots divided by 2
	int add = i; // how much to add or subtract if wrong; gets divided by 2 each time
	int result; // -1 = guess too high, 0 = correct, 1 = guess too low

	// find a file name to save it to
	strcpy(freename+9,ext);

	for (;;)
	{
		freename[4] = (char)('0' + (char)(i/1000));
		freename[5] = (char)('0' + (char)((i/100)%10));
		freename[6] = (char)('0' + (char)((i/10)%10));
		freename[7] = (char)('0' + (char)(i%10));

		if (FIL_WriteFileOK(va(pandf,pathname,freename))) // access succeeds
			result = 1; // too low
		else // access fails: equal or too high
		{
			if (!i)
				break; // not too high, so it must be equal! YAY!

			freename[4] = (char)('0' + (char)((i-1)/1000));
			freename[5] = (char)('0' + (char)(((i-1)/100)%10));
			freename[6] = (char)('0' + (char)(((i-1)/10)%10));
			freename[7] = (char)('0' + (char)((i-1)%10));
			if (!FIL_WriteFileOK(va(pandf,pathname,freename))) // access fails
				result = -1; // too high
			else
				break; // not too high, so equal, YAY!
		}

		add /= 2;

		if (!add) // don't get stuck at 5 due to truncation!
			add = 1;

		i += add * result;

		if (add < 0 || add > 9999)
			return NULL;
	}

	freename[4] = (char)('0' + (char)(i/1000));
	freename[5] = (char)('0' + (char)((i/100)%10));
	freename[6] = (char)('0' + (char)((i/10)%10));
	freename[7] = (char)('0' + (char)(i%10));

	return freename;
}
#endif

#ifdef HAVE_PNG
FUNCNORETURN static void PNG_error(png_structp PNG, png_const_charp pngtext)
{
	//CONS_Printf("libpng error at %p: %s", PNG, pngtext);
	I_Error("libpng error at %p: %s", PNG, pngtext);
}

static void PNG_warn(png_structp PNG, png_const_charp pngtext)
{
	CONS_Printf("libpng warning at %p: %s", PNG, pngtext);
}

static void M_PNGhdr(png_structp png_ptr, png_infop png_info_ptr, PNG_CONST png_uint_32 width, PNG_CONST png_uint_32 height, PNG_CONST png_byte *palette)
{
	const png_byte png_interlace = PNG_INTERLACE_NONE; //PNG_INTERLACE_ADAM7
	if (palette)
	{
		png_colorp png_PLTE = png_malloc(png_ptr, sizeof(png_color)*256); //palette
		const png_byte *pal = palette;
		png_uint_16 i;
		for (i = 0; i < 256; i++)
		{
			png_PLTE[i].red   = *pal; pal++;
			png_PLTE[i].green = *pal; pal++;
			png_PLTE[i].blue  = *pal; pal++;
		}
		png_set_IHDR(png_ptr, png_info_ptr, width, height, 8, PNG_COLOR_TYPE_PALETTE,
		 png_interlace, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_write_info_before_PLTE(png_ptr, png_info_ptr);
		png_set_PLTE(png_ptr, png_info_ptr, png_PLTE, 256);
		png_free(png_ptr, (png_voidp)png_PLTE); // safe in libpng-1.2.1+
		png_set_filter(png_ptr, PNG_FILTER_TYPE_BASE, PNG_FILTER_NONE);
		png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
	}
	else
	{
		png_set_IHDR(png_ptr, png_info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
		 png_interlace, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_write_info_before_PLTE(png_ptr, png_info_ptr);
		png_set_compression_strategy(png_ptr, Z_FILTERED);
	}
}

static void M_PNGText(png_structp png_ptr, png_infop png_info_ptr, PNG_CONST png_byte movie)
{
#ifdef PNG_TEXT_SUPPORTED
#define SRB2PNGTXT 8
	png_text png_infotext[SRB2PNGTXT];
	char keytxt[SRB2PNGTXT][12] = {"Title", "Author", "Description", "Playername", "Mapnum", "Mapname", "Location", "Interface"}; //PNG_KEYWORD_MAX_LENGTH(79) is the max
	char titletxt[] = "Sonic Robo Blast 2"VERSIONSTRING;
	png_charp authortxt = I_GetUserName();
	png_charp playertxt =  cv_playername.zstring;
	char desctxt[] = "SRB2 Screenshot";
	char Movietxt[] = "SRB2 Movie";
	size_t i;
	char interfacetxt[] =
#ifdef SDL
	 "SDL";
#elif defined (_WINDOWS)
	 "DirectX";
#elif defined (PC_DOS)
	 "Allegro";
#else
	 "Unknown";
#endif
	char maptext[8];
	char lvlttltext[48];
	char locationtxt[40];

	if (gamestate == GS_LEVEL)
		snprintf(maptext, 8, "%s", G_BuildMapName(gamemap));
	else
		snprintf(maptext, 8, "Unknown");

	if (gamestate == GS_LEVEL && mapheaderinfo[gamemap-1].lvlttl)
		snprintf(lvlttltext, 48, "%s%s%s",
			mapheaderinfo[gamemap-1].lvlttl,
			(mapheaderinfo[gamemap-1].nozone) ? "" : " ZONE",
			(mapheaderinfo[gamemap-1].actnum > 0) ? va(" %d",mapheaderinfo[gamemap-1].actnum) : "");
	else
		snprintf(lvlttltext, 48, "Unknown");

	if (gamestate == GS_LEVEL && &players[displayplayer] && players[displayplayer].mo)
		snprintf(locationtxt, 40, "X:%d Y:%d Z:%d A:%d",
			players[displayplayer].mo->x>>FRACBITS,
			players[displayplayer].mo->y>>FRACBITS,
			players[displayplayer].mo->z>>FRACBITS,
			FixedInt(AngleFixed(players[displayplayer].mo->angle)));
	else
		snprintf(locationtxt, 40, "Unknown");

	png_memset(png_infotext,0x00,sizeof (png_infotext));

	for (i = 0; i < SRB2PNGTXT; i++)
		png_infotext[i].key  = keytxt[i];

	png_infotext[0].text = titletxt;
	png_infotext[1].text = authortxt;
	if (movie)
		png_infotext[2].text = Movietxt;
	else
		png_infotext[2].text = desctxt;
	png_infotext[3].text = playertxt;
	png_infotext[4].text = maptext;
	png_infotext[5].text = lvlttltext;
	png_infotext[6].text = locationtxt;
	png_infotext[7].text = interfacetxt;

	png_set_text(png_ptr, png_info_ptr, png_infotext, SRB2PNGTXT);
#undef SRB2PNGTXT
#endif
}

static inline void M_PNGImage(png_structp png_ptr, png_infop png_info_ptr, PNG_CONST png_uint_32 height, png_bytep png_buf)
{
	png_uint_32 pitch = png_get_rowbytes(png_ptr, png_info_ptr);
	png_bytepp row_pointers = png_malloc(png_ptr, height* sizeof (png_bytep));
	png_uint_32 y;
	for (y = 0; y < height; y++)
	{
		row_pointers[y] = png_buf;
		png_buf += pitch;
	}
	png_write_image(png_ptr, row_pointers);
	png_free(png_ptr, (png_voidp)row_pointers);
}

#ifdef USE_APNG
static png_structp apng_ptr = NULL;
static png_infop   apng_info_ptr = NULL;
static png_FILE_p  apng_FILE = NULL;
static png_uint_32 apng_frames = 0;
static png_byte    acTL_cn[5] = { 97,  99,  84,  76, '\0'};
#ifdef PNG_STATIC // Win32 build have static libpng
#define apng_set_acTL png_set_acTL
#define apng_write_frame_head png_write_frame_head
#define apng_write_frame_tail png_write_frame_tail
#else // outside libpng may not have apng support

#ifndef PNG_WRITE_APNG_SUPPORTED // libpng header may not have apng patch

#ifndef PNG_INFO_acTL
#define PNG_INFO_acTL 0x10000L
#endif
#ifndef PNG_INFO_fcTL
#define PNG_INFO_fcTL 0x20000L
#endif
#ifndef PNG_FIRST_FRAME_HIDDEN
#define PNG_FIRST_FRAME_HIDDEN       0x0001
#endif
#ifndef PNG_DISPOSE_OP_NONE
#define PNG_DISPOSE_OP_NONE        0x00
#endif
#ifndef PNG_DISPOSE_OP_BACKGROUND
#define PNG_DISPOSE_OP_BACKGROUND  0x01
#endif
#ifndef PNG_DISPOSE_OP_PREVIOUS
#define PNG_DISPOSE_OP_PREVIOUS    0x02
#endif
#ifndef PNG_BLEND_OP_SOURCE
#define PNG_BLEND_OP_SOURCE        0x00
#endif
#ifndef PNG_BLEND_OP_OVER
#define PNG_BLEND_OP_OVER          0x01
#endif
#ifndef PNG_HAVE_acTL
#define PNG_HAVE_acTL             0x4000
#endif
#ifndef PNG_HAVE_fcTL
#define PNG_HAVE_fcTL             0x8000L
#endif

#endif
typedef PNG_EXPORT(png_uint_32, (*P_png_set_acTL)) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_uint_32 num_frames, png_uint_32 num_plays));
typedef PNG_EXPORT (void, (*P_png_write_frame_head)) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_bytepp row_pointers,
   png_uint_32 width, png_uint_32 height,
   png_uint_32 x_offset, png_uint_32 y_offset,
   png_uint_16 delay_num, png_uint_16 delay_den, png_byte dispose_op,
   png_byte blend_op));

typedef PNG_EXPORT (void, (*P_png_write_frame_tail)) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
static P_png_set_acTL apng_set_acTL = NULL;
static P_png_write_frame_head apng_write_frame_head = NULL;
static P_png_write_frame_tail apng_write_frame_tail = NULL;
#endif

static inline boolean M_PNGLib(void)
{
#ifdef PNG_STATIC // Win32 build have static libpng
	return true;
#else
	static void *pnglib = NULL;
	if (apng_set_acTL && apng_write_frame_head && apng_write_frame_tail)
		return true;
	if (pnglib)
		return false;
#ifdef _WIN32
	pnglib = GetModuleHandleA("libpng.dll");
	if (!pnglib)
		pnglib = GetModuleHandleA("libpng12.dll");
	if (!pnglib)
		pnglib = GetModuleHandleA("libpng13.dll");
#elif defined (SDL)
#ifdef __APPLE__
	pnglib = hwOpen("libpng.dylib");
#else
	pnglib = hwOpen("libpng.so");
#endif
#endif
	if (!pnglib)
		return false;
#ifdef SDL
	apng_set_acTL = hwSym("png_set_acTL", pnglib);
	apng_write_frame_head = hwSym("png_write_frame_head", pnglib);
	apng_write_frame_tail = hwSym("png_write_frame_tail", pnglib);
#endif
#ifdef _WIN32
	apng_set_acTL = GetProcAddress("png_set_acTL", pnglib);
	apng_write_frame_head = GetProcAddress("png_write_frame_head", pnglib);
	apng_write_frame_tail = GetProcAddress("png_write_frame_tail", pnglib);
#endif
	return (apng_set_acTL && apng_write_frame_head && apng_write_frame_tail);
#endif
}

static void M_PNGFrame(png_structp png_ptr, png_infop png_info_ptr, png_bytep png_buf)
{
	png_uint_32 pitch = png_get_rowbytes(png_ptr, png_info_ptr);
	PNG_CONST png_uint_32 height = vid.height;
	png_bytepp row_pointers = png_malloc(png_ptr, height* sizeof (png_bytep));
	png_uint_32 y;

	apng_frames++;

#ifndef PNG_STATIC
	if (apng_set_acTL)
#endif
		apng_set_acTL(apng_ptr, apng_info_ptr, apng_frames, 0);

	for (y = 0; y < height; y++)
	{
		row_pointers[y] = png_buf;
		png_buf += pitch;
	}

#ifndef PNG_STATIC
	if (apng_write_frame_head)
#endif
		apng_write_frame_head(apng_ptr, apng_info_ptr, row_pointers,
			vid.width, /* width */
			height,    /* height */
			0,         /* x offset */
			0,         /* y offset */
			2, TICRATE,/* delay numerator and denominator */
			PNG_DISPOSE_OP_BACKGROUND, /* dispose */
			PNG_BLEND_OP_SOURCE        /* blend */
		                     );

	png_write_image(png_ptr, row_pointers);

#ifndef PNG_STATIC
	if (apng_write_frame_tail)
#endif
		apng_write_frame_tail(apng_ptr, apng_info_ptr);

	png_free(png_ptr, (png_voidp)row_pointers);
}

static inline boolean M_PNGfind_acTL(void)
{
	png_byte cn[8]; // 4 bytes for len then 4 byes for name
	long curpos = 0; // then , [0-2^31] bytes for data, then 4 bytes for CRC
	long endpos = ftell(apng_FILE); // not the real end of file, just what of libpng wrote
	for (fseek(apng_FILE, 0, SEEK_SET); // let go to the start of the file
	     ftell(apng_FILE)+12 < endpos;  // let not go over the file bound
	     fseek(apng_FILE, 1, SEEK_CUR)  //  we went 8 steps back and now we go 1 step forward
	    )
	{
		if (fread(cn, sizeof(cn), 1, apng_FILE) != 1) // read 8 bytes
			return false; // failed to read data
		if (fseek(apng_FILE, -8, SEEK_CUR) != 0) //rewind 8 bytes
			return false; // failed to rewird
		curpos = ftell(apng_FILE);
		if (!png_memcmp(cn+4, acTL_cn, 4)) //cmp for chuck header
			return true; // found it
	}
	return false; // acTL chuck not found
}

static void M_PNGfix_acTL(png_structp png_ptr, png_infop png_info_ptr)
{
	png_byte data[16];
	long oldpos;

#ifndef PNG_STATIC
	if (apng_set_acTL)
#endif
		apng_set_acTL(png_ptr, png_info_ptr, apng_frames, 0);

	png_debug(1, "in png_write_acTL\n");

	png_ptr->num_frames_to_write = apng_frames;

	png_save_uint_32(data, apng_frames);
	png_save_uint_32(data + 4, 0);

	oldpos = ftell(apng_FILE);

	if (M_PNGfind_acTL())
		png_write_chunk(png_ptr, (png_bytep)acTL_cn, data, (png_size_t)8);

	fseek(apng_FILE, oldpos, SEEK_SET);
}

static boolean M_SetupaPNG(png_const_charp filename, png_bytep pal)
{
	if (cv_apng_disable.value)
		return false;

	apng_FILE = fopen(filename,"wb+"); // + mode for reading
	if (!apng_FILE)
	{
		CONS_Printf("M_StartMovie: Error on opening %s for write\n",
		            filename);
		return false;
	}

	apng_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
	 PNG_error, PNG_warn);
	if (!apng_ptr)
	{
		CONS_Printf("M_StartMovie: Error on initialize libpng\n");
		fclose(apng_FILE);
		remove(filename);
		return false;
	}

	apng_info_ptr = png_create_info_struct(apng_ptr);
	if (!apng_info_ptr)
	{
		CONS_Printf("M_StartMovie: Error on allocate for libpng\n");
		png_destroy_write_struct(&apng_ptr,  NULL);
		fclose(apng_FILE);
		remove(filename);
		return false;
	}

	png_init_io(apng_ptr, apng_FILE);

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
	png_set_user_limits(apng_ptr, MAXVIDWIDTH, MAXVIDHEIGHT);
#endif

	//png_set_filter(apng_ptr, 0, PNG_ALL_FILTERS);

	if (cv_zlib_levela.value != -1)
		png_set_compression_level(apng_ptr, cv_zlib_levela.value);
	if (cv_zlib_memorya.value != -1)
		png_set_compression_mem_level(apng_ptr, cv_zlib_memorya.value);
	if (cv_zlib_strategya.value != -1)
		png_set_compression_strategy(apng_ptr, cv_zlib_strategya.value);
	if (cv_zlib_window_bitsa.value != -1)
		png_set_compression_window_bits(apng_ptr, cv_zlib_window_bitsa.value);

	M_PNGhdr(apng_ptr, apng_info_ptr, vid.width, vid.height, pal);

	M_PNGText(apng_ptr, apng_info_ptr, true);

#ifndef PNG_STATIC
	if (apng_set_acTL)
#endif
		apng_set_acTL(apng_ptr, apng_info_ptr, PNG_UINT_31_MAX, 0);

	png_write_info(apng_ptr, apng_info_ptr);

	apng_frames = 0;

	return true;
}
#endif

boolean M_StartMovie(void)
{
#ifdef USE_APNG
	const char *freename = NULL, *pathname = ".";
	boolean ret = false;

	if (!M_PNGLib())
		return false;

	if (cv_screenshot_option.value == 0)
		pathname = usehome ? srb2home : srb2path;
	else if (cv_screenshot_option.value == 1)
		pathname = srb2home;
	else if (cv_screenshot_option.value == 2)
		pathname = srb2path;
	else if (cv_screenshot_option.value == 3 && *cv_screenshot_folder.string != '\0')
		pathname = cv_screenshot_folder.string;

	if (rendermode == render_none)
		I_Error("Can't make an aPNG file without a render system");
	else
		freename = Newsnapshotfile(pathname,"png");

	if (!freename)
		goto failure;

	if (rendermode == render_soft)
		ret = M_SetupaPNG(va(pandf,pathname,freename), W_CacheLumpName(GetPalette(), PU_CACHE));
	else
		ret = M_SetupaPNG(va(pandf,pathname,freename), NULL);

failure:
	if (!ret)
	{
		if (freename)
			CONS_Printf("Couldn't create aPNG file %s in %s\n", freename, pathname);
		else
			CONS_Printf("Couldn't create aPNG file (all 10000 slots used!) in %s\n", pathname);
	}
	return ret;
#else
	return false;
#endif
}

void M_SaveFrame(void)
{
#ifdef USE_APNG
	UINT8 *linear = NULL;

	if (!apng_FILE)
	{
		COM_BufAddText("screenshot");
		return;
	}

	if (rendermode == render_soft)
	{
		// munge planar buffer to linear
		linear = screens[2];
		I_ReadScreen(linear);
	}
#ifdef HWRENDER
	else
		linear = HWR_GetScreenshot();
#endif
	M_PNGFrame(apng_ptr, apng_info_ptr, (png_bytep)linear);
#ifdef HWRENDER
	if (rendermode != render_soft && linear)
		free(linear);
#endif

	if (apng_frames == PNG_UINT_31_MAX)
	{
		M_StopMovie();
		CONS_Printf("recording into next new file\n");
		M_StartMovie();
	}
#else
	COM_BufAddText("screenshot");
#endif
}

boolean M_StopMovie(void)
{
#ifdef USE_APNG
	if (!apng_FILE)
		return false;

	if (apng_frames)
	{
		M_PNGfix_acTL(apng_ptr, apng_info_ptr);
		png_write_end(apng_ptr, apng_info_ptr);
	}

	png_destroy_write_struct(&apng_ptr, &apng_info_ptr);

	fclose(apng_FILE);
	apng_FILE = NULL;
	apng_frames = 0;
	return true;
#else
	return false;
#endif
}

#endif

// ==========================================================================
//                            SCREEN SHOTS
// ==========================================================================
#ifdef USE_PNG
/** Writes a PNG file to disk.
  *
  * \param filename Filename to write to.
  * \param data     The image data.
  * \param width    Width of the picture.
  * \param height   Height of the picture.
  * \param palette  Palette of image data
  *  \note if palette is NULL, BGR888 format
  */
boolean M_SavePNG(const char *filename, void *data, int width, int height, const UINT8 *palette)
{
	png_structp png_ptr;
	png_infop png_info_ptr;
	PNG_CONST png_byte *PLTE = (const png_byte *)palette;
#ifdef PNG_SETJMP_SUPPORTED
#ifdef USE_FAR_KEYWORD
	jmp_buf jmpbuf;
#endif
#endif
	png_FILE_p png_FILE;

	png_FILE = fopen(filename,"wb");
	if (!png_FILE)
	{
		CONS_Printf("M_SavePNG: Error on opening %s for write\n", filename);
		return false;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
	 PNG_error, PNG_warn);
	if (!png_ptr)
	{
		CONS_Printf("M_SavePNG: Error on initialize libpng\n");
		fclose(png_FILE);
		remove(filename);
		return false;
	}

	png_info_ptr = png_create_info_struct(png_ptr);
	if (!png_info_ptr)
	{
		CONS_Printf("M_SavePNG: Error on allocate for libpng\n");
		png_destroy_write_struct(&png_ptr,  NULL);
		fclose(png_FILE);
		remove(filename);
		return false;
	}

#ifdef USE_FAR_KEYWORD
	if (setjmp(jmpbuf))
#else
	if (setjmp(png_jmpbuf(png_ptr)))
#endif
	{
		//CONS_Printf("libpng write error on %s\n", filename);
		png_destroy_write_struct(&png_ptr, &png_info_ptr);
		fclose(png_FILE);
		remove(filename);
		return false;
	}
#ifdef USE_FAR_KEYWORD
	png_memcpy(png_jmpbuf(png_ptr),jmpbuf, sizeof (jmp_buf));
#endif
	png_init_io(png_ptr, png_FILE);

#ifdef PNG_SET_USER_LIMITS_SUPPORTED
	png_set_user_limits(png_ptr, MAXVIDWIDTH, MAXVIDHEIGHT);
#endif

	//png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);

	if (cv_zlib_level.value != -1)
		png_set_compression_level(png_ptr, cv_zlib_level.value);
	if (cv_zlib_memory.value != -1)
		png_set_compression_mem_level(png_ptr, cv_zlib_memory.value);
	if (cv_zlib_strategy.value != -1)
		png_set_compression_strategy(png_ptr, cv_zlib_strategy.value);
	if (cv_zlib_window_bits.value != -1)
		png_set_compression_window_bits(png_ptr, cv_zlib_window_bits.value);

	M_PNGhdr(png_ptr, png_info_ptr, width, height, PLTE);

	M_PNGText(png_ptr, png_info_ptr, false);

	png_write_info(png_ptr, png_info_ptr);

	M_PNGImage(png_ptr, png_info_ptr, height, data);

	png_write_end(png_ptr, png_info_ptr);
	png_destroy_write_struct(&png_ptr, &png_info_ptr);

	fclose(png_FILE);
	return true;
}
#else
/** PCX file structure.
  */
typedef struct
{
	UINT8 manufacturer;
	UINT8 version;
	UINT8 encoding;
	UINT8 bits_per_pixel;

	UINT16 xmin, ymin;
	UINT16 xmax, ymax;
	UINT16 hres, vres;
	UINT8  palette[48];

	UINT8 reserved;
	UINT8 color_planes;
	UINT16 bytes_per_line;
	UINT16 palette_type;

	char filler[58];
	UINT8 data; ///< Unbounded; used for all picture data.
} pcx_t;

/** Writes a PCX file to disk.
  *
  * \param filename Filename to write to.
  * \param data     The image data.
  * \param width    Width of the picture.
  * \param height   Height of the picture.
  * \param palette  Palette of image data
  */
#if NUMSCREENS > 2
static boolean WritePCXfile(const char *filename, const UINT8 *data, int width, int height, const UINT8 *palette)
{
	int i;
	size_t length;
	pcx_t *pcx;
	UINT8 *pack;

	pcx = Z_Malloc(width*height*2 + 1000, PU_STATIC, NULL);

	pcx->manufacturer = 0x0a; // PCX id
	pcx->version = 5; // 256 color
	pcx->encoding = 1; // uncompressed
	pcx->bits_per_pixel = 8; // 256 color
	pcx->xmin = pcx->ymin = 0;
	pcx->xmax = SHORT(width - 1);
	pcx->ymax = SHORT(height - 1);
	pcx->hres = SHORT(width);
	pcx->vres = SHORT(height);
	memset(pcx->palette, 0, sizeof (pcx->palette));
	pcx->reserved = 0;
	pcx->color_planes = 1; // chunky image
	pcx->bytes_per_line = SHORT(width);
	pcx->palette_type = SHORT(1); // not a grey scale
	memset(pcx->filler, 0, sizeof (pcx->filler));

	// pack the image
	pack = &pcx->data;

	for (i = 0; i < width*height; i++)
	{
		if ((*data & 0xc0) != 0xc0)
			*pack++ = *data++;
		else
		{
			*pack++ = 0xc1;
			*pack++ = *data++;
		}
	}

	// write the palette
	*pack++ = 0x0c; // palette ID byte
	for (i = 0; i < 768; i++)
		*pack++ = *palette++;

	// write output file
	length = pack - (UINT8 *)pcx;
	i = FIL_WriteFile(filename, pcx, length);

	Z_Free(pcx);
	return i;
}
#endif
#endif

void M_ScreenShot(void)
{
	takescreenshot = true;
}

/** Takes a screenshot.
  * The screenshot is saved as "srb2xxxx.pcx" (or "srb2xxxx.tga" in hardware
  * rendermode) where xxxx is the lowest four-digit number for which a file
  * does not already exist.
  *
  * \sa HWR_ScreenShot
  */
void M_DoScreenShot(void)
{
#if NUMSCREENS > 2
	const char *freename = NULL, *pathname = ".";
	boolean ret = false;
	UINT8 *linear = NULL;

	// Don't take multiple screenshots, obviously
	takescreenshot = false;

	if (cv_screenshot_option.value == 0)
		pathname = usehome ? srb2home : srb2path;
	else if (cv_screenshot_option.value == 1)
		pathname = srb2home;
	else if (cv_screenshot_option.value == 2)
		pathname = srb2path;
	else if (cv_screenshot_option.value == 3 && *cv_screenshot_folder.string != '\0')
		pathname = cv_screenshot_folder.string;

#ifdef USE_PNG
	if (rendermode != render_none)
		freename = Newsnapshotfile(pathname,"png");
#else
	if (rendermode == render_soft)
		freename = Newsnapshotfile(pathname,"pcx");
	else if (rendermode != render_none)
		freename = Newsnapshotfile(pathname,"tga");
#endif
	else
		I_Error("Can't take a screenshot without a render system");

	if (rendermode == render_soft)
	{
		// munge planar buffer to linear
		linear = screens[2];
		I_ReadScreen(linear);
	}

	if (!freename)
		goto failure;

	// save the pcx file
#ifdef HWRENDER
	if (rendermode != render_soft)
		ret = HWR_Screenshot(va(pandf,pathname,freename));
	else
#endif
	if (rendermode != render_none)
	{
#ifdef USE_PNG
		ret = M_SavePNG(va(pandf,pathname,freename), linear, vid.width, vid.height,
			W_CacheLumpName(GetPalette(), PU_CACHE));
#else
		ret = WritePCXfile(va(pandf,pathname,freename), linear, vid.width, vid.height,
			W_CacheLumpName(GetPalette(), PU_CACHE));
#endif
	}

failure:
	if (ret)
	{
		if (!moviemode)
			CONS_Printf("screen shot %s saved in %s\n", freename, pathname);
	}
	else
	{
		if (freename)
			CONS_Printf("Couldn't create screen shot %s in %s\n", freename, pathname);
		else
			CONS_Printf("Couldn't create screen shot (all 10000 slots used!) in %s\n", pathname);
	}
#endif
}

// ==========================================================================
//                        MISC STRING FUNCTIONS
// ==========================================================================

/** Returns a temporary string made out of varargs.
  * For use with CONS_Printf().
  *
  * \param format Format string.
  * \return Pointer to a static buffer of 1024 characters, containing the
  *         resulting string.
  */
char *va(const char *format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

/** Creates a string in the first argument that is the second argument followed
  * by the third argument followed by the first argument.
  * Useful for making filenames with full path. s1 = s2+s3+s1
  *
  * \param s1 First string, suffix, and destination.
  * \param s2 Second string. Ends up first in the result.
  * \param s3 Third string. Ends up second in the result.
  */
void strcatbf(char *s1, const char *s2, const char *s3)
{
	char tmp[1024];

	strcpy(tmp, s1);
	strcpy(s1, s2);
	strcat(s1, s3);
	strcat(s1, tmp);
}

#if defined (__GNUC__) && defined (__i386__) // from libkwave, under GPL
// Alam: note libkwave memcpy code comes from mplayer's libvo/aclib_template.c, r699

/* for small memory blocks (<256 bytes) this version is faster */
#define small_memcpy(dest,src,n)\
{\
register unsigned long int dummy;\
__asm__ __volatile__(\
	"cld\n\t"\
	"rep; movsb"\
	:"=&D"(dest), "=&S"(src), "=&c"(dummy)\
	:"0" (dest), "1" (src),"2" (n)\
	: "memory", "cc");\
}
/* linux kernel __memcpy (from: /include/asm/string.h) */
ATTRINLINE static FUNCINLINE void *__memcpy (void *dest, const void * src, size_t n)
{
	int d0, d1, d2;

	if ( n < 4 )
	{
		small_memcpy(dest, src, n);
	}
	else
	{
		__asm__ __volatile__ (
			"rep ; movsl;"
			"testb $2,%b4;"
			"je 1f;"
			"movsw;"
			"1:\ttestb $1,%b4;"
			"je 2f;"
			"movsb;"
			"2:"
		: "=&c" (d0), "=&D" (d1), "=&S" (d2)
		:"0" (n/4), "q" (n),"1" ((long) dest),"2" ((long) src)
		: "memory");
	}

	return dest;
}

#define SSE_MMREG_SIZE 16
#define MMX_MMREG_SIZE 8

#define MMX1_MIN_LEN 0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */

/* SSE note: i tried to move 128 bytes a time instead of 64 but it
didn't make any measureable difference. i'm using 64 for the sake of
simplicity. [MF] */
static /*FUNCTARGET("sse2")*/ void *sse_cpy(void * dest, const void * src, size_t n)
{
	void *retval = dest;
	size_t i;

	/* PREFETCH has effect even for MOVSB instruction ;) */
	__asm__ __volatile__ (
		"prefetchnta (%0);"
		"prefetchnta 32(%0);"
		"prefetchnta 64(%0);"
		"prefetchnta 96(%0);"
		"prefetchnta 128(%0);"
		"prefetchnta 160(%0);"
		"prefetchnta 192(%0);"
		"prefetchnta 224(%0);"
		"prefetchnta 256(%0);"
		"prefetchnta 288(%0);"
		: : "r" (src) );

	if (n >= MIN_LEN)
	{
		register unsigned long int delta;
		/* Align destinition to MMREG_SIZE -boundary */
		delta = ((unsigned long int)dest)&(SSE_MMREG_SIZE-1);
		if (delta)
		{
			delta=SSE_MMREG_SIZE-delta;
			n -= delta;
			small_memcpy(dest, src, delta);
		}
		i = n >> 6; /* n/64 */
		n&=63;
		if (((unsigned long)src) & 15)
		/* if SRC is misaligned */
		 for (; i>0; i--)
		 {
			__asm__ __volatile__ (
				"prefetchnta 320(%0);"
				"prefetchnta 352(%0);"
				"movups (%0), %%xmm0;"
				"movups 16(%0), %%xmm1;"
				"movups 32(%0), %%xmm2;"
				"movups 48(%0), %%xmm3;"
				"movntps %%xmm0, (%1);"
				"movntps %%xmm1, 16(%1);"
				"movntps %%xmm2, 32(%1);"
				"movntps %%xmm3, 48(%1);"
			:: "r" (src), "r" (dest) : "memory");
			src = (const unsigned char *)src + 64;
			dest = (unsigned char *)dest + 64;
		}
		else
			/*
			   Only if SRC is aligned on 16-byte boundary.
			   It allows to use movaps instead of movups, which required data
			   to be aligned or a general-protection exception (#GP) is generated.
			*/
		 for (; i>0; i--)
		 {
			__asm__ __volatile__ (
				"prefetchnta 320(%0);"
				"prefetchnta 352(%0);"
				"movaps (%0), %%xmm0;"
				"movaps 16(%0), %%xmm1;"
				"movaps 32(%0), %%xmm2;"
				"movaps 48(%0), %%xmm3;"
				"movntps %%xmm0, (%1);"
				"movntps %%xmm1, 16(%1);"
				"movntps %%xmm2, 32(%1);"
				"movntps %%xmm3, 48(%1);"
			:: "r" (src), "r" (dest) : "memory");
			src = ((const unsigned char *)src) + 64;
			dest = ((unsigned char *)dest) + 64;
		}
		/* since movntq is weakly-ordered, a "sfence"
		 * is needed to become ordered again. */
		__asm__ __volatile__ ("sfence":::"memory");
		/* enables to use FPU */
		__asm__ __volatile__ ("emms":::"memory");
	}
	/*
	 *	Now do the tail of the block
	 */
	if (n) __memcpy(dest, src, n);
	return retval;
}

static FUNCTARGET("mmx") void *mmx2_cpy(void *dest, const void *src, size_t n)
{
	void *retval = dest;
	size_t i;

	/* PREFETCH has effect even for MOVSB instruction ;) */
	__asm__ __volatile__ (
		"prefetchnta (%0);"
		"prefetchnta 32(%0);"
		"prefetchnta 64(%0);"
		"prefetchnta 96(%0);"
		"prefetchnta 128(%0);"
		"prefetchnta 160(%0);"
		"prefetchnta 192(%0);"
		"prefetchnta 224(%0);"
		"prefetchnta 256(%0);"
		"prefetchnta 288(%0);"
	: : "r" (src));

	if (n >= MIN_LEN)
	{
		register unsigned long int delta;
		/* Align destinition to MMREG_SIZE -boundary */
		delta = ((unsigned long int)dest)&(MMX_MMREG_SIZE-1);
		if (delta)
		{
			delta=MMX_MMREG_SIZE-delta;
			n -= delta;
			small_memcpy(dest, src, delta);
		}
		i = n >> 6; /* n/64 */
		n&=63;
		for (; i>0; i--)
		{
			__asm__ __volatile__ (
				"prefetchnta 320(%0);"
				"prefetchnta 352(%0);"
				"movq (%0), %%mm0;"
				"movq 8(%0), %%mm1;"
				"movq 16(%0), %%mm2;"
				"movq 24(%0), %%mm3;"
				"movq 32(%0), %%mm4;"
				"movq 40(%0), %%mm5;"
				"movq 48(%0), %%mm6;"
				"movq 56(%0), %%mm7;"
				"movntq %%mm0, (%1);"
				"movntq %%mm1, 8(%1);"
				"movntq %%mm2, 16(%1);"
				"movntq %%mm3, 24(%1);"
				"movntq %%mm4, 32(%1);"
				"movntq %%mm5, 40(%1);"
				"movntq %%mm6, 48(%1);"
				"movntq %%mm7, 56(%1);"
			:: "r" (src), "r" (dest) : "memory");
			src = ((const unsigned char *)src) + 64;
			dest = ((unsigned char *)dest) + 64;
		}
		/* since movntq is weakly-ordered, a "sfence"
		* is needed to become ordered again. */
		__asm__ __volatile__ ("sfence":::"memory");
		__asm__ __volatile__ ("emms":::"memory");
	}
	/*
	 *	Now do the tail of the block
	 */
	if (n) __memcpy(dest, src, n);
	return retval;
}

static FUNCTARGET("mmx") void *mmx1_cpy(void *dest, const void *src, size_t n) //3DNOW
{
	void *retval = dest;
	size_t i;

	/* PREFETCH has effect even for MOVSB instruction ;) */
	__asm__ __volatile__ (
		"prefetch (%0);"
		"prefetch 32(%0);"
		"prefetch 64(%0);"
		"prefetch 96(%0);"
		"prefetch 128(%0);"
		"prefetch 160(%0);"
		"prefetch 192(%0);"
		"prefetch 224(%0);"
		"prefetch 256(%0);"
		"prefetch 288(%0);"
	: : "r" (src));

	if (n >= MMX1_MIN_LEN)
	{
		register unsigned long int delta;
		/* Align destinition to MMREG_SIZE -boundary */
		delta = ((unsigned long int)dest)&(MMX_MMREG_SIZE-1);
		if (delta)
		{
			delta=MMX_MMREG_SIZE-delta;
			n -= delta;
			small_memcpy(dest, src, delta);
		}
		i = n >> 6; /* n/64 */
		n&=63;
		for (; i>0; i--)
		{
			__asm__ __volatile__ (
				"prefetch 320(%0);"
				"prefetch 352(%0);"
				"movq (%0), %%mm0;"
				"movq 8(%0), %%mm1;"
				"movq 16(%0), %%mm2;"
				"movq 24(%0), %%mm3;"
				"movq 32(%0), %%mm4;"
				"movq 40(%0), %%mm5;"
				"movq 48(%0), %%mm6;"
				"movq 56(%0), %%mm7;"
				"movq %%mm0, (%1);"
				"movq %%mm1, 8(%1);"
				"movq %%mm2, 16(%1);"
				"movq %%mm3, 24(%1);"
				"movq %%mm4, 32(%1);"
				"movq %%mm5, 40(%1);"
				"movq %%mm6, 48(%1);"
				"movq %%mm7, 56(%1);"
			:: "r" (src), "r" (dest) : "memory");
			src = ((const unsigned char *)src) + 64;
			dest = ((unsigned char *)dest) + 64;
		}
		__asm__ __volatile__ ("femms":::"memory"); // same as mmx_cpy() but with a femms
	}
	/*
	 *	Now do the tail of the block
	 */
	if (n) __memcpy(dest, src, n);
	return retval;
}
#endif

// Alam: why? memcpy may be __cdecl/_System and our code may be not the same type
static void *cpu_cpy(void *dest, const void *src, size_t n)
{
	if(src == NULL)
	{
		I_OutputMsg("Memcpy from 0x0?!: %p %p %"PRIdS"\n", dest, src, n);
		return dest;
	}

	if(dest == NULL)
	{
		I_OutputMsg("Memcpy to 0x0?!: %p %p %"PRIdS"\n", dest, src, n);
		return dest;
	}

	return memcpy(dest, src, n);
}

static /*FUNCTARGET("mmx")*/ void *mmx_cpy(void *dest, const void *src, size_t n)
{
#if defined (_MSC_VER) && defined (_X86_)
	_asm
	{
		mov ecx, [n]
		mov esi, [src]
		mov edi, [dest]
		shr ecx, 6 // mit mmx: 64bytes per iteration
		jz lower_64 // if lower than 64 bytes
		loop_64: // MMX transfers multiples of 64bytes
		movq mm0,  0[ESI] // read sources
		movq mm1,  8[ESI]
		movq mm2, 16[ESI]
		movq mm3, 24[ESI]
		movq mm4, 32[ESI]
		movq mm5, 40[ESI]
		movq mm6, 48[ESI]
		movq mm7, 56[ESI]

		movq  0[EDI], mm0 // write destination
		movq  8[EDI], mm1
		movq 16[EDI], mm2
		movq 24[EDI], mm3
		movq 32[EDI], mm4
		movq 40[EDI], mm5
		movq 48[EDI], mm6
		movq 56[EDI], mm7

		add esi, 64
		add edi, 64
		dec ecx
		jnz loop_64
		emms // close mmx operation
		lower_64:// transfer rest of buffer
		mov ebx,esi
		sub ebx,src
		mov ecx,[n]
		sub ecx,ebx
		shr ecx, 3 // multiples of 8 bytes
		jz lower_8
		loop_8:
		movq  mm0, [esi] // read source
		movq [edi], mm0 // write destination
		add esi, 8
		add edi, 8
		dec ecx
		jnz loop_8
		emms // close mmx operation
		lower_8:
		mov ebx,esi
		sub ebx,src
		mov ecx,[n]
		sub ecx,ebx
		rep movsb
		mov eax, [dest] // return dest
	}
#elif defined (__GNUC__) && defined (__i386__)
	void *retval = dest;
	size_t i;

	if (n >= MMX1_MIN_LEN)
	{
		register unsigned long int delta;
		/* Align destinition to MMREG_SIZE -boundary */
		delta = ((unsigned long int)dest)&(MMX_MMREG_SIZE-1);
		if (delta)
		{
			delta=MMX_MMREG_SIZE-delta;
			n -= delta;
			small_memcpy(dest, src, delta);
		}
		i = n >> 6; /* n/64 */
		n&=63;
		for (; i>0; i--)
		{
			__asm__ __volatile__ (
				"movq (%0), %%mm0;"
				"movq 8(%0), %%mm1;"
				"movq 16(%0), %%mm2;"
				"movq 24(%0), %%mm3;"
				"movq 32(%0), %%mm4;"
				"movq 40(%0), %%mm5;"
				"movq 48(%0), %%mm6;"
				"movq 56(%0), %%mm7;"
				"movq %%mm0, (%1);"
				"movq %%mm1, 8(%1);"
				"movq %%mm2, 16(%1);"
				"movq %%mm3, 24(%1);"
				"movq %%mm4, 32(%1);"
				"movq %%mm5, 40(%1);"
				"movq %%mm6, 48(%1);"
				"movq %%mm7, 56(%1);"
			:: "r" (src), "r" (dest) : "memory");
			src = ((const unsigned char *)src) + 64;
			dest = ((unsigned char *)dest) + 64;
		}
		__asm__ __volatile__ ("emms":::"memory");
	}
	/*
	 *	Now do the tail of the block
	 */
	if (n) __memcpy(dest, src, n);
	return retval;
#else
	return cpu_cpy(dest, src, n);
#endif
}

void *(*M_Memcpy)(void* dest, const void* src, size_t n) = cpu_cpy;

/** Memcpy that uses MMX, 3DNow, MMXExt or even SSE
  * Do not use on overlapped memory, use memmove for that
  */
void M_SetupMemcpy(void)
{
#if defined (__GNUC__) && defined (__i386__)
	if (R_SSE2)
		M_Memcpy = sse_cpy;
	else if (R_MMXExt)
		M_Memcpy = mmx2_cpy;
	else if (R_3DNow)
		M_Memcpy = mmx1_cpy;
	else
#endif
	if (R_MMX)
		M_Memcpy = mmx_cpy;
#if 0
	M_Memcpy = cpu_cpy;
#endif
}
