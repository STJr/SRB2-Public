// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2001 by DooM Legacy Team.
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
//
//-----------------------------------------------------------------------------
/// \file
/// \brief 3D sound import/export prototypes for low-level hardware interface

#ifndef __HW_3DS_DRV_H__
#define __HW_3DS_DRV_H__

// Use standart hardware API
#include "hw_dll.h"
#include "hws_data.h"

#if defined (SDL) || !defined (HWD)
EXPORT void HWRAPI(Shutdown) (void);
#endif

// Use standart Init and Shutdown functions

EXPORT int    HWRAPI (Startup) (I_Error_t FatalErrorFunction, snddev_t *snd_dev);
EXPORT u_int  HWRAPI (AddSfx) (sfx_data_t *sfx);
EXPORT int    HWRAPI (AddSource) (source3D_data_t *src, u_int sfxhandle);
EXPORT int    HWRAPI (StartSource) (int handle);
EXPORT void   HWRAPI (StopSource) (int handle);
EXPORT int    HWRAPI (GetHW3DSVersion) (void);
EXPORT void   HWRAPI (BeginFrameUpdate) (void);
EXPORT void   HWRAPI (EndFrameUpdate) (void);
EXPORT int    HWRAPI (IsPlaying) (int handle);
EXPORT void   HWRAPI (UpdateListener) (listener_data_t *data, int num);
EXPORT void   HWRAPI (UpdateSourceParms) (int handle, int vol, int sep);
EXPORT void   HWRAPI (SetGlobalSfxVolume) (int volume);
EXPORT int    HWRAPI (SetCone) (int handle, cone_def_t *cone_def);
EXPORT void   HWRAPI (Update3DSource) (int handle, source3D_pos_t *data);
EXPORT int    HWRAPI (ReloadSource) (int handle, u_int sfxhandle);
EXPORT void   HWRAPI (KillSource) (int handle);
EXPORT void   HWRAPI (KillSfx) (u_int sfxhandle);
EXPORT void   HWRAPI (GetHW3DSTitle) (char *buf, size_t size);


#if !defined (_CREATE_DLL_)

struct hardware3ds_s
{
	Startup             pfnStartup;
	AddSfx              pfnAddSfx;
	AddSource           pfnAddSource;
	StartSource         pfnStartSource;
	StopSource          pfnStopSource;
	GetHW3DSVersion     pfnGetHW3DSVersion;
	BeginFrameUpdate    pfnBeginFrameUpdate;
	EndFrameUpdate      pfnEndFrameUpdate;
	IsPlaying           pfnIsPlaying;
	UpdateListener      pfnUpdateListener;
	UpdateSourceParms   pfnUpdateSourceParms;
	SetGlobalSfxVolume  pfnSetGlobalSfxVolume;
	SetCone             pfnSetCone;
	Update3DSource      pfnUpdate3DSource;
	ReloadSource        pfnReloadSource;
	KillSource          pfnKillSource;
	KillSfx             pfnKillSfx;
	Shutdown            pfnShutdown;
	GetHW3DSTitle       pfnGetHW3DSTitle;
};

extern struct hardware3ds_s hw3ds_driver;

#define HW3DS hw3ds_driver


#endif  // _CREATE_DLL_

#endif // __HW_3DS_DRV_H__
