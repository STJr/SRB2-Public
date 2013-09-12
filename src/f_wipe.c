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
/// \brief Wipe screen special effect.

#include "i_video.h"
#include "v_video.h"
#include "r_draw.h" // transtable
#include "p_pspr.h" // tr_transxxx
#include "i_system.h"
#include "m_menu.h"
#include "f_finale.h"
#if defined (SHUFFLE) && defined (HWRENDER)
#include "hardware/hw_main.h"
#endif

#if NUMSCREENS < 3
#define NOWIPE // do not enable wipe image post processing for ARM, SH and MIPS CPUs
#endif

//--------------------------------------------------------------------------
//                        SCREEN WIPE PACKAGE
//--------------------------------------------------------------------------

boolean WipeInAction = false;
#ifndef NOWIPE

static UINT8 *wipe_scr_start; //screens 2
static UINT8 *wipe_scr_end; //screens 3
static UINT8 *wipe_scr; //screens 0

/**	\brief	start the wipe

	\param	width	width of wipe
	\param	height	height of wipe
	\param	ticks	ticks for wipe

	\return	unknown


*/
static inline INT32 F_InitWipe(INT32 width, INT32 height, tic_t ticks)
{
	if(rendermode != render_soft)
		return 0;
	(void)ticks;
	M_Memcpy(wipe_scr, wipe_scr_start, width*height*scr_bpp);
	return 0;
}

/**	\brief	wipe ticker

	\param	width	width of wipe
	\param	height	height of wipe
	\param	ticks	ticks for wipe

	\return	the change in wipe


*/
static INT32 F_DoWipe(INT32 width, INT32 height, tic_t ticks)
{
	boolean changed = false;
	UINT8 *w;
	UINT8 *e;
	UINT8 newval;
	static INT32 slowdown = 0;

	while (ticks--)
	{
		// slowdown
		if (slowdown++)
		{
			slowdown = 0;
			return false;
		}

#ifdef SHUFFLE
		if(rendermode != render_soft)
		{

			HWR_DoScreenWipe();
			changed = true;
			continue;
		}
#endif
		w = wipe_scr;
		e = wipe_scr_end;

		while (w != wipe_scr + width*height)
		{
			if (*w != *e)
			{
				if (((newval = transtables[(*e<<8) + *w + ((tr_trans80-1)<<FF_TRANSSHIFT)]) == *w)
					&& ((newval = transtables[(*e<<8) + *w + ((tr_trans50-1)<<FF_TRANSSHIFT)]) == *w)
					&& ((newval = transtables[(*w<<8) + *e + ((tr_trans80-1)<<FF_TRANSSHIFT)]) == *w))
				{
					newval = *e;
				}
				*w = newval;
				changed = true;
			}
			w++;
			e++;
		}
	}
	return !changed;
}
#endif

/** Save the "before" screen of a wipe.
  */
void F_WipeStartScreen(void)
{
#ifndef NOWIPE
	if(rendermode != render_soft)
	{
#ifdef SHUFFLE
		HWR_StartScreenWipe();
#endif
		return;
	}
	wipe_scr_start = screens[2];
	if(rendermode == render_soft)
		I_ReadScreen(wipe_scr_start);
#endif
}

/** Save the "after" screen of a wipe.
  *
  * \param x      Starting x coordinate of the starting screen to restore.
  * \param y      Starting y coordinate of the starting screen to restore.
  * \param width  Width of the starting screen to restore.
  * \param height Height of the starting screen to restore.
  */
void F_WipeEndScreen(INT32 x, INT32 y, INT32 width, INT32 height)
{
	if(rendermode != render_soft)
	{
#if defined (SHUFFLE) && defined (HWRENDER)
		HWR_EndScreenWipe();
#endif
		return;
	}
#ifdef NOWIPE
	(void)x;
	(void)y;
	(void)width;
	(void)height;
#else
	wipe_scr_end = screens[3];
	I_ReadScreen(wipe_scr_end);
	V_DrawBlock(x, y, 0, width, height, wipe_scr_start);
#endif
}

/**	\brief	wipe screen

	\param	x	x starting point
	\param	y	y starting point
	\param	width	width of wipe
	\param	height	height of wipe
	\param	ticks	ticks for wipe

	\return	if true, the wipe is done


*/

INT32 F_ScreenWipe(INT32 x, INT32 y, INT32 width, INT32 height, tic_t ticks)
{
	INT32 rc = 1;
	// initial stuff
	(void)x;
	(void)y;
#ifdef NOWIPE
	width = height = ticks = 0;
#else
	if (!WipeInAction)
	{
		WipeInAction = true;
		wipe_scr = screens[0];
		F_InitWipe(width, height, ticks);
	}

	rc = F_DoWipe(width, height, ticks);

	if (rc)
		WipeInAction = false; //Alam: All done?
#endif
	return rc;
}

//
// F_RunWipe
//
//
// After setting up the screens you want to
// wipe, calling this will do a 'typical'
// wipe.
//
void F_RunWipe(tic_t duration, boolean drawMenu)
{
	tic_t wipestart, tics, nowtime, y;
	boolean done;

	wipestart = I_GetTime() - 1;
	y = wipestart + duration; // init a timeout
	do
	{
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
			if (!tics) I_Sleep();
		} while (!tics);
		wipestart = nowtime;

#ifdef SHUFFLE
		done = F_ScreenWipe(0, 0, vid.width, vid.height, tics);
#else
		if (rendermode == render_soft)
			done = F_ScreenWipe(0, 0, vid.width, vid.height, tics);
		else
			done = true;
#endif
		I_OsPolling();
		I_UpdateNoBlit();

		if (drawMenu)
			M_Drawer(); // menu is drawn even on top of wipes

		if (rendermode == render_soft)
			I_FinishUpdate(); // page flip or blit buffer
	} while (!done && I_GetTime() < y);
}
