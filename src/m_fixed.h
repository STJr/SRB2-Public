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
/**	\brief	Redefinition of INT32 as fixed_t
	unit used as fixed_t
*/

typedef INT32 fixed_t;

/*!
  \brief convert fixed_t into floating number
*/
#define FIXED_TO_FLOAT(x) (((float)(x)) / ((float)FRACUNIT))
#define FLOAT_TO_FIXED(f) (fixed_t)((f) * ((float)FRACUNIT))


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

#if defined (__WATCOMC__) && FRACBITS == 16
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
#elif defined (__GNUC__) && defined (__i386__) && !defined (NOASM)
	// DJGPP, i386 linux, cygwin or mingw
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // asm
	{
		fixed_t ret;
		asm
		(
			 "imull %2;"           // a*b
			 "shrdl %3,%%edx,%0;"  // shift logical right FRACBITS bits
			:"=a" (ret)            // eax is always the result and the first operand (%0,%1)
			:"0" (a), "r" (b)      // and %2 is what we use imull on with what in %1
			, "I" (FRACBITS)       // %3 holds FRACBITS (normally 16)
			:"%cc", "%edx"         // edx and condition codes clobbered
		);
		return ret;
	}

	FUNCMATH FUNCINLINE static inline fixed_t FixedDiv2(fixed_t a, fixed_t b)
	{
		fixed_t ret;
		asm
		(
			  "movl  %1,%%edx;"    // these two instructions allow the next two to pair, on the Pentium processor.
			  "sarl  $31,%%edx;"   // shift arithmetic right 31 on EDX
			  "shldl %3,%1,%%edx;" // DP shift logical left FRACBITS on EDX
			  "sall  %3,%0;"       // shift arithmetic left FRACBITS on EAX
			  "idivl %2;"          // EDX/b = EAX
			: "=a" (ret)
			: "0" (a), "r" (b)
			, "I" (FRACBITS)
			: "%edx"
		);
		return ret;
	}
#elif defined (__GNUC__) && defined (__arm__) && !defined(NOASM) //ARMv4 ASM
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // let abuse smull
	{
		fixed_t ret;
		asm
		(
			  "smull %[lo], r1, %[a], %[b];"
			  "mov %[lo], %[lo], lsr %3;"
			  "orr %[lo], %[lo], r1, lsl %3;"
			: [lo] "=&r" (ret) // rhi, rlo and rm must be distinct registers
			: [a] "r" (a), [b] "r" (b)
			, "i" (FRACBITS)
			: "r1"
		);
		return ret;
	}

	#define __USE_C_FIXEDDIV__ // no double or asm div in ARM land
#elif defined (__GNUC__) && defined (__ppc__) && !defined(NOASM) && 0 // WII: PPC CPU
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // asm
	{
		fixed_t ret, hi, lo;
		asm
		(
			  "mullw %0, %2, %3;"
			  "mulhw %1, %2, %3"
			: "=r" (hi), "=r" (lo)
			: "r" (a), "r" (b)
			, "I" (FRACBITS)
		);
		ret = (INT64)((hi>>FRACBITS)+lo)<<FRACBITS;
		return ret;
	}

	#define __USE_C_FIXEDDIV__// Alam: I am lazy
#elif defined (__GNUC__) && defined (__mips__) && !defined(NOASM) // PSP: MIPS CPU
	FUNCMATH FUNCINLINE static inline fixed_t FixedMul(fixed_t a, fixed_t b) // asm
	{
		fixed_t ret;
		asm
		(
			  "mult %3, %4;"    // a*b=h<32+l
			: "=r" (ret), "=l" (a), "=h" (b) //todo: abuse shr opcode
			: "0" (a), "r" (b)
			, "I" (FRACBITS)
			//: "+l", "+h"
		);
		ret = (INT64)((a>>FRACBITS)+b)<<FRACBITS;
		return ret;
	}

	#define __USE_C_FIXEDDIV__ // no 64b asm div in MIPS land
#elif defined (__GNUC__) && defined (__sh__) && 0 // DC: SH4 CPU
#elif defined (__GNUC__) && defined (__m68k__) && 0 // DEAD: Motorola 6800 CPU
#elif defined (_MSC_VER) && defined(USEASM) && FRACBITS == 16
	// Microsoft Visual C++ (no asm inline)
	fixed_t __cdecl FixedMul(fixed_t a, fixed_t b);
	fixed_t __cdecl FixedDiv2(fixed_t a, fixed_t b);
#else
	#define __USE_C_FIXEDMUL__
	#define __USE_C_FIXEDDIV__
#endif

#ifdef __USE_C_FIXEDMUL__
FUNCMATH fixed_t FixedMul(fixed_t a, fixed_t b);
#endif

#ifdef __USE_C_FIXEDDIV__
FUNCMATH fixed_t FixedDiv2(fixed_t a, fixed_t b);
#endif

/**	\brief	The FixedInt function

	\param	a	fixed_t number

	\return	 a/FRACUNIT
*/

FUNCMATH FUNCINLINE static ATTRINLINE fixed_t FixedInt(fixed_t a)
{
	return FixedMul(a, 1);
}

