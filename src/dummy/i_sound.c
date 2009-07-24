#include "../i_sound.h"

byte sound_started = 0;

void *I_GetSfx(sfxinfo_t *sfx)
{
	(void)sfx;
	return NULL;
}

void I_FreeSfx(sfxinfo_t *sfx)
{
	(void)sfx;
}

void I_StartupSound(void){}

void I_ShutdownSound(void){}

//
//  SFX I/O
//

int I_StartSound(sfxenum_t id, int vol, int sep, int pitch, int priority)
{
	(void)id;
	(void)vol;
	(void)sep;
	(void)pitch;
	(void)priority;
	return -1;
}

void I_StopSound(int handle)
{
	(void)handle;
}

int I_SoundIsPlaying(int handle)
{
	(void)handle;
	return false;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
	(void)handle;
	(void)vol;
	(void)sep;
	(void)pitch;
}

void I_SetSfxVolume(int volume)
{
	(void)volume;
}

//
//  MUSIC I/O
//
byte music_started = 0;

void I_InitMusic(void){}

void I_ShutdownMusic(void){}

void I_PauseSong(int handle)
{
	(void)handle;
}

void I_ResumeSong(int handle)
{
	(void)handle;
}

//
//  MIDI I/O
//

byte midimusic_started = 0;

void I_InitMIDIMusic(void){}

void I_ShutdownMIDIMusic(void){}

void I_SetMIDIMusicVolume(int volume)
{
	(void)volume;
}

int I_RegisterSong(void *data, size_t len)
{
	(void)data;
	(void)len;
	return -1;
}

boolean I_PlaySong(int handle, int looping)
{
	(void)handle;
	(void)looping;
	return false;
}

void I_StopSong(int handle)
{
	(void)handle;
}

void I_UnRegisterSong(int handle)
{
	(void)handle;
}

//
//  DIGMUSIC I/O
//

byte digmusic_started = 0;

void I_InitDigMusic(void){}

void I_ShutdownDigMusic(void){}

boolean I_StartDigSong(const char *musicname, int looping)
{
	(void)musicname;
	(void)looping;
	return false;
}

void I_StopDigSong(void){}

void I_SetDigMusicVolume(int volume)
{
	(void)volume;
}

boolean I_SetSongSpeed(float speed)
{
	(void)speed;
	return false;
}
