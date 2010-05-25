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
#if 0 //#ifndef _WIN32 // MSVCRT does not have *f() functions
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

vector_t *FV_Load(vector_t *vec, fixed_t x, fixed_t y, fixed_t z)
{
	vec->x = x;
	vec->y = y;
	vec->z = z;
	return vec;
}

vector_t *FV_Copy(vector_t *a_o, const vector_t *a_i)
{
	return M_Memcpy(a_o, a_i, sizeof(vector_t));
}

vector_t *FV_AddEx(const vector_t *a_i, const vector_t *a_c, vector_t *a_o)
{
	a_o->x = a_i->x + a_c->x;
	a_o->y = a_i->y + a_c->y;
	a_o->z = a_i->z + a_c->z;
	return a_o;
}

vector_t *FV_Add(vector_t *a_i, const vector_t *a_c)
{
	return FV_AddEx(a_i, a_c, a_i);
}

vector_t *FV_SubEx(const vector_t *a_i, const vector_t *a_c, vector_t *a_o)
{
	a_o->x = a_i->x - a_c->x;
	a_o->y = a_i->y - a_c->y;
	a_o->z = a_i->z - a_c->z;
	return a_o;
}

vector_t *FV_Sub(vector_t *a_i, const vector_t *a_c)
{
	return FV_SubEx(a_i, a_c, a_i);
}

vector_t *FV_MulEx(const vector_t *a_i, fixed_t a_c, vector_t *a_o)
{
	a_o->x = FixedMul(a_i->x, a_c);
	a_o->y = FixedMul(a_i->y, a_c);
	a_o->z = FixedMul(a_i->z, a_c);
	return a_o;
}

vector_t *FV_Mul(vector_t *a_i, fixed_t a_c)
{
	return FV_MulEx(a_i, a_c, a_i);
}

vector_t *FV_DivideEx(const vector_t *a_i, fixed_t a_c, vector_t *a_o)
{
	a_o->x = FixedDiv(a_i->x, a_c);
	a_o->y = FixedDiv(a_i->y, a_c);
	a_o->z = FixedDiv(a_i->z, a_c);
	return a_o;
}

vector_t *FV_Divide(vector_t *a_i, fixed_t a_c)
{
	return FV_DivideEx(a_i, a_c, a_i);
}

// Vector Complex Math
vector_t *FV_Midpoint(const vector_t *a_1, const vector_t *a_2, vector_t *a_o)
{
	a_o->x = FixedDiv(a_2->x - a_1->x, 2*FRACUNIT);
	a_o->y = FixedDiv(a_2->y - a_1->y, 2*FRACUNIT);
	a_o->z = FixedDiv(a_2->z - a_1->z, 2*FRACUNIT);
	a_o->x = a_1->x + a_o->x;
	a_o->y = a_1->y + a_o->y;
	a_o->z = a_1->z + a_o->z;
	return a_o;
}

fixed_t FV_Distance(const vector_t *p1, const vector_t *p2)
{
	INT32 xs = FixedMul(p2->x-p1->x,p2->x-p1->x);
	INT32 ys = FixedMul(p2->y-p1->y,p2->y-p1->y);
	INT32 zs = FixedMul(p2->z-p1->z,p2->z-p1->z);
	return FixedSqrt(xs+ys+zs);
}

fixed_t FV_Magnitude(const vector_t *a_normal)
{
	INT32 xs = FixedMul(a_normal->x,a_normal->x);
	INT32 ys = FixedMul(a_normal->y,a_normal->y);
	INT32 zs = FixedMul(a_normal->z,a_normal->z);
	return FixedSqrt(xs+ys+zs);
}

// Also returns the magnitude
fixed_t FV_NormalizeEx(const vector_t *a_normal, vector_t *a_o)
{
	fixed_t magnitude = FV_Magnitude(a_normal);
	a_o->x = FixedDiv(a_normal->x, magnitude);
	a_o->y = FixedDiv(a_normal->y, magnitude);
	a_o->z = FixedDiv(a_normal->z, magnitude);
	return magnitude;
}

fixed_t FV_Normalize(vector_t *a_normal)
{
	return FV_NormalizeEx(a_normal, a_normal);
}