/**	\brief	The FixedDiv function

	\param	a	fixed_t number
	\param	b	fixed_t number

	\return	a/b


*/
FUNCMATH FUNCINLINE static ATTRINLINE fixed_t FixedDiv(fixed_t a, fixed_t b)
{
	if ((abs(a) >> (FRACBITS-2)) >= abs(b))
		return (a^b) < 0 ? INT32_MIN : INT32_MAX;

	return FixedDiv2(a, b);
}

/**	\brief	The FixedRem function

	\param	x	fixed_t number
	\param	y	fixed_t number

	\return	 remainder of dividing x by y
*/
FUNCMATH FUNCINLINE static ATTRINLINE fixed_t FixedRem(fixed_t x, fixed_t y)
{
	const boolean n = x < 0;
	x = abs(x);
	while (x >= y)
		x -= y;
	if (n)
		return -x;
	else
		return x;
}

/**	\brief	The FixedSqrt function

	\param	x	fixed_t number

	\return	sqrt(x)


*/
FUNCMATH fixed_t FixedSqrt(fixed_t x);

/**	\brief	The FixedHypot function

	\param	x	fixed_t number
	\param	y	fixed_t number

	\return	sqrt(x*x+y*y)


*/
FUNCMATH fixed_t FixedHypot(fixed_t x, fixed_t y);

typedef struct
{
	fixed_t x, y, z;
} vector_t;

vector_t *FV_Load(vector_t *vec, fixed_t x, fixed_t y, fixed_t z);
vector_t *FV_Copy(vector_t *a_o, const vector_t *a_i);
vector_t *FV_AddEx(const vector_t *a_i, const vector_t *a_c, vector_t *a_o);
vector_t *FV_Add(vector_t *a_i, const vector_t *a_c);
vector_t *FV_SubEx(const vector_t *a_i, const vector_t *a_c, vector_t *a_o);
vector_t *FV_Sub(vector_t *a_i, const vector_t *a_c);
vector_t *FV_MulEx(const vector_t *a_i, fixed_t a_c, vector_t *a_o);
vector_t *FV_Mul(vector_t *a_i, fixed_t a_c);
vector_t *FV_DivideEx(const vector_t *a_i, fixed_t a_c, vector_t *a_o);
vector_t *FV_Divide(vector_t *a_i, fixed_t a_c);
vector_t *FV_Midpoint(const vector_t *a_1, const vector_t *a_2, vector_t *a_o);
fixed_t FV_Distance(const vector_t *p1, const vector_t *p2);
fixed_t FV_Magnitude(const vector_t *a_normal);
fixed_t FV_NormalizeEx(const vector_t *a_normal, vector_t *a_o);
fixed_t FV_Normalize(vector_t *a_normal);
vector_t *FV_NegateEx(const vector_t *a_1, vector_t *a_o);
vector_t *FV_Negate(vector_t *a_1);
boolean FV_Equal(const vector_t *a_1, const vector_t *a_2);
fixed_t FV_Dot(const vector_t *a_1, const vector_t *a_2);
vector_t *FV_Cross(const vector_t *a_1, const vector_t *a_2, vector_t *a_o);
vector_t *FV_ClosestPointOnLine(const vector_t *Line, const vector_t *p, vector_t *out);
void FV_ClosestPointOnTriangle (const vector_t *tri, const vector_t *point, vector_t *result);
vector_t *FV_Point2Vec (const vector_t *point1, const vector_t *point2, vector_t *a_o);
void FV_Normal (const vector_t *a_triangle, vector_t *a_normal);
fixed_t FV_PlaneDistance(const vector_t *a_normal, const vector_t *a_point);
boolean FV_IntersectedPlane(const vector_t *a_triangle, const vector_t *a_line, vector_t *a_normal, fixed_t *originDistance);
fixed_t FV_PlaneIntersection(const vector_t *pOrigin, const vector_t *pNormal, const vector_t *rOrigin, const vector_t *rVector);
fixed_t FV_IntersectRaySphere(const vector_t *rO, const vector_t *rV, const vector_t *sO, fixed_t sR);
vector_t *FV_IntersectionPoint(const vector_t *vNormal, const vector_t *vLine, fixed_t distance, vector_t *ReturnVec);
UINT8 FV_PointOnLineSide(const vector_t *point, const vector_t *line);
boolean FV_PointInsideBox(const vector_t *point, const vector_t *box);

typedef struct
{
	fixed_t m[16];
} matrix_t;

void FM_LoadIdentity(matrix_t* matrix);
void FM_CreateObjectMatrix(matrix_t *matrix, fixed_t x, fixed_t y, fixed_t z, fixed_t anglex, fixed_t angley, fixed_t anglez, fixed_t upx, fixed_t upy, fixed_t upz, fixed_t radius);
void FM_MultMatrixVec(const matrix_t *matrix, const vector_t *vec, vector_t *out);
void FM_MultMatrix(matrix_t *dest, const matrix_t *multme);
void FM_Translate(matrix_t *dest, fixed_t x, fixed_t y, fixed_t z);
void FM_Scale(matrix_t *dest, fixed_t x, fixed_t y, fixed_t z);

#endif //m_fixed.h
