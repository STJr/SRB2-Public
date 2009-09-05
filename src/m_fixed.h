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
/// \brief Fixed point arithmetics implementation
///
///	Fixed point, 32bit as 16.16.

#ifndef __M_FIXED__
#define __M_FIXED__

#include "doomtype.h"
#include <math.h>
#ifdef __GNUC__
#include <stdlib.h>
#endif

#ifdef _WIN32_WCE
#include "sdl/SRB2CE/cehelp.h"
#endif

/*!
  \brief bits of the fraction
*/
#define FRACBITS 16
/*!
  \brief units of the fraction
*/
#define FRACUNIT (1<<FRACBITS)
#define FRACMASK (FRACUNIT -1)
/**	\brief	Redefinition of int as fixed_t
	unit used as fixed_t
*/

#if defined (_MSC_VER)
typedef __int32 fixed_t;
#else
typedef int fixed_t;
#endif

/*!
  \brief convert fixed_t into floating number
*/
#define FIXED_TO_FLOAT(x) (((float)(x)) / ((float)FRACUNIT))


/**	\brief	The TMulScale16 function

	\param	a	a parameter of type fixed_t
	\param	b	a parameter of type fixed_t
	\param	c	a parameter of type fixed_t
	\param	d	a parameter of type fixed_t
	\param	e	a parameter of type fixed_t
	\param	f	a parameter of type fixed_t

	\return	fixed_t


*/
FUNCMATH FUNCINLINE static ATTRINLINE fixed_t TMulScale16(fixed_t a, fixed_t b, fixed_t c, fixed_t d, fixed_t e, fixed_t f) \
{ \
	return (fixed_t)((((INT64)a * (INT64)b) + ((INT64)c * (INT64)d) \
		+ ((INT64)e * (INT64)f)) >> 16); \
}

/**	\brief	The DMulScale16 function

	\param	a	a parameter of type fixed_t
	\param	b	a parameter of type fixed_t
	\param	c	a parameter of type fixed_t
	\param	d	a parameter of type fixed_t

	\return	fixed_t


*/
FUNCMATH FUNCINLINE static ATTRINLINE fixed_t DMulScale16(fixed_t a, fixed_t b, fixed_t c, fixed_t d) \
{ \
	return (fixed_t)((((INT64)a * (INT64)b) + ((INT64)c * (INT64)d)) >> 16); \
}

#ifdef __WATCOMC__
	#pragma aux FixedMul =  \
		"imul ebx",         \
		"shrd eax,edx,16"   \
		parm    [eax] [ebx] \
		value   [eax]       \
		modify exact [eax edx]

	#pragma aux FixedDiv2 = \
		"cdq",              \
		"shld edx,eax,16",  \
		"sal eax,16",       \
		"idiv ebx"          \
		parm    [eax] [ebx] \
		value   [eax]       \
		modify exact [eax edx]
#elif defined (__GNUC__) && defined (__i386__) && !defined (NOASM) && FRACBITS == 16
	// DJGPP, i386 linux, cygwin or mingw
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // asm
	{
		fixed_t ret;
		asm
		(
			 "imull %2;"           // a*b
			 "shrdl $16,%%edx,%0;" // shift 16 bits
			:"=a" (ret)            // eax is always the result and the first operand (%0,%1)
			:"0" (a), "r" (b)      // and %2 is what we use imull on with what in %1
			:"%cc", "%edx"         // edx and condition codes clobbered */
		);
		return ret;
	}

	FUNCMATH FUNCINLINE static inline fixed_t FixedDiv2(fixed_t a, fixed_t b)
	{
		fixed_t ret;
		asm
		(
			  "movl  %1,%%edx;"  // these two instructions allow the next
			  "sarl  $31,%%edx;" // two to pair, on the Pentium processor.
			  "shldl $16,%1,%%edx;"
			  "sall  $16,%0;"
			  "idivl %2;"
			: "=a" (ret)
			: "0" (a), "r" (b)
			: "%edx"
		);
		return ret;
	}
#elif defined (__GNUC__) && defined (__arm__)
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // ARM asm
	{
		fixed_t ret;
		asm
		(
			  "smull %[lo], r1, %[a], %[b];"
			  "mov %[lo], %[lo], lsr #16;"
			  "orr %[lo], %[lo], r1, lsl #16;"
			: [lo] "=&r" (ret)	// rhi, rlo and rm must be distinct registers
			: [a] "r" (a), [b] "r" (b)
			: "r1"
		);
		return ret;
	}

	FUNCMATH FUNCINLINE static inline fixed_t FixedDiv2(fixed_t a, fixed_t b) // no double or asm div in ARM land
	{
		return (((INT64)a)<<16)/b;
	}
#elif defined (_MSC_VER) && defined(USEASM) && FRACBITS == 16
	// Microsoft Visual C++ (no asm inline)
	fixed_t __cdecl FixedMul(fixed_t a, fixed_t b);
	fixed_t __cdecl FixedDiv2(fixed_t a, fixed_t b);
#else
	FUNCMATH fixed_t FixedMul(fixed_t a, fixed_t b);
	FUNCMATH fixed_t FixedDiv2(fixed_t a, fixed_t b);
	#define __USE_C_FIXED__
#endif

/**	\brief	The FixedDiv function

	\param	a	fixed_t number
	\param	b	fixed_t number

	\return	a/b


*/
FUNCINLINE static ATTRINLINE fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if ((abs(a) >> (FRACBITS-2)) >= abs(b))
		return (a^b) < 0 ? MININT : MAXINT;

	return FixedDiv2(a, b);
}


/**	\brief	The FixedMod function
	\author CPhipps from PrBoom

	\param	a	fixed_t number
	\param	b	fixed_t number

	\return	 a % b, guaranteeing 0 <= a < b
	\note that the C standard for % does not guarantee this
*/
FUNCINLINE static ATTRINLINE fixed_t FixedMod(fixed_t a, fixed_t b)
{
	if (b & (b-1))
	{
		const fixed_t r = a % b;
		return ((r < 0) ? r+b : r);
	}
	return (a & (b-1));
}

#endif //m_fixed.h