vector_t *FV_NegateEx(const vector_t *a_1, vector_t *a_o)
{
	a_o->x = -a_1->x;
	a_o->y = -a_1->y;
	a_o->z = -a_1->z;
	return a_o;
}

vector_t *FV_Negate(vector_t *a_1)
{
	return FV_NegateEx(a_1, a_1);
}

boolean FV_Equal(const vector_t *a_1, const vector_t *a_2)
{
	fixed_t Epsilon = FRACUNIT/FRACUNIT;

	if ((abs(a_2->x - a_1->x) > Epsilon) ||
		(abs(a_2->y - a_1->y) > Epsilon) ||
		(abs(a_2->z - a_1->z) > Epsilon))
	{
		return true;
	}

	return false;
}

fixed_t FV_Dot(const vector_t *a_1, const vector_t *a_2)
{
	return (FixedMul(a_1->x, a_2->x) + FixedMul(a_1->y, a_2->y) + FixedMul(a_1->z, a_2->z));
}

vector_t *FV_Cross(const vector_t *a_1, const vector_t *a_2, vector_t *a_o)
{
	a_o->x = FixedMul(a_1->y, a_2->z) - FixedMul(a_1->z, a_2->y);
	a_o->y = FixedMul(a_1->z, a_2->x) - FixedMul(a_1->x, a_2->z);
	a_o->z = FixedMul(a_1->x, a_2->y) - FixedMul(a_1->y, a_2->x);
	return a_o;
}

//
// ClosestPointOnLine
//
// Finds the point on a line closest
// to the specified point.
//
vector_t *FV_ClosestPointOnLine(const vector_t *Line, const vector_t *p, vector_t *out)
{
   // Determine t (the length of the vector from ‘Line[0]’ to ‘p’)
   vector_t c, V;
   fixed_t t, d = 0;
   FV_SubEx(p, &Line[0], &c);
   FV_SubEx(&Line[1], &Line[0], &V);
   FV_NormalizeEx(&V, &V);

   d = FV_Distance(&Line[0], &Line[1]);
   t = FV_Dot(&V, &c);

   // Check to see if ‘t’ is beyond the extents of the line segment
   if (t < 0)
   {
	   return FV_Copy(out, &Line[0]);
   }
   if (t > d)
   {
	   return FV_Copy(out, &Line[1]);
   }

   // Return the point between ‘Line[0]’ and ‘Line[1]’
   FV_Mul(&V, t);

   return FV_AddEx(&Line[0], &V, out);
}

//
// ClosestPointOnTriangle
//
// Given a triangle and a point,
// the closest point on the edge of
// the triangle is returned.
//
void FV_ClosestPointOnTriangle (const vector_t *tri, const vector_t *point, vector_t *result)
{
	UINT8 i;
	fixed_t dist, closestdist;
	vector_t EdgePoints[3];
	vector_t Line[2];

	FV_Copy(&Line[0], &tri[0]);
	FV_Copy(&Line[1], &tri[1]);
	FV_ClosestPointOnLine(Line, point, &EdgePoints[0]);

	FV_Copy(&Line[0], &tri[1]);
	FV_Copy(&Line[1], &tri[2]);
	FV_ClosestPointOnLine(Line, point, &EdgePoints[1]);

	FV_Copy(&Line[0], &tri[2]);
	FV_Copy(&Line[1], &tri[0]);
	FV_ClosestPointOnLine(Line, point, &EdgePoints[2]);

	// Find the closest one of the three
	FV_Copy(result, &EdgePoints[0]);
	closestdist = FV_Distance(point, &EdgePoints[0]);
	for (i = 1; i < 3; i++)
	{
		dist = FV_Distance(point, &EdgePoints[i]);

		if (dist < closestdist)
		{
			closestdist = dist;
			FV_Copy(result, &EdgePoints[i]);
		}
	}

	// We now have the closest point! Whee!
}

//
// Point2Vec
//
// Given two points, create a vector between them.
//
vector_t *FV_Point2Vec (const vector_t *point1, const vector_t *point2, vector_t *a_o)
{
	a_o->x = point1->x - point2->x;
	a_o->y = point1->y - point2->y;
	a_o->z = point1->z - point2->z;
	return a_o;
}

