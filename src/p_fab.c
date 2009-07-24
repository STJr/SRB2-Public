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
/// \brief some new action routines, separated from the original doom
///	sources, so that you can include it or remove it easy.

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "m_random.h"

static void Translucency_OnChange(void);

/**	\brief cv_translucency
	console variables to turn on and off translucency
*/
consvar_t cv_translucency = {"translucency", "On", CV_CALL|CV_SAVE, CV_OnOff,
	Translucency_OnChange, 0, NULL, NULL, 0, 0, NULL};

/**	\brief Reset Translucency
*/

static boolean resettrans = false;

/**	\brief	The R_SetTrans function
	 Set the translucency map for each frame state of mobj

	\param	state1	1st state
	\param	state2	last state
	\param	transmap	translucency

	\return	void


*/
static void R_SetTrans(statenum_t state1, statenum_t state2, transnum_t transmap)
{
	state_t *state = &states[state1];

	do
	{
		state->frame &= ~FF_TRANSMASK;
		if (!resettrans)
			state->frame |= (transmap<<FF_TRANSSHIFT);
		state++;
	} while (state1++ < state2);
}

/**	\brief	The P_SetTranslucencies function
	 hack the translucency in the states for a set of standard doom sprites

	\return	void


*/
static void P_SetTranslucencies(void)
{
	R_SetTrans(S_SMOK1, S_SMOK5, tr_trans50);
	R_SetTrans(S_SPLASH1, 0, tr_trans50);
	R_SetTrans(S_SPLASH2, 0, tr_trans70);
	R_SetTrans(S_SPLASH3, 0, tr_trans90);

	R_SetTrans(S_DRIPA1, S_DRIPC2, tr_trans30);

	R_SetTrans(S_BLUECRYSTAL1, S_BLUECRYSTAL1, tr_trans30);

	R_SetTrans(S_THOK1, 0, tr_trans50); // Thok! mobj

	R_SetTrans(S_FLAME1, S_FLAME4, tr_trans50); // Flame

	R_SetTrans(S_PARTICLE, S_PARTICLE, tr_trans70);

	// Flame jet
	R_SetTrans(S_FLAMEJETFLAME1, S_FLAMEJETFLAME1, tr_trans50);
	R_SetTrans(S_FLAMEJETFLAME2, S_FLAMEJETFLAME2, tr_trans60);
	R_SetTrans(S_FLAMEJETFLAME3, S_FLAMEJETFLAME3, tr_trans70);

	R_SetTrans(S_BLACKEGG_GOOP1, S_BLACKEGG_GOOP3, tr_trans50);
	R_SetTrans(S_BLACKEGG_GOOP4, 0, tr_trans60);
	R_SetTrans(S_BLACKEGG_GOOP5, 0, tr_trans70);
	R_SetTrans(S_BLACKEGG_GOOP6, 0, tr_trans80);
	R_SetTrans(S_BLACKEGG_GOOP7, 0, tr_trans90);

	R_SetTrans(S_FOG1, S_FOG14, tr_trans50);

	// if higher translucency needed, toy around with the other tr_trans variables

	// shield translucencies
	R_SetTrans(S_SORB1, S_SORB8, tr_trans50);

	// translucent spark
	R_SetTrans(S_SPRK1, S_SPRK1, tr_trans40);
	R_SetTrans(S_SPRK2, S_SPRK4, tr_trans50);
	R_SetTrans(S_SPRK5, S_SPRK7, tr_trans60);
	R_SetTrans(S_SPRK8, S_SPRK10, tr_trans70);
	R_SetTrans(S_SPRK11, S_SPRK13, tr_trans80);
	R_SetTrans(S_SPRK14, S_SPRK16, tr_trans90);

	R_SetTrans(S_SMALLBUBBLE, S_SMALLBUBBLE1, tr_trans50);
	R_SetTrans(S_MEDIUMBUBBLE, S_MEDIUMBUBBLE1, tr_trans50);
	R_SetTrans(S_LARGEBUBBLE, S_EXTRALARGEBUBBLE, tr_trans50);

	R_SetTrans(S_SPLISH1, S_SPLISH9, tr_trans50);
	R_SetTrans(S_TOKEN, 0, tr_trans50);
	R_SetTrans(S_RAIN1, 0, tr_trans50);
}

/**	\brief	The Translucency_OnChange function
	executed when cv_translucency changed
*/
static void Translucency_OnChange(void)
{
	if (!cv_translucency.value)
		resettrans = true;
	P_SetTranslucencies();
	resettrans = false;
}
