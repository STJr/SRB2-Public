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
/// \brief share joystick information with game control code

#ifndef __I_JOY_H__
#define __I_JOY_H__

#include "g_input.h"

/*!
  \brief	-JOYAXISRANGE to +JOYAXISRANGE for each axis

	(1024-1) so we can do a right shift instead of division
	(doesnt matter anyway, just give enough precision)
	a gamepad will return -1, 0, or 1 in the event data
	an analog type joystick will return a value
	from -JOYAXISRANGE to +JOYAXISRANGE for each axis
*/

#define JOYAXISRANGE 1023

// detect a bug if we increase JOYBUTTONS above DIJOYSTATE's number of buttons
#if (JOYBUTTONS > 64)
"JOYBUTTONS is greater than INT64 bits can hold"
#endif

/**	\brief	The struct JoyType_s

 share some joystick information (maybe 2 for splitscreen), to the game input code,
 actually, we need to know if it is a gamepad or analog controls
*/

struct JoyType_s
{
	/*! if true, we MUST Poll() to get new joystick data,
	that is: we NEED the DIRECTINPUTDEVICE2 ! (watchout NT compatibility) */
	int bJoyNeedPoll;
	/*! this joystick is a gamepad, read: digital axes
	if FALSE, interpret the joystick event data as JOYAXISRANGE (see above) */
	int bGamepadStyle;

};
typedef struct JoyType_s JoyType_t;
/**	\brief Joystick info
	for palyer 1 and 2's joystick/gamepad
*/

extern JoyType_t Joystick, Joystick2;

#endif // __I_JOY_H__