//
// Normal
//
// Calculates the normal of a polygon.
//
void FV_Normal (const vector_t *a_triangle, vector_t *a_normal)
{
	vector_t a_1;
	vector_t a_2;

	FV_Point2Vec(&a_triangle[2], &a_triangle[0], &a_1);
	FV_Point2Vec(&a_triangle[1], &a_triangle[0], &a_2);

	FV_Cross(&a_1, &a_2, a_normal);

	FV_NormalizeEx(a_normal, a_normal);
}

//
// PlaneDistance
//
// Calculates distance between a plane and the origin.
//
fixed_t FV_PlaneDistance(const vector_t *a_normal, const vector_t *a_point)
{
	return -(FixedMul(a_normal->x, a_point->x) + FixedMul(a_normal->y, a_point->y) + FixedMul(a_normal->z, a_point->z));
}

boolean FV_IntersectedPlane(const vector_t *a_triangle, const vector_t *a_line, vector_t *a_normal, fixed_t *originDistance)
{
	fixed_t distance1 = 0, distance2 = 0;

	FV_Normal(a_triangle, a_normal);

	*originDistance = FV_PlaneDistance(a_normal, &a_triangle[0]);

	distance1 = (FixedMul(a_normal->x, a_line[0].x)  + FixedMul(a_normal->y, a_line[0].y)
				+ FixedMul(a_normal->z, a_line[0].z)) + *originDistance;

	distance2 = (FixedMul(a_normal->x, a_line[1].x)  + FixedMul(a_normal->y, a_line[1].y)
				+ FixedMul(a_normal->z, a_line[1].z)) + *originDistance;

	// Positive or zero number means no intersection
	if (FixedMul(distance1, distance2) >= 0)
		return false;

	return true;
}

//
// PlaneIntersection
//
// Returns the distance from
// rOrigin to where the ray
// intersects the plane. Assumes
// you already know it intersects
// the plane.
//
fixed_t FV_PlaneIntersection(const vector_t *pOrigin, const vector_t *pNormal, const vector_t *rOrigin, const vector_t *rVector)
{
  fixed_t d = -(FV_Dot(pNormal, pOrigin));
  fixed_t number = FV_Dot(pNormal,rOrigin) + d;
  fixed_t denom = FV_Dot(pNormal,rVector);
  return -FixedDiv(number, denom);
}

//
// IntersectRaySphere
// Input : rO - origin of ray in world space
//         rV - vector describing direction of ray in world space
//         sO - Origin of sphere
//         sR - radius of sphere
// Notes : Normalized directional vectors expected
// Return: distance to sphere in world units, -1 if no intersection.
//
fixed_t FV_IntersectRaySphere(const vector_t *rO, const vector_t *rV, const vector_t *sO, fixed_t sR)
{
	vector_t Q;
	fixed_t c, v, d;
	FV_SubEx(sO, rO, &Q);

	c = FV_Magnitude(&Q);
	v = FV_Dot(&Q, rV);
	d = FixedMul(sR, sR) - (FixedMul(c,c) - FixedMul(v,v));

	// If there was no intersection, return -1
	if (d < 0*FRACUNIT)
		return (-1*FRACUNIT);

	// Return the distance to the [first] intersecting point
	return (v - FixedSqrt(d));
}

