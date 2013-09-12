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
/// \brief System-independent sound and music routines

#ifdef MUSSERV
#include <sys/msg.h>
struct musmsg
{
	long msg_type;
	char msg_text[12];
};
extern INT32 msg_id;
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "g_game.h"
#include "m_argv.h"
#include "r_main.h" // R_PointToAngle2() used to calc stereo sep.
#include "r_things.h" // for skins
#include "i_system.h"
#include "i_sound.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_main.h"
#include "r_sky.h" // skyflatnum
#include "p_local.h" // camera info

#ifdef HW3SOUND
// 3D Sound Interface
#include "hardware/hw3sound.h"
#else
static int S_AdjustSoundParams(const mobj_t *listener, const mobj_t *source, int *vol, int *sep, int *pitch, sfxinfo_t *sfxinfo);
#endif

CV_PossibleValue_t soundvolume_cons_t[] = {{0, "MIN"}, {31, "MAX"}, {0, NULL}};
static void SetChannelsNum(void);

// commands for music and sound servers
#ifdef MUSSERV
consvar_t musserver_cmd = {"musserver_cmd", "musserver", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t musserver_arg = {"musserver_arg", "-t 20 -f -u 0 -i music.dta", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif
#ifdef SNDSERV
consvar_t sndserver_cmd = {"sndserver_cmd", "llsndserv", CV_SAVE, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t sndserver_arg = {"sndserver_arg", "-quiet", CV_SAVE, NULL, 0, NULL, NULL, 0, 0, NULL};
#endif

#if defined (_WINDOWS) && !defined (SURROUND) //&& defined (_X86_)
#define SURROUND
#endif

#if defined (_WIN32_WCE) || defined (DC) || defined(GP2X)
consvar_t cv_samplerate = {"samplerate", "11025", 0, CV_Unsigned, NULL, 11025, NULL, NULL, 0, 0, NULL}; //Alam: For easy hacking?
#elif defined(_PSP)
consvar_t cv_samplerate = {"samplerate", "44100", 0, CV_Unsigned, NULL, 44100, NULL, NULL, 0, 0, NULL}; //Alam: For easy hacking?
#else
consvar_t cv_samplerate = {"samplerate", "22050", 0, CV_Unsigned, NULL, 22050, NULL, NULL, 0, 0, NULL}; //Alam: For easy hacking?
#endif

// stereo reverse
consvar_t stereoreverse = {"stereoreverse", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// if true, all sounds are loaded at game startup
static consvar_t precachesound = {"precachesound", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// actual general (maximum) sound & music volume, saved into the config
consvar_t cv_soundvolume = {"soundvolume", "15", CV_SAVE, soundvolume_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_digmusicvolume = {"digmusicvolume", "31", CV_SAVE, soundvolume_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_midimusicvolume = {"midimusicvolume", "31", CV_SAVE, soundvolume_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
// number of channels available
#if defined (_WIN32_WCE) || defined (DC) || defined (PSP) || defined(GP2X)
consvar_t cv_numChannels = {"snd_channels", "8", CV_SAVE|CV_CALL, CV_Unsigned, SetChannelsNum, 0, NULL, NULL, 0, 0, NULL};
#else
consvar_t cv_numChannels = {"snd_channels", "32", CV_SAVE|CV_CALL, CV_Unsigned, SetChannelsNum, 0, NULL, NULL, 0, 0, NULL};
#endif

static consvar_t surround = {"surround", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

#define S_MAX_VOLUME 127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (before (1200*0x10000))
#define S_CLIPPING_DIST (1536*0x10000)

// Distance to origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (before (160*0x10000))
#define S_CLOSE_DIST (160*0x10000)

// added 2-2-98 in 8 bit volume control (before remove the +4)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>(FRACBITS+4))

// Adjustable by menu.
#define NORM_VOLUME snd_MaxVolume

#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128

#define S_PITCH_PERTURB 1
#define S_STEREO_SWING (96*0x10000)

#ifdef SURROUND
#define SURROUND_SEP -128
#endif

// percent attenuation from front to back
#define S_IFRACVOL 30

typedef struct
{
	// sound information (if null, channel avail.)
	sfxinfo_t *sfxinfo;

	// origin of sound
	const void *origin;

	// handle of the sound being played
	INT32 handle;

} channel_t;

// the set of channels available
static channel_t *channels = NULL;
static INT32 numofchannels = 0;

// whether songs are mus_paused
static boolean mus_paused = 0;

// music currently being played
musicinfo_t *mus_playing = 0;

static INT32 nextcleanup;

//
// Internals.
//
static void S_StopChannel(INT32 cnum);

//
// S_getChannel
//
// If none available, return -1. Otherwise channel #.
//
static INT32 S_getChannel(const void *origin, sfxinfo_t *sfxinfo)
{
	// channel number to use
	INT32 cnum;

	channel_t *c;

	// Find an open channel
	for (cnum = 0; cnum < numofchannels; cnum++)
	{
		if (!channels[cnum].sfxinfo)
			break;

		// Now checks if same sound is being played, rather
		// than just one sound per mobj
		else if (sfxinfo == channels[cnum].sfxinfo && (sfxinfo->pitch & SF_NOMULTIPLESOUND))
		{
			return -1;
			break;
		}
		else if (sfxinfo == channels[cnum].sfxinfo && sfxinfo->singularity == true)
		{
			S_StopChannel(cnum);
			break;
		}
		else if (origin && channels[cnum].origin == origin && channels[cnum].sfxinfo == sfxinfo)
		{
			if (sfxinfo->pitch & SF_NOINTERRUPT)
				return -1;
			else
				S_StopChannel(cnum);
			break;
		}
		else if (origin && channels[cnum].origin == origin && ((channels[cnum].sfxinfo == sfxinfo)
			|| (channels[cnum].sfxinfo->name != sfxinfo->name
			&& channels[cnum].sfxinfo->pitch == 1 && sfxinfo->pitch == 1
			&& channels[cnum].sfxinfo->pitch == sfxinfo->pitch)))
		{
			S_StopChannel(cnum);
			break;
		}
	}

	// None available
	if (cnum == numofchannels)
	{
		// Look for lower priority
		for (cnum = 0; cnum < numofchannels; cnum++)
			if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
				break;

		if (cnum == numofchannels)
		{
			// FUCK! No lower priority. Sorry, Charlie.
			return -1;
		}
		else
		{
			// Otherwise, kick out lower priority.
			S_StopChannel(cnum);
		}
	}

	c = &channels[cnum];

	// channel is decided to be cnum.
	c->sfxinfo = sfxinfo;
	c->origin = origin;

	return cnum;
}

void S_RegisterSoundStuff(void)
{
	if (dedicated)
	{
		nosound = true;
		return;
	}

	CV_RegisterVar(&stereoreverse);
	CV_RegisterVar(&precachesound);

#ifdef SNDSERV
	CV_RegisterVar(&sndserver_cmd);
	CV_RegisterVar(&sndserver_arg);
#endif
#ifdef MUSSERV
	CV_RegisterVar(&musserver_cmd);
	CV_RegisterVar(&musserver_arg);
#endif
	CV_RegisterVar(&surround);
	CV_RegisterVar(&cv_samplerate);

#if defined (macintosh) && !defined (SDL) // mp3 playlist stuff
	{
		INT32 i;
		for (i = 0; i < PLAYLIST_LENGTH; i++)
		{
			user_songs[i].name = malloc(7);
			if (!user_songs[i].name)
				I_Error("No more free memory for mp3 playlist");
			sprintf(user_songs[i].name, "song%d%d",i/10,i%10);
			user_songs[i].defaultvalue = malloc(sizeof (char));
			if (user_songs[i].defaultvalue)
				I_Error("No more free memory for blank mp3 playerlist");
			*user_songs[i].defaultvalue = 0;
			user_songs[i].flags = CV_SAVE;
			user_songs[i].PossibleValue = NULL;
			CV_RegisterVar(&user_songs[i]);
		}
		CV_RegisterVar(&play_mode);
	}
#endif
}

static void SetChannelsNum(void)
{
	INT32 i;

	// Allocating the internal channels for mixing
	// (the maximum number of sounds rendered
	// simultaneously) within zone memory.
	if (channels)
		S_StopSounds();

	Z_Free(channels);
	channels = NULL;


	if (cv_numChannels.value == 999999999) //Alam_GBC: OH MY ROD!(ROD rimmiced with GOD!)
		CV_StealthSet(&cv_numChannels,cv_numChannels.defaultvalue);

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_SetSourcesNum();
		return;
	}
#endif
	if (cv_numChannels.value)
		channels = (channel_t *)Z_Malloc(cv_numChannels.value * sizeof (channel_t), PU_STATIC, NULL);
	numofchannels = cv_numChannels.value;

	// Free all channels for use
	for (i = 0; i < numofchannels; i++)
		channels[i].sfxinfo = 0;
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(INT32 sfxVolume, INT32 digMusicVolume, INT32 midiMusicVolume)
{
	INT32 i;

	if (dedicated)
		return;

	S_SetSfxVolume(sfxVolume);
	S_SetDigMusicVolume(digMusicVolume);
	S_SetMIDIMusicVolume(midiMusicVolume);

	SetChannelsNum();

	// no sounds are playing, and they are not mus_paused
	mus_paused = 0;

	// Note that sounds have not been cached (yet).
	for (i = 1; i < NUMSFX; i++)
	{
		S_sfx[i].usefulness = -1; // for I_GetSfx()
		S_sfx[i].lumpnum = LUMPERROR;
	}

	// precache sounds if requested by cmdline, or precachesound var true
	if (!nosound && (M_CheckParm("-precachesound") || precachesound.value))
	{
		// Initialize external data (all sounds) at start, keep static.
		CONS_Printf("Loading sounds... ");

		for (i = 1; i < NUMSFX; i++)
			if (S_sfx[i].name)
				S_sfx[i].data = I_GetSfx(&S_sfx[i]);

		CONS_Printf(" pre-cached all sound data\n");
	}
}

// Retrieve the lump number of sfx
//
lumpnum_t S_GetSfxLumpNum(sfxinfo_t *sfx)
{
	char namebuf[9];
	lumpnum_t sfxlump;

	sprintf(namebuf, "ds%s", sfx->name);

	sfxlump = W_CheckNumForName(namebuf);
	if (sfxlump != LUMPERROR)
		return sfxlump;

	strlcpy(namebuf, sfx->name, sizeof namebuf);

	sfxlump = W_CheckNumForName(namebuf);
	if (sfxlump != LUMPERROR)
		return sfxlump;

	return W_GetNumForName("dsthok");
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

// Stop all sounds, load level info, THEN start sounds.
void S_StopSounds(void)
{
	INT32 cnum;

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_StopSounds();
		return;
	}
#endif

	// kill all playing sounds at start of level
	for (cnum = 0; cnum < numofchannels; cnum++)
		if (channels[cnum].sfxinfo)
			S_StopChannel(cnum);
}

void S_StopSoundByNum(sfxenum_t sfxnum)
{
	INT32 cnum;

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_StopSoundByNum(sfxnum);
		return;
	}
#endif
	for (cnum = 0; cnum < numofchannels; cnum++)
	{
		if (channels[cnum].sfxinfo == &S_sfx[sfxnum])
		{
			S_StopChannel(cnum);
			break;
		}
	}
}

void S_Start(void)
{
	if (!(mapmusic & 2048))
		mapmusic = mapheaderinfo[gamemap-1].musicslot;

	mus_paused = 0;

	S_ChangeMusic(mapmusic & 2047, true);
	nextcleanup = 15;
}

typedef struct
{
	fixed_t x, y, z;
	angle_t angle;
} listener_t;

void S_StartSoundAtVolume(const void *origin_p, sfxenum_t sfx_id, INT32 volume)
{
	INT32 sep, pitch, priority, cnum;
	sfxinfo_t *sfx;

	const mobj_t *origin = (const mobj_t *)origin_p;

	listener_t listener  = {0,0,0,0};
	listener_t listener2 = {0,0,0,0};

	mobj_t *listenmobj = players[displayplayer].mo;
	mobj_t *listenmobj2 = NULL;

	if (sound_disabled || !sound_started)
		return;

	// In space, no one can hear you spin, unless you die.
	if (!splitscreen && players[displayplayer].powers[pw_spacetime] &&
		!(sfx_id == sfx_altdi1 || sfx_id == sfx_altdi2 || sfx_id == sfx_altdi3 || sfx_id == sfx_altdi4))

	if (oncontinuescreen)
		return;

	if (splitscreen) listenmobj2 = players[secondarydisplayplayer].mo;

	if (nosound)
		return;

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_StartSound(origin, sfx_id);
		return;
	};
#endif

	if (cv_chasecam.value)
	{
		listener.x = camera.x;
		listener.y = camera.y;
		listener.z = camera.z;
		listener.angle = camera.angle;
	}
	else if (listenmobj)
	{
		listener.x = listenmobj->x;
		listener.y = listenmobj->y;
		listener.z = listenmobj->z;
		listener.angle = listenmobj->angle;
	}
	else if (origin)
		return;

	if (listenmobj2)
	{
		if (cv_chasecam2.value)
		{
			listener2.x = camera2.x;
			listener2.y = camera2.y;
			listener2.z = camera2.z;
			listener2.angle = camera2.angle;
		}
		else
		{
			listener2.x = listenmobj2->x;
			listener2.y = listenmobj2->y;
			listener2.z = listenmobj2->z;
			listener2.angle = listenmobj2->angle;
		}
	}

	// Don't want a sound? Okay then...
	if (sfx_id == 0 || sfx_id == sfx_None)
		return;

	// check for bogus sound #
	I_Assert(sfx_id >= 1);
	I_Assert(sfx_id < NUMSFX);

	sfx = &S_sfx[sfx_id];

	if (sfx->skinsound != -1 && origin && origin->skin)
	{
		// redirect player sound to the sound in the skin table
		sfx_id = ((skin_t *)origin->skin)->soundsid[sfx->skinsound];
		sfx = &S_sfx[sfx_id];
	}

	// Initialize sound parameters
	pitch = NORM_PITCH;
	priority = NORM_PRIORITY;

	if (splitscreen && listenmobj2) // Copy the sound for the split player
	{
		// Check to see if it is audible, and if not, modify the params
		if (origin && origin != listenmobj2)
		{
			INT32 rc;
			rc = S_AdjustSoundParams(listenmobj2, origin, &volume, &sep, &pitch, sfx);

			if (!rc)
				goto dontplay; // Maybe the other player can hear it...

			if (origin->x == listener2.x && origin->y == listener2.y)
				sep = NORM_SEP;
		}
		else
			sep = NORM_SEP;

		// try to find a channel
		cnum = S_getChannel(origin, sfx);

		if (cnum < 0)
			return; // If there's no free channels, it's not gonna be free for player 1, either.

		// This is supposed to handle the loading/caching.
		// For some odd reason, the caching is done nearly
		// each time the sound is needed?

		// cache data if necessary
		// NOTE: set sfx->data NULL sfx->lump -1 to force a reload
		if (!sfx->data)
			sfx->data = I_GetSfx(sfx);

		// increase the usefulness
		if (sfx->usefulness++ < 0)
			sfx->usefulness = -1;

#ifdef SURROUND
		// Avoid channel reverse if surround
		if (stereoreverse.value && sep != SURROUND_SEP)
			sep = (~sep) & 255;
#else
		if (stereoreverse.value)
			sep = (~sep) & 255;
#endif

		// Assigns the handle to one of the channels in the
		// mix/output buffer.
		channels[cnum].handle = I_StartSound(sfx_id, volume, sep, pitch, priority);
	}

dontplay:

	// Check to see if it is audible, and if not, modify the params
	if (origin && origin != listenmobj)
	{
		INT32 rc;
		rc = S_AdjustSoundParams(listenmobj, origin, &volume, &sep, &pitch, sfx);

		if (!rc)
			return;

		if (origin->x == listener.x && origin->y == listener.y)
			sep = NORM_SEP;
	}
	else
		sep = NORM_SEP;

	// try to find a channel
	cnum = S_getChannel(origin, sfx);

	if (cnum < 0)
		return;

	// This is supposed to handle the loading/caching.
	// For some odd reason, the caching is done nearly
	// each time the sound is needed?

	// cache data if necessary
	// NOTE: set sfx->data NULL sfx->lump -1 to force a reload
	if (!sfx->data)
		sfx->data = I_GetSfx(sfx);

	// increase the usefulness
	if (sfx->usefulness++ < 0)
		sfx->usefulness = -1;

#ifdef SURROUND
	// Avoid channel reverse if surround
	if (stereoreverse.value && sep != SURROUND_SEP)
		sep = (~sep) & 255;
#else
	if (stereoreverse.value)
		sep = (~sep) & 255;
#endif

	// Assigns the handle to one of the channels in the
	// mix/output buffer.
	channels[cnum].handle = I_StartSound(sfx_id, volume, sep, pitch, priority);
}

void S_StartSound(const void *origin, sfxenum_t sfx_id)
{
	if (sound_disabled)
		return;

	if (mariomode) // Sounds change in Mario mode!
	{
		switch (sfx_id)
		{
			case sfx_altow1:
			case sfx_altow2:
			case sfx_altow3:
			case sfx_altow4:
				sfx_id = sfx_mario8;
				break;
			case sfx_thok:
				sfx_id = sfx_mario7;
				break;
			case sfx_pop:
				sfx_id = sfx_mario5;
				break;
			case sfx_jump:
				sfx_id = sfx_mario6;
				break;
			case sfx_shield:
				sfx_id = sfx_mario3;
				break;
			case sfx_itemup:
				sfx_id = sfx_mario4;
				break;
			case sfx_tink:
				sfx_id = sfx_mario1;
				break;
			case sfx_cgot:
				sfx_id = sfx_mario9;
				break;
			case sfx_lose:
				sfx_id = sfx_mario2;
				break;
			default:
				break;
		}

	}

	// the volume is handled 8 bits
#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
		HW3S_StartSound(origin, sfx_id);
	else
#endif
		S_StartSoundAtVolume(origin, sfx_id, 255);
}

void S_StopSound(void *origin)
{
	INT32 cnum;

	// Sounds without origin can have multiple sources, they shouldn't
	// be stopped by new sounds.
	if (!origin)
		return;

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_StopSound(origin);
		return;
	}
#endif
	for (cnum = 0; cnum < numofchannels; cnum++)
	{
		if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
		{
			S_StopChannel(cnum);
			break;
		}
	}
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
	if (!nodigimusic)
		I_PauseSong(0);

	if (mus_playing && !mus_paused)
	{
		I_PauseSong(mus_playing->handle);
		mus_paused = true;
	}

	// pause cd music
#if defined (__unix__) || defined (UNIXCOMMON) || defined (SDL)
	I_PauseCD();
#else
	I_StopCD();
#endif
}

void S_ResumeSound(void)
{
	if (!nodigimusic)
		I_ResumeSong(0);
	else
	if (mus_playing && mus_paused)
	{
		I_ResumeSong(mus_playing->handle);
		mus_paused = false;
	}

	// resume cd music
	I_ResumeCD();
}

//
// Updates music & sounds
//
static INT32 actualsfxvolume; // check for change through console
static INT32 actualdigmusicvolume;
static INT32 actualmidimusicvolume;

void S_UpdateSounds(void)
{
	INT32 audible, cnum, volume, sep, pitch;
	sfxinfo_t *sfx;
	channel_t *c;
	MumblePos_t MPos;

	listener_t listener;
	listener_t listener2;

	mobj_t *listenmobj = players[displayplayer].mo;
	mobj_t *listenmobj2 = NULL;
	if (splitscreen) listenmobj2 = players[secondarydisplayplayer].mo;

	memset(&listener, 0, sizeof(listener_t));
	memset(&listener2, 0, sizeof(listener_t));

	// Update sound/music volumes, if changed manually at console
	if (actualsfxvolume != cv_soundvolume.value)
		S_SetSfxVolume (cv_soundvolume.value);
	if (actualdigmusicvolume != cv_digmusicvolume.value)
		S_SetDigMusicVolume (cv_digmusicvolume.value);
	if (actualmidimusicvolume != cv_midimusicvolume.value)
		S_SetMIDIMusicVolume (cv_midimusicvolume.value);

	if (gamestate != GS_LEVEL) return;		// We're done now, if we're not in a level.

	if (dedicated || nosound)
		return;

	if (cv_chasecam.value)
	{
		listener.x = camera.x;
		listener.y = camera.y;
		listener.z = camera.z;
		listener.angle = camera.angle;
	}
	else if (listenmobj)
	{
		listener.x = listenmobj->x;
		listener.y = listenmobj->y;
		listener.z = listenmobj->z;
		listener.angle = listenmobj->angle;
	}

	MPos.fPosition[0] = listener.x;
	MPos.fPosition[1] = listener.y;
	MPos.fPosition[2] = listener.z;
	if (listener.angle == 0)
		MPos.fFront = (double)((listener.angle*45.0l)/(double)ANGLE_45);
	else
		MPos.fFront = 0.0l;
	MPos.fTop = 0.0l;
	I_UpdateMumble(&MPos);

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
	{
		HW3S_UpdateSources();
		return;
	}
#endif

	if (listenmobj2)
	{
		if (cv_chasecam2.value)
		{
			listener2.x = camera2.x;
			listener2.y = camera2.y;
			listener2.z = camera2.z;
			listener2.angle = camera2.angle;
		}
		else
		{
			listener2.x = listenmobj2->x;
			listener2.y = listenmobj2->y;
			listener2.z = listenmobj2->z;
			listener2.angle = listenmobj2->angle;
		}
	}

	// Clean up unused data.
#if 0
	{
		static tic_t nextcleanup = 0;
		size_t i;
		if (!gametic) nextcleanup = 0;
		if (gametic > nextcleanup)
		{
			for (i = 1; i < NUMSFX; i++)
			{
				if (S_sfx[i].usefulness == 0)
				{
					S_sfx[i].usefulness--;

					// don't forget to unlock it !!!
					// __dmpi_unlock_....
					//Z_ChangeTag(S_sfx[i].data, PU_CACHE);
					I_FreeSfx(S_sfx+i);
					//S_sfx[i].data = 0;

					CONS_Printf("\2flushed sfx %.6s\n", S_sfx[i].name);
				}
			}
			nextcleanup = gametic + 15;
		}
	}
#endif

	// FIXTHIS: nextcleanup is probably unused

	for (cnum = 0; cnum < numofchannels; cnum++)
	{
		c = &channels[cnum];
		sfx = c->sfxinfo;

		if (c->sfxinfo)
		{
			if (I_SoundIsPlaying(c->handle))
			{
				// initialize parameters
				volume = 255; // 8 bits internal volume precision
				pitch = NORM_PITCH;
				sep = NORM_SEP;

				// check non-local sounds for distance clipping
				//  or modify their params
				if (c->origin && ((c->origin != players[consoleplayer].mo) ||
					(splitscreen && c->origin != players[secondarydisplayplayer].mo)))
				{
					// Whomever is closer gets the sound, but only in splitscreen.
					if (listenmobj && listenmobj2 && splitscreen)
					{
						const mobj_t *soundmobj = c->origin;

						fixed_t dist1, dist2;
						dist1 = P_AproxDistance(listener.x-soundmobj->x, listener.y-soundmobj->y);
						dist2 = P_AproxDistance(listener2.x-soundmobj->x, listener2.y-soundmobj->y);

						if (dist1 <= dist2)
						{
							// Player 1 gets the sound
							audible = S_AdjustSoundParams(listenmobj, c->origin, &volume, &sep, &pitch,
								c->sfxinfo);
						}
						else
						{
							// Player 2 gets the sound
							audible = S_AdjustSoundParams(listenmobj2, c->origin, &volume, &sep, &pitch,
								c->sfxinfo);
						}

						if (audible)
							I_UpdateSoundParams(c->handle, volume, sep, pitch);
						else
							S_StopChannel(cnum);
					}
					else if (listenmobj && !splitscreen)
					{
						// In the case of a single player, he or she always should get updated sound.
						audible = S_AdjustSoundParams(listenmobj, c->origin, &volume, &sep, &pitch,
							c->sfxinfo);

						if (audible)
							I_UpdateSoundParams(c->handle, volume, sep, pitch);
						else
							S_StopChannel(cnum);
					}
				}
			}
			else
			{
				// if channel is allocated but sound has stopped, free it
				S_StopChannel(cnum);
			}
		}
	}
}

void S_SetDigMusicVolume(INT32 volume)
{
	if (volume < 0 || volume > 31)
		CONS_Printf("musicvolume should be between 0-31\n");

	CV_SetValue(&cv_digmusicvolume, volume&31);
	actualdigmusicvolume = cv_digmusicvolume.value;   //check for change of var

#ifdef DJGPPDOS
	I_SetDigMusicVolume(31); // Trick for buggy dos drivers. Win32 doesn't need this.
#endif

	if (!nodigimusic)
		I_SetDigMusicVolume(volume&31);
}

void S_SetMIDIMusicVolume(INT32 volume)
{
	if (volume < 0 || volume > 31)
		CONS_Printf("musicvolume should be between 0-31\n");

	CV_SetValue(&cv_midimusicvolume, volume&31);
	actualmidimusicvolume = cv_midimusicvolume.value;   //check for change of var

#ifdef DJGPPDOS
	I_SetMIDIMusicVolume(31); // Trick for buggy dos drivers. Win32 doesn't need this.
#endif

	I_SetMIDIMusicVolume(volume&31);
}

void S_SetSfxVolume(INT32 volume)
{
	if (volume < 0 || volume > 31)
		CONS_Printf("sfxvolume should be between 0-31\n");

	CV_SetValue(&cv_soundvolume, volume&31);
	actualsfxvolume = cv_soundvolume.value; // check for change of var

#ifdef HW3SOUND
	hws_mode == HWS_DEFAULT_MODE ? I_SetSfxVolume(volume&31) : HW3S_SetSfxVolume(volume&31);
#else
	// now hardware volume
	I_SetSfxVolume(volume&31);
#endif
}

static boolean S_MIDIMusic(musicinfo_t *music, INT32 looping)
{
	if (nomidimusic)
		return false;

	if (music_disabled)
		return false;

	// get lumpnum if neccessary
	if (!music->lumpnum)
	{
		if (W_CheckNumForName(va("d_%s", music->name)) == LUMPERROR)
			return false;
		music->lumpnum = W_GetNumForName(va("d_%s", music->name));
	}

	// load & register it
	music->data = W_CacheLumpNum(music->lumpnum, PU_MUSIC);
#if defined (macintosh) && !defined (SDL)
	music->handle = I_RegisterSong(music_num);
#else
	music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
#endif

#ifdef MUSSERV
	if (msg_id != -1)
	{
		struct musmsg msg_buffer;

		msg_buffer.msg_type = 6;
		memset(msg_buffer.msg_text, 0, sizeof (msg_buffer.msg_text));
		sprintf(msg_buffer.msg_text, "d_%s", music->name);
		msgsnd(msg_id, (struct msgbuf*)&msg_buffer, sizeof (msg_buffer.msg_text), IPC_NOWAIT);
	}
#endif

	// play it
	if (!I_PlaySong(music->handle, looping))
		return false;

	mus_playing = music;
	return true;
}

static boolean S_DigMusic(musicinfo_t *music, INT32 looping)
{
	if (nodigimusic)
		return false;

	if (digital_disabled)
		return false;

	if (!I_StartDigSong(music->name, looping))
		return false;

	mus_playing = music;
	return true;
}

void S_ChangeMusic(musicenum_t music_num, INT32 looping)
{
	musicinfo_t *music;

#if defined (DC) || defined (_WIN32_WCE) || defined (PSP) || defined(GP2X)
	S_ClearSfx();
#endif

	if (nomidimusic && nodigimusic)
		return;

	if (music_disabled && digital_disabled)
		return;

	// No Music
	if (music_num == mus_None)
	{
		S_StopMusic();
		return;
	}

	if (music_num >= NUMMUSIC)
	{
		I_Error("Bad music number %d\n", music_num);
		return;
	}
	else
		music = &S_music[music_num];

	if (mus_playing == music)
		return;

	if (!nomidimusic) // Make sure the MIDI is not playing
		S_StopMusic(); // shutdown old music

	if (!S_DigMusic(music, looping) && !S_MIDIMusic(music, looping) && (!nodigimusic || !nomidimusic || !digital_disabled || !music_disabled))
		CONS_Printf("ERROR: Music lump %.6s not found!\n", music->name);
}

boolean S_SpeedMusic(float speed)
{
	return I_SetSongSpeed(speed);
}

void S_StopMusic(void)
{
	if (!mus_playing)
		return;

	if (mus_paused)
		I_ResumeSong(mus_playing->handle);

	if (!nodigimusic)
		I_StopDigSong();

	S_SpeedMusic(1.0f);
	I_StopSong(mus_playing->handle);
	I_UnRegisterSong(mus_playing->handle);

#ifndef SDL //SDL uses RWOPS
	Z_ChangeTag(mus_playing->data, PU_CACHE);
#endif

	mus_playing->data = NULL;
	mus_playing = NULL;

}

void S_ClearSfx(void)
{
#ifndef DJGPPDOS
	size_t i;
	for (i = 1; i < NUMSFX; i++)
		I_FreeSfx(S_sfx + i);
#endif
}

static void S_StopChannel(INT32 cnum)
{
	INT32 i;
	channel_t *c = &channels[cnum];

	if (c->sfxinfo)
	{
		// stop the sound playing
		if (I_SoundIsPlaying(c->handle))
			I_StopSound(c->handle);

		// check to see
		//  if other channels are playing the sound
		for (i = 0; i < numofchannels; i++)
			if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
				break;

		// degrade usefulness of sound data
		c->sfxinfo->usefulness--;

		c->sfxinfo = 0;
	}
}

//
// S_CalculateSoundDistance
//
// Calculates the distance between two points for a sound.
// Clips the distance to prevent overflow.
//
fixed_t S_CalculateSoundDistance(fixed_t sx1, fixed_t sy1, fixed_t sz1, fixed_t sx2, fixed_t sy2, fixed_t sz2)
{
	fixed_t approx_dist, adx, ady;

	// calculate the distance to sound origin and clip it if necessary
	adx = abs((sx1>>FRACBITS) - (sx2>>FRACBITS));
	ady = abs((sy1>>FRACBITS) - (sy2>>FRACBITS));

	// From _GG1_ p.428. Approx. euclidian distance fast.
	// Take Z into account
	adx = adx + ady - ((adx < ady ? adx : ady)>>1);
	ady = abs((sz1>>FRACBITS) - (sz2>>FRACBITS));
	approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

	if (approx_dist >= FRACUNIT/2)
		approx_dist = FRACUNIT/2-1;

	approx_dist <<= FRACBITS;

	return approx_dist;
}

//
// Changes volume, stereo-separation, and pitch variables
// from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
INT32 S_AdjustSoundParams(const mobj_t *listener, const mobj_t *source, INT32 *vol, INT32 *sep, INT32 *pitch,
	sfxinfo_t *sfxinfo)
{
	fixed_t approx_dist;
	angle_t angle;

	listener_t listensource;

	pitch = NULL;
	if (!listener)
		return false;

	if (listener == players[displayplayer].mo && cv_chasecam.value)
	{
		listensource.x = camera.x;
		listensource.y = camera.y;
		listensource.z = camera.z;
		listensource.angle = camera.angle;
	}
	else if (splitscreen && listener == players[secondarydisplayplayer].mo && cv_chasecam2.value)
	{
		listensource.x = camera2.x;
		listensource.y = camera2.y;
		listensource.z = camera2.z;
		listensource.angle = camera2.angle;
	}
	else
	{
		listensource.x = listener->x;
		listensource.y = listener->y;
		listensource.z = listener->z;
		listensource.angle = listener->angle;
	}

	if (sfxinfo->pitch & SF_OUTSIDESOUND) // Rain special case
	{
		fixed_t x, y, yl, yh, xl, xh, closex, closey, newdist;

		if (R_PointInSubsector(listensource.x, listensource.y)->sector->ceilingpic == skyflatnum)
			approx_dist = 0;
		else
		{
			// Essentially check in a 1024 unit radius of the player for an outdoor area.
			yl = listensource.y - 1024*FRACUNIT;
			yh = listensource.y + 1024*FRACUNIT;
			xl = listensource.x - 1024*FRACUNIT;
			xh = listensource.x + 1024*FRACUNIT;
			closex = listensource.x + 2048*FRACUNIT;
			closey = listensource.y + 2048*FRACUNIT;
			approx_dist = 1024*FRACUNIT;
			for (y = yl; y <= yh; y += FRACUNIT*64)
				for (x = xl; x <= xh; x += FRACUNIT*64)
				{
					if (R_PointInSubsector(x, y)->sector->ceilingpic == skyflatnum)
					{
						// Found the outdoors!
						newdist = S_CalculateSoundDistance(listensource.x, listensource.y, 0, x, y, 0);
						if (newdist < approx_dist)
						{
							closex = x;
							closey = y;
							approx_dist = newdist;
						}
					}
				}
		}
	}
	else
	{
		approx_dist = S_CalculateSoundDistance(listensource.x, listensource.y, listensource.z,
												source->x, source->y, source->z);
	}

	// Taunts, deaths, etc, should all be heard louder.
	if (sfxinfo->pitch & SF_X8AWAYSOUND)
		approx_dist = FixedDiv(approx_dist,8*FRACUNIT);

	// Combine 8XAWAYSOUND with 4XAWAYSOUND and get.... 32XAWAYSOUND?
	if (sfxinfo->pitch & SF_X4AWAYSOUND)
		approx_dist = FixedDiv(approx_dist,4*FRACUNIT);

	if (sfxinfo->pitch & SF_X2AWAYSOUND)
		approx_dist = FixedDiv(approx_dist,2*FRACUNIT);

	if (approx_dist > S_CLIPPING_DIST)
		return 0;

	// angle of source to listener
	angle = R_PointToAngle2(listensource.x, listensource.y, source->x, source->y);

	if (angle > listensource.angle)
		angle = angle - listensource.angle;
	else
		angle = angle + (ANGLE_MAX - listensource.angle + 1);

#ifdef SURROUND
	// Produce a surround sound for angle from 105 till 255
	if (surround.value == 1 && (angle > ANG105 && angle < ANG255 ))
		*sep = SURROUND_SEP;
	else
#endif
	{
		angle >>= ANGLETOFINESHIFT;

		// stereo separation
		*sep = 128 - (FixedMul(S_STEREO_SWING, FINESINE(angle))>>FRACBITS);
	}

	// volume calculation
	if (approx_dist < S_CLOSE_DIST)
	{
		// SfxVolume is now hardware volume
		*vol = 255; // not snd_SfxVolume
	}
	else
	{
		// distance effect
		*vol = (15 * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS)) / S_ATTENUATOR;
	}

	return (*vol > 0);
}

// Searches through the channels and checks for origin or id.
// returns 0 of not found, returns 1 if found.
// if id == NUMSFX, then don't check it...
INT32 S_SoundPlaying(void *origin, sfxenum_t id)
{
	INT32 cnum;

#ifdef HW3SOUND
	if (hws_mode != HWS_DEFAULT_MODE)
		return HW3S_SoundPlaying(origin, id);
#endif

	for (cnum = 0; cnum < numofchannels; cnum++)
	{
		if (origin && channels[cnum].origin == origin)
			return 1;
		if (id != NUMSFX && (size_t)(channels[cnum].sfxinfo - S_sfx) == (size_t)id)
			return 1;
	}
	return 0;
}

//
// S_StartSoundName
// Starts a sound using the given name.
#define MAXNEWSOUNDS 10
static sfxenum_t newsounds[MAXNEWSOUNDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void S_StartSoundName(void *mo, const char *soundname)
{
	INT32 i, soundnum = 0;
	// Search existing sounds...
	for (i = sfx_None + 1; i < NUMSFX; i++)
	{
		if (!S_sfx[i].name)
			continue;
		if (!stricmp(S_sfx[i].name, soundname))
		{
			soundnum = i;
			break;
		}
	}

	if (!soundnum)
	{
		for (i = 0; i < MAXNEWSOUNDS; i++)
		{
			if (newsounds[i] == 0)
				break;
			if (!S_SoundPlaying(NULL, newsounds[i]))
			{
				S_RemoveSoundFx(newsounds[i]);
				break;
			}
		}

		if (i == MAXNEWSOUNDS)
		{
			CONS_Printf("Cannot load another extra sound!\n");
			return;
		}

		soundnum = S_AddSoundFx(soundname, false, -1, false);
		newsounds[i] = soundnum;
	}

	S_StartSound(mo, soundnum);
}
