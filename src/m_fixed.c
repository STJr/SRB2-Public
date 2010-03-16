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
/// \brief Fixed point implementation

#include "doomdef.h"
#include "m_fixed.h"

#ifndef _arch_dreamcast // Alam: if you want hypot(), use -lm
#define HAVE_HYPOT
#endif
#ifndef _WIN32 // MSVCRT does not have *f() functions
#define HAVE_HYPOTF
#define HAVE_SQRTF
#endif

#ifdef __USE_C_FIXEDMUL__

/**	\brief	The FixedMul function

	\param	a	fixed_t number
	\param	b	fixed_t number

	\return	a*b>>FRACBITS


*/
fixed_t FixedMul(fixed_t a, fixed_t b)
{
#if 0 //defined (_WIN32) && !defined (_XBOX) && !defined (_WIN32_WCE)
	return (fixed_t)MulDiv(a, b, FRACUNIT);
#elif 1 // Wine's MulDiv( INT nMultiplicand, INT nMultiplier, INT nDivisor)
	INT64 ret;
	// If the result is positive, we "add" to round. else, we subtract to round.
	if ( ( (a <  0) && (b <  0) ) ||
	     ( (a >= 0) && (b >= 0) ) )
		ret = (((INT64)a * b) + (FRACUNIT/2)) / FRACUNIT;
	else
		ret = (((INT64)a * b) - (FRACUNIT/2)) / FRACUNIT;

	if ((ret > 2147483647) || (ret < -2147483647)) return -1;
	return (fixed_t)ret;
#else
	return (fixed_t)(((INT64) a * (INT64) b)>>FRACBITS);
#endif
}

#endif //__USE_C_FIXEDMUL__

#ifdef __USE_C_FIXEDDIV__
/**	\brief	The FixedDiv2 function

	\param	a	fixed_t number
	\param	b	fixed_t number

	\return	a/b * FRACUNIT


*/
fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
#if 0 //defined (_WIN32) && !defined (_XBOX) && !defined (_WIN32_WCE)
	INT c = MulDiv(a, FRACUNIT, b);
	if (c == -1)
		I_Error("FixedDiv: divide by zero");
	return (fixed_t)c;
#elif 1 // Wine's MulDiv( INT nMultiplicand, INT nMultiplier, INT nDivisor)
	INT64 ret;

	if (b == 0)
		I_Error("FixedDiv: divide by zero");

	// We want to deal with a positive divisor to simplify the logic.
	if (b < 0)
	{
		a = -a;
		b = -b;
	}

	// If the result is positive, we "add" to round. else, we subtract to round.
	if (a >= 0)
		ret = (((INT64)a * FRACUNIT) + (b/2)) / b;
	else
		ret = (((INT64)a * FRACUNIT) - (b/2)) / b;

	if ((ret > 2147483647) || (ret < -2147483647))
		I_Error("FixedDiv: divide by zero");
	return (fixed_t)ret;
#else
	double c = ((double)a) / ((double)b) * FRACUNIT;

	if (c >= 2147483648.0 || c < -2147483648.0)
		I_Error("FixedDiv: divide by zero");
	return (fixed_t)c;
#endif
}

#endif // __USE_C_FIXEDDIV__

#ifndef NO_M

fixed_t FixedSqrt(fixed_t x)
{
	const float fx = FIXED_TO_FLOAT(x);
	float fr;
#ifdef HAVE_SQRTF
	fr = sqrtf(fx);
#else
	fr = (float)sqrt(fx);
#endif
	return FLOAT_TO_FIXED(fr);
}

fixed_t FixedHypot(fixed_t x, fixed_t y)
{
#ifdef HAVE_HYPOT
	const float fx = FIXED_TO_FLOAT(x);
	const float fy = FIXED_TO_FLOAT(y);
	float fz;
#ifdef HAVE_HYPOTF
	fz = hypotf(fx, fy);
#else
	fz = (float)hypot(fx, fy);
#endif
	return FLOAT_TO_FIXED(fz);
#else // !HAVE_HYPOT
	fixed_t ax, yx, yx2, yx1;
	if (abs(y) > abs(x)) // |y|>|x|
	{
		ax = abs(y); // |y| => ax
		yx = FixedDiv(x, y); // (x/y)
	}
	else // |x|>|y|
	{
		ax = abs(x); // |x| => ax
		yx = FixedDiv(y, x); // (x/y)
	}
	yx2 = FixedMul(yx, yx); // (x/y)^2
	yx1 = FixedSqrt(1+FRACUNIT + yx2); // (1 + (x/y)^2)^1/2
	return FixedMul(ax, yx1); // |x|*((1 + (x/y)^2)^1/2)
#endif
}

#endif // no math libary?