//
// IntersectionPoint
//
// This returns the intersection point of the line that intersects the plane
//
vector_t *FV_IntersectionPoint(const vector_t *vNormal, const vector_t *vLine, fixed_t distance, vector_t *ReturnVec)
{
	vector_t vLineDir; // Variables to hold the point and the line's direction
	fixed_t Numerator = 0, Denominator = 0, dist = 0;

	// Here comes the confusing part.  We need to find the 3D point that is actually
	// on the plane.  Here are some steps to do that:

	// 1)  First we need to get the vector of our line, Then normalize it so it's a length of 1
	FV_Point2Vec(&vLine[1], &vLine[0], &vLineDir);		// Get the Vector of the line
	FV_NormalizeEx(&vLineDir, &vLineDir);				// Normalize the lines vector


	// 2) Use the plane equation (distance = Ax + By + Cz + D) to find the distance from one of our points to the plane.
	//    Here I just chose a arbitrary point as the point to find that distance.  You notice we negate that
	//    distance.  We negate the distance because we want to eventually go BACKWARDS from our point to the plane.
	//    By doing this is will basically bring us back to the plane to find our intersection point.
	Numerator = - (FixedMul(vNormal->x, vLine[0].x) +		// Use the plane equation with the normal and the line
				   FixedMul(vNormal->y, vLine[0].y) +
				   FixedMul(vNormal->z, vLine[0].z) + distance);

	// 3) If we take the dot product between our line vector and the normal of the polygon,
	//    this will give us the cosine of the angle between the 2 (since they are both normalized - length 1).
	//    We will then divide our Numerator by this value to find the offset towards the plane from our arbitrary point.
	Denominator = FV_Dot(vNormal, &vLineDir);		// Get the dot product of the line's vector and the normal of the plane

	// Since we are using division, we need to make sure we don't get a divide by zero error
	// If we do get a 0, that means that there are INFINITE points because the the line is
	// on the plane (the normal is perpendicular to the line - (Normal.Vector = 0)).
	// In this case, we should just return any point on the line.

	if( Denominator == 0*FRACUNIT) // Check so we don't divide by zero
	{
		ReturnVec->x = vLine[0].x;
		ReturnVec->y = vLine[0].y;
		ReturnVec->z = vLine[0].z;
		return ReturnVec;	// Return an arbitrary point on the line
	}

	// We divide the (distance from the point to the plane) by (the dot product)
	// to get the distance (dist) that we need to move from our arbitrary point.  We need
	// to then times this distance (dist) by our line's vector (direction).  When you times
	// a scalar (single number) by a vector you move along that vector.  That is what we are
	// doing.  We are moving from our arbitrary point we chose from the line BACK to the plane
	// along the lines vector.  It seems logical to just get the numerator, which is the distance
	// from the point to the line, and then just move back that much along the line's vector.
	// Well, the distance from the plane means the SHORTEST distance.  What about in the case that
	// the line is almost parallel with the polygon, but doesn't actually intersect it until half
	// way down the line's length.  The distance from the plane is short, but the distance from
	// the actual intersection point is pretty long.  If we divide the distance by the dot product
	// of our line vector and the normal of the plane, we get the correct length.  Cool huh?

	dist = FixedDiv(Numerator, Denominator);				// Divide to get the multiplying (percentage) factor

	// Now, like we said above, we times the dist by the vector, then add our arbitrary point.
	// This essentially moves the point along the vector to a certain distance.  This now gives
	// us the intersection point.  Yay!

	// Return the intersection point
	ReturnVec->x = vLine[0].x + FixedMul(vLineDir.x, dist);
	ReturnVec->y = vLine[0].y + FixedMul(vLineDir.y, dist);
	ReturnVec->z = vLine[0].z + FixedMul(vLineDir.z, dist);
	return ReturnVec;
}

//
// PointOnLineSide
//
// If on the front side of the line, returns 1.
// If on the back side of the line, returns 0.
// 2D only.
//
UINT8 FV_PointOnLineSide(const vector_t *point, const vector_t *line)
{
	fixed_t s1 = FixedMul((point->y - line[0].y),(line[1].x - line[0].x));
	fixed_t s2 = FixedMul((point->x - line[0].x),(line[1].y - line[0].y));
	return (UINT8)(s1 - s2 < 0);
}

//
// PointInsideBox
//
// Given four points of a box,
// determines if the supplied point is
// inside the box or not.
//
boolean FV_PointInsideBox(const vector_t *point, const vector_t *box)
{
	vector_t lastLine[2];

	FV_Load(&lastLine[0], box[3].x, box[3].y, box[3].z);
	FV_Load(&lastLine[1], box[0].x, box[0].y, box[0].z);

	if (FV_PointOnLineSide(point, &box[0])
		|| FV_PointOnLineSide(point, &box[1])
		|| FV_PointOnLineSide(point, &box[2])
		|| FV_PointOnLineSide(point, lastLine))
		return false;

	return true;
}
//
// LoadIdentity
//
// Loads the identity matrix into a matrix
//
void FM_LoadIdentity(matrix_t* matrix)
{
#define M(row,col)  matrix->m[col * 4 + row]
	memset(matrix, 0x00, sizeof(matrix_t));

	M(0, 0) = FRACUNIT;
	M(1, 1) = FRACUNIT;
	M(2, 2) = FRACUNIT;
	M(3, 3) = FRACUNIT;
#undef M
}

//
// CreateObjectMatrix
//
// Creates a matrix that can be used for
// adjusting the position of an object
//
void FM_CreateObjectMatrix(matrix_t *matrix, fixed_t x, fixed_t y, fixed_t z, fixed_t anglex, fixed_t angley, fixed_t anglez, fixed_t upx, fixed_t upy, fixed_t upz, fixed_t radius)
{
	vector_t upcross;
	vector_t upvec;
	vector_t basevec;

	FV_Load(&upvec, upx, upy, upz);
	FV_Load(&basevec, anglex, angley, anglez);
	FV_Cross(&upvec, &basevec, &upcross);
	FV_Normalize(&upcross);

	FM_LoadIdentity(matrix);

	matrix->m[0] = upcross.x;
	matrix->m[1] = upcross.y;
	matrix->m[2] = upcross.z;
	matrix->m[3] = 0*FRACUNIT;

	matrix->m[4] = upx;
	matrix->m[5] = upy;
	matrix->m[6] = upz;
	matrix->m[7] = 0;

	matrix->m[8] = anglex;
	matrix->m[9] = angley;
	matrix->m[10] = anglez;
	matrix->m[11] = 0;

	matrix->m[12] = x - FixedMul(upx,radius);
	matrix->m[13] = y - FixedMul(upy,radius);
	matrix->m[14] = z - FixedMul(upz,radius);
	matrix->m[15] = FRACUNIT;
}

//
// MultMatrixVec
//
// Multiplies a vector by the specified matrix
//
void FM_MultMatrixVec(const matrix_t *matrix, const vector_t *vec, vector_t *out)
{
#define M(row,col)  matrix->m[col * 4 + row]
	out->x = FixedMul(vec->x,M(0, 0))
	       + FixedMul(vec->y,M(0, 1))
	       + FixedMul(vec->z,M(0, 2))
	       + M(0, 3);

	out->y = FixedMul(vec->x,M(1, 0))
	       + FixedMul(vec->y,M(1, 1))
	       + FixedMul(vec->z,M(1, 2))
	       + M(1, 3);

	out->z = FixedMul(vec->x,M(2, 0))
	       + FixedMul(vec->y,M(2, 1))
	       + FixedMul(vec->z,M(2, 2))
	       + M(2, 3);
#undef M
}

//
// MultMatrix
//
// Multiples one matrix into another
//
void FM_MultMatrix(matrix_t *dest, const matrix_t *multme)
{
	matrix_t result;
	UINT8 i, j;
#define M(row,col)  multme->m[col * 4 + row]
#define D(row,col)  dest->m[col * 4 + row]
#define R(row,col)  result.m[col * 4 + row]

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
			R(i, j) = FixedMul(D(i, 0),M(0, j)) + FixedMul(D(i, 1),M(1, j)) + FixedMul(D(i, 2),M(2, j)) + FixedMul(D(i, 3),M(3, j));
	}

	M_Memcpy(dest, &result, sizeof(matrix_t));

#undef R
#undef D
#undef M
}

//
// Translate
//
// Translates a matrix
//
void FM_Translate(matrix_t *dest, fixed_t x, fixed_t y, fixed_t z)
{
	matrix_t trans;
#define M(row,col)  trans.m[col * 4 + row]

	memset(&trans, 0x00, sizeof(matrix_t));

	M(0, 0) = M(1, 1) = M(2, 2) = M(3, 3) = FRACUNIT;
	M(0, 3) = x;
	M(1, 3) = y;
	M(2, 3) = z;

	FM_MultMatrix(dest, &trans);
#undef M
}

//
// Scale
//
// Scales a matrix
//
void FM_Scale(matrix_t *dest, fixed_t x, fixed_t y, fixed_t z)
{
	matrix_t scale;
#define M(row,col)  scale.m[col * 4 + row]

	memset(&scale, 0x00, sizeof(matrix_t));

	M(3, 3) = FRACUNIT;
	M(0, 0) = x;
	M(1, 1) = y;
	M(2, 2) = z;

	FM_MultMatrix(dest, &scale);
#undef M
}
