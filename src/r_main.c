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
/// \brief Rendering main loop and setup functions,
///	utility functions (BSP, geometry, trigonometry).
///	See tables.c, too.

#include "doomdef.h"
#include "g_game.h"
#include "g_input.h"
#include "r_local.h"
#include "r_splats.h" // faB(21jan): testing
#include "r_sky.h"
#include "st_stuff.h"
#include "p_local.h"
#include "keys.h"
#include "i_video.h"
#include "m_menu.h"
#include "p_local.h"
#include "am_map.h"
#include "d_main.h"
#include "v_video.h"
#include "dstrings.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

//profile stuff ---------------------------------------------------------
//#define TIMING
#ifdef TIMING
#include "p5prof.h"
INT64 mycount;
INT64 mytotal = 0;
//unsigned long  nombre = 100000;
#endif
//profile stuff ---------------------------------------------------------

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048

// increment every time a check is made
size_t validcount = 1;

INT32 centerx, centery;

fixed_t centerxfrac, centeryfrac;
fixed_t projection;
fixed_t projectiony; // aspect ratio

// just for profiling purposes
size_t framecount;

size_t sscount;
size_t loopcount;

fixed_t viewx, viewy, viewz;
angle_t viewangle, aimingangle;
fixed_t viewcos, viewsin;
sector_t *viewsector;
player_t *viewplayer;

//
// precalculated math tables
//
angle_t clipangle;
angle_t doubleclipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
INT32 viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t xtoviewangle[MAXVIDWIDTH+1];

// UNUSED.
// The finetangentgent[angle+FINEANGLES/4] table
// holds the fixed_t tangent values for view angles,
// ranging from INT32_MIN to 0 to INT32_MAX.

fixed_t *finecosine = &finesine[FINEANGLES/4];

lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
lighttable_t *scalelightfixed[MAXLIGHTSCALE];
lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// Hack to support extra boom colormaps.
size_t num_extra_colormaps;
extracolormap_t extra_colormaps[MAXCOLORMAPS];

static CV_PossibleValue_t precipdensity_cons_t[] = {{1, "Thick"}, {2, "Heavy"}, {3, "Moderate"}, {4, "Light"}, {0, NULL}};
static CV_PossibleValue_t grtranslucenthud_cons_t[] = {{1, "MIN"}, {255, "MAX"}, {0, NULL}};

static void ChaseCam_OnChange(void);
static void ChaseCam2_OnChange(void);

consvar_t cv_tailspickup = {"tailspickup", "On", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chasecam = {"chasecam", "On", CV_CALL, CV_OnOff, ChaseCam_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_chasecam2 = {"chasecam2", "On", CV_CALL, CV_OnOff, ChaseCam2_OnChange, 0, NULL, NULL, 0, 0, NULL};

consvar_t cv_shadow = {"shadow", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_shadowoffs = {"offsetshadows", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_precipdensity = {"precipdensity", "Heavy", CV_SAVE, precipdensity_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_soniccd = {"soniccd", "Off", CV_NETVAR, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_allowmlook = {"allowmlook", "Yes", CV_NETVAR, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_precipdist = {"precipdist", "1024", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_showhud = {"showhud", "Yes", CV_CALL,  CV_YesNo, R_SetViewSize, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_limitdraw = {"limitdraw", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grtranslucenthud = {"gr_translucenthud", "255", CV_SAVE|CV_CALL, grtranslucenthud_cons_t, R_SetViewSize, 0, NULL, NULL, 0, 0, NULL};
// Enabling homremoval constitutes a rather sizeable performance hit.
consvar_t cv_homremoval = {"homremoval", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

void SplitScreen_OnChange(void)
{
	if (!cv_debug && netgame)
	{
		if (splitscreen)
		{
			CONS_Printf("Splitscreen not supported in netplay, "
				"sorry!\n");
			splitscreen = false;
		}
		return;
	}

	// recompute screen size
	R_ExecuteSetViewSize();

	// change the menu
	M_SwitchSplitscreen();

	if (!demoplayback)
	{
		if (splitscreen)
			CL_AddSplitscreenPlayer();
		else
			CL_RemoveSplitscreenPlayer();

		if (server && !netgame)
			multiplayer = splitscreen;
	}
	else
	{
		INT32 i;
		secondarydisplayplayer = consoleplayer;
		for (i = 0; i < MAXPLAYERS; i++)
			if (playeringame[i] && i != consoleplayer)
			{
				secondarydisplayplayer = i;
				break;
			}
	}
}

static void ChaseCam_OnChange(void)
{
	if (!cv_chasecam.value || !cv_useranalog.value)
		CV_SetValue(&cv_analog, 0);
	else
		CV_SetValue(&cv_analog, 1);
}

static void ChaseCam2_OnChange(void)
{
	if (!cv_chasecam2.value || !cv_useranalog2.value)
		CV_SetValue(&cv_analog2, 0);
	else
		CV_SetValue(&cv_analog2, 1);
}

//
// R_PointOnSide
// Traverse BSP (sub) tree,
// check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//
INT32 R_PointOnSide(fixed_t x, fixed_t y, node_t *node)
{
	if (!node->dx)
		return x <= node->x ? node->dy > 0 : node->dy < 0;

	if (!node->dy)
		return y <= node->y ? node->dx < 0 : node->dx > 0;

	x -= node->x;
	y -= node->y;

	// Try to quickly decide by looking at sign bits.
	if ((node->dy ^ node->dx ^ x ^ y) < 0)
		return (node->dy ^ x) < 0;  // (left is negative)
	return FixedMul(y, node->dx>>FRACBITS) >= FixedMul(node->dy>>FRACBITS, x);
}

// killough 5/2/98: reformatted
INT32 R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *line)
{
	fixed_t lx = line->v1->x;
	fixed_t ly = line->v1->y;
	fixed_t ldx = line->v2->x - lx;
	fixed_t ldy = line->v2->y - ly;

	if (!ldx)
		return x <= lx ? ldy > 0 : ldy < 0;

	if (!ldy)
		return y <= ly ? ldx < 0 : ldx > 0;

	x -= lx;
	y -= ly;

	// Try to quickly decide by looking at sign bits.
	if ((ldy ^ ldx ^ x ^ y) < 0)
		return (ldy ^ x) < 0;          // (left is negative)
	return FixedMul(y, ldx>>FRACBITS) >= FixedMul(ldy>>FRACBITS, x);
}

//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangle[] table. The +1 size of tantoangle[]
//  is to handle the case when x==y without additional
//  checking.
//
// killough 5/2/98: reformatted, cleaned up

angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
	return (y -= viewy, (x -= viewx) || y) ?
	x >= 0 ?
	y >= 0 ?
		(x > y) ? tantoangle[SlopeDiv(y,x)] :                          // octant 0
		ANGLE_90-1-tantoangle[SlopeDiv(x,y)] :                         // octant 1
		x > (y = -y) ? 0-tantoangle[SlopeDiv(y,x)] :                   // octant 8
		ANGLE_270+tantoangle[SlopeDiv(x,y)] :                          // octant 7
		y >= 0 ? (x = -x) > y ? ANGLE_180-1-tantoangle[SlopeDiv(y,x)] :// octant 3
		ANGLE_90 + tantoangle[SlopeDiv(x,y)] :                         // octant 2
		(x = -x) > (y = -y) ? ANGLE_180+tantoangle[ SlopeDiv(y,x)] :   // octant 4
		ANGLE_270-1-tantoangle[SlopeDiv(x,y)] :                        // octant 5
		0;
}

angle_t R_PointToAngle2(fixed_t pviewx, fixed_t pviewy, fixed_t x, fixed_t y)
{
	return (y -= pviewy, (x -= pviewx) || y) ?
	x >= 0 ?
	y >= 0 ?
		(x > y) ? tantoangle[SlopeDiv(y,x)] :                          // octant 0
		ANGLE_90-1-tantoangle[SlopeDiv(x,y)] :                         // octant 1
		x > (y = -y) ? 0-tantoangle[SlopeDiv(y,x)] :                   // octant 8
		ANGLE_270+tantoangle[SlopeDiv(x,y)] :                          // octant 7
		y >= 0 ? (x = -x) > y ? ANGLE_180-1-tantoangle[SlopeDiv(y,x)] :// octant 3
		ANGLE_90 + tantoangle[SlopeDiv(x,y)] :                         // octant 2
		(x = -x) > (y = -y) ? ANGLE_180+tantoangle[ SlopeDiv(y,x)] :   // octant 4
		ANGLE_270-1-tantoangle[SlopeDiv(x,y)] :                        // octant 5
		0;
}

fixed_t R_PointToDist2(fixed_t px2, fixed_t py2, fixed_t px1, fixed_t py1)
{
	angle_t angle;
	fixed_t dx, dy, dist;

	dx = abs(px1 - px2);
	dy = abs(py1 - py2);

	if (dy > dx)
	{
		fixed_t temp;

		temp = dx;
		dx = dy;
		dy = temp;
	}
	if (!dy)
		return dx;

	angle = (tantoangle[FixedDiv(dy, dx)>>DBITS] + ANGLE_90) >> ANGLETOFINESHIFT;

	// use as cosine
	dist = FixedDiv(dx, FINESINE(angle));

	return dist;
}

// Little extra utility. Works in the same way as R_PointToAngle2
fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
	return R_PointToDist2(viewx, viewy, x, y);
}

/***************************************
*** Zdoom C++ to Legacy C conversion ***
****************************************/

// Utility to find the Z height at an XY location in a sector (for slopes)
fixed_t R_SecplaneZatPoint(secplane_t *secplane, fixed_t x, fixed_t y)
{
	return FixedMul(secplane->ic, -secplane->d - DMulScale16(secplane->a, x, secplane->b, y));
}

// Returns the value of z at (x,y) if d is equal to dist
fixed_t R_SecplaneZatPointDist (secplane_t *secplane, fixed_t x, fixed_t y, fixed_t dist)
{
	return FixedMul(secplane->ic, -dist - DMulScale16(secplane->a, x, secplane->b, y));
}

// Flips the plane's vertical orientiation, so that if it pointed up,
// it will point down, and vice versa.
void R_SecplaneFlipVert(secplane_t *secplane)
{
	secplane->a = -secplane->a;
	secplane->b = -secplane->b;
	secplane->c = -secplane->c;
	secplane->d = -secplane->d;
	secplane->ic = -secplane->ic;
}

// Returns true if 2 planes are the same
boolean R_ArePlanesSame(secplane_t *original, secplane_t *other)
{
	return original->a == other->a && original->b == other->b
		&& original->c == other->c && original->d == other->d;
}

// Returns true if 2 planes are different
boolean R_ArePlanesDifferent(secplane_t *original, secplane_t *other)
{
	return original->a != other->a || original->b != other->b
		|| original->c != other->c || original->d != other->d;
}

// Moves a plane up/down by hdiff units
void R_SecplaneChangeHeight(secplane_t *secplane, fixed_t hdiff)
{
	secplane->d = secplane->d - FixedMul(hdiff, secplane->c);
}

// Returns how much this plane's height would change if d were set to oldd
fixed_t R_SecplaneHeightDiff(secplane_t *secplane, fixed_t oldd)
{
	return FixedMul(oldd - secplane->d, secplane->ic);
}

fixed_t R_SecplanePointToDist(secplane_t *secplane, fixed_t x, fixed_t y, fixed_t z)
{
	return -TMulScale16(secplane->a, x, y, secplane->b, z, secplane->c);
}

fixed_t R_SecplanePointToDist2(secplane_t *secplane, fixed_t x, fixed_t y, fixed_t z)
{
	return -TMulScale16(secplane->a, x, secplane->b, y, z, secplane->c);
}

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//
// killough 5/2/98: reformatted, cleaned up
//
// note: THIS IS USED ONLY FOR WALLS!
fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
	INT32 anglea = ANGLE_90 + (visangle-viewangle);
	INT32 angleb = ANGLE_90 + (visangle-rw_normalangle);
	INT32 den = FixedMul(rw_distance, FINESINE(anglea>>ANGLETOFINESHIFT));
	// proff 11/06/98: Changed for high-res
	fixed_t num = FixedMul(projectiony, FINESINE(angleb>>ANGLETOFINESHIFT));

	if (den > num>>16)
	{
		num = FixedDiv(num, den);
		if (num > 64*FRACUNIT)
			return 64*FRACUNIT;
		if (num < 256)
			return 256;
		return num;
	}
	return 64*FRACUNIT;
}

//
// R_InitTextureMapping
//
static void R_InitTextureMapping(void)
{
	INT32 i;
	INT32 x;
	INT32 t;
	fixed_t focallength;

	// Use tangent table to generate viewangletox:
	//  viewangletox will give the next greatest x
	//  after the view angle.
	//
	// Calc focallength
	//  so FIELDOFVIEW angles covers SCREENWIDTH.
	focallength = FixedDiv(centerxfrac,
		FINETANGENT(FINEANGLES/4+/*cv_fov.value*/ FIELDOFVIEW/2));

	for (i = 0; i < FINEANGLES/2; i++)
	{
		if (FINETANGENT(i) > FRACUNIT*2)
			t = -1;
		else if (FINETANGENT(i) < -FRACUNIT*2)
			t = viewwidth+1;
		else
		{
			t = FixedMul(FINETANGENT(i), focallength);
			t = (centerxfrac - t+FRACUNIT-1)>>FRACBITS;

			if (t < -1)
				t = -1;
			else if (t > viewwidth+1)
				t = viewwidth+1;
		}
		viewangletox[i] = t;
	}

	// Scan viewangletox[] to generate xtoviewangle[]:
	//  xtoviewangle will give the smallest view angle
	//  that maps to x.
	for (x = 0; x <= viewwidth;x++)
	{
		i = 0;
		while (viewangletox[i] > x)
			i++;
		xtoviewangle[x] = (i<<ANGLETOFINESHIFT) - ANGLE_90;
	}

	// Take out the fencepost cases from viewangletox.
	for (i = 0; i < FINEANGLES/2; i++)
	{
		t = FixedMul(FINETANGENT(i), focallength);
		t = centerx - t;

		if (viewangletox[i] == -1)
			viewangletox[i] = 0;
		else if (viewangletox[i] == viewwidth+1)
			viewangletox[i]  = viewwidth;
	}

	clipangle = xtoviewangle[0];
	doubleclipangle = clipangle*2;
}



//
// R_InitLightTables
// Only inits the zlight table,
//  because the scalelight table changes with view size.
//
#define DISTMAP 2

static inline void R_InitLightTables(void)
{
	INT32 i;
	INT32 j;
	INT32 level;
	INT32 startmapl;
	INT32 scale;

	// Calculate the light levels to use
	//  for each level / distance combination.
	for (i = 0; i < LIGHTLEVELS; i++)
	{
		startmapl = ((LIGHTLEVELS-1-i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTZ; j++)
		{
			//added : 02-02-98 : use BASEVIDWIDTH, vid.width is not set already,
			// and it seems it needs to be calculated only once.
			scale = FixedDiv((BASEVIDWIDTH/2*FRACUNIT), (j+1)<<LIGHTZSHIFT);
			scale >>= LIGHTSCALESHIFT;
			level = startmapl - scale/DISTMAP;

			if (level < 0)
				level = 0;

			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS-1;

			zlight[i][j] = colormaps + level*256;
		}
	}
}


//
// R_SetViewSize
// Do not really change anything here,
// because it might be in the middle of a refresh.
// The change will take effect next refresh.
//
boolean setsizeneeded;

void R_SetViewSize(void)
{
	setsizeneeded = true;
}

//
// R_ExecuteSetViewSize
//
void R_ExecuteSetViewSize(void)
{
	fixed_t cosadj;
	fixed_t dy;
	INT32 i;
	INT32 j;
	INT32 level;
	INT32 startmapl;
	INT32 aspectx;  //added : 02-02-98 : for aspect ratio calc. below...

	setsizeneeded = false;

	if (rendermode == render_none)
		return;

	// status bar overlay
	st_overlay = cv_showhud.value;

	scaledviewwidth = vid.width;
	viewheight = vid.height;

	if (splitscreen)
		viewheight >>= 1;

	viewwidth = scaledviewwidth;

	centery = viewheight/2;
	centerx = viewwidth/2;
	centerxfrac = centerx<<FRACBITS;
	centeryfrac = centery<<FRACBITS;

	projection = centerxfrac;
	projectiony = (((vid.height*centerx*BASEVIDWIDTH)/BASEVIDHEIGHT)/vid.width)<<FRACBITS;

	R_InitViewBuffer(scaledviewwidth, viewheight);

	R_InitTextureMapping();

#ifdef HWRENDER
	if (rendermode != render_soft)
		HWR_InitTextureMapping();
#endif

	// thing clipping
	for (i = 0; i < viewwidth; i++)
		screenheightarray[i] = (INT16)viewheight;

	// setup sky scaling (uses pspriteyscale)
	R_SetSkyScale();

	// planes
	aspectx = (((vid.height*centerx*BASEVIDWIDTH)/BASEVIDHEIGHT)/vid.width);

	if (rendermode == render_soft)
	{
		// this is only used for planes rendering in software mode
		j = viewheight*4;
		for (i = 0; i < j; i++)
		{
			dy = ((i - viewheight*2)<<FRACBITS) + FRACUNIT/2;
			dy = abs(dy);
			yslopetab[i] = FixedDiv(aspectx*FRACUNIT, dy);
		}
	}

	for (i = 0; i < viewwidth; i++)
	{
		cosadj = abs(FINECOSINE(xtoviewangle[i]>>ANGLETOFINESHIFT));
		distscale[i] = FixedDiv(FRACUNIT, cosadj);
	}

	// Calculate the light levels to use for each level/scale combination.
	for (i = 0; i< LIGHTLEVELS; i++)
	{
		startmapl = ((LIGHTLEVELS - 1 - i)*2)*NUMCOLORMAPS/LIGHTLEVELS;
		for (j = 0; j < MAXLIGHTSCALE; j++)
		{
			level = startmapl - j*vid.width/(viewwidth)/DISTMAP;

			if (level < 0)
				level = 0;

			if (level >= NUMCOLORMAPS)
				level = NUMCOLORMAPS - 1;

			scalelight[i][j] = colormaps + level*256;
		}
	}

	// continue to do the software setviewsize as long as we use the reference software view
#ifdef HWRENDER
	if (rendermode != render_soft)
		HWR_SetViewSize();
#endif

	am_recalc = true;
}

//
// R_Init
//

void R_Init(void)
{
	R_LoadSkinTable();

	// screensize independent
	if (devparm)
		CONS_Printf("\nR_InitData");
	R_InitData();

	if (devparm)
		CONS_Printf("\nR_InitViewBorder");
	R_InitViewBorder();
	R_SetViewSize(); // setsizeneeded is set true

	if (devparm)
		CONS_Printf("\nR_InitPlanes");
	R_InitPlanes();

	// this is now done by SCR_Recalc() at the first mode set
	if (devparm)
		CONS_Printf("\nR_InitLightTables");
	R_InitLightTables();

	if (devparm)
		CONS_Printf("\nR_InitTranslationTables\n");
	R_InitTranslationTables();

	R_InitDrawNodes();

	framecount = 0;
}

//
// R_PointInSubsector
//
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
	size_t nodenum = numnodes-1;

	while (!(nodenum & NF_SUBSECTOR))
		nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];

	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_IsPointInSubsector, same as above but returns 0 if not in subsector
//
subsector_t *R_IsPointInSubsector(fixed_t x, fixed_t y)
{
	node_t *node;
	INT32 side, i;
	size_t nodenum;
	subsector_t *ret;

	// single subsector is a special case
	if (numnodes == 0)
		return subsectors;

	nodenum = numnodes - 1;

	while (!(nodenum & NF_SUBSECTOR))
	{
		node = &nodes[nodenum];
		side = R_PointOnSide(x, y, node);
		nodenum = node->children[side];
	}

	ret = &subsectors[nodenum & ~NF_SUBSECTOR];
	for (i = 0; i < ret->numlines; i++)
		if (R_PointOnSegSide(x, y, &segs[ret->firstline + i]))
			return 0;

	return ret;
}

//
// R_SetupFrame
//

static mobj_t *viewmobj;

// WARNING: a should be unsigned but to add with 2048, it isn't!
#define AIMINGTODY(a) ((FINETANGENT((2048+(((INT32)a)>>ANGLETOFINESHIFT)) & FINEMASK)*160)>>FRACBITS)

void R_SetupFrame(player_t *player)
{
	INT32 dy = 0;
	camera_t *thiscam;

	if (splitscreen && player == &players[secondarydisplayplayer]
		&& player != &players[consoleplayer])
	{
		thiscam = &camera2;
	}
	else
		thiscam = &camera;

	if (cv_chasecam.value && thiscam == &camera && !thiscam->chase)
	{
		P_ResetCamera(player, &camera);
		thiscam->chase = true;
	}
	else if (cv_chasecam2.value && thiscam == &camera2 && !thiscam->chase)
	{
		P_ResetCamera(player, &camera2);
		thiscam->chase = true;
	}
	else if (!cv_chasecam.value && thiscam == &camera)
		thiscam->chase = false;
	else if (!cv_chasecam2.value && thiscam == &camera2)
		thiscam->chase = false;

	if (player->awayviewtics)
	{
		// cut-away view stuff
		viewmobj = player->awayviewmobj; // should be a MT_ALTVIEWMAN
		viewz = viewmobj->z + 20*FRACUNIT;
		aimingangle = player->awayviewaiming;
		viewangle = viewmobj->angle;
	}
	else if ((cv_chasecam.value && thiscam == &camera)
		|| (cv_chasecam2.value && thiscam == &camera2))
	// use outside cam view
	{
		viewmobj = player->mo; // LIES! FILTHY STINKING LIES!!!
		I_Assert(viewmobj != NULL);
		viewz = thiscam->z + (thiscam->height>>1);
		aimingangle = thiscam->aiming;
		viewangle = thiscam->angle;
	}
	else
	// use the player's eyes view
	{
		viewz = player->viewz;

		viewmobj = player->mo;

		aimingangle = player->aiming;
		viewangle = viewmobj->angle;

		if (!demoplayback && player->playerstate != PST_DEAD)
		{
			if (player == &players[consoleplayer])
			{
				viewangle = localangle; // WARNING: camera uses this
				aimingangle = localaiming;
			}
			else if (player == &players[secondarydisplayplayer])
			{
				viewangle = localangle2;
				aimingangle = localaiming2;
			}
		}
	}

	if (!viewmobj)
#ifdef PARANOIA
		{
			const size_t playeri = (size_t)(player - players);
			I_Error("R_SetupFrame: viewmobj null (player %"PRIdS")", playeri);
		}
#else
		return;
#endif

	viewplayer = player;

	if (((cv_chasecam.value && thiscam == &camera) || (cv_chasecam2.value && thiscam == &camera2))
		&& !player->awayviewtics)
	{
		viewx = thiscam->x;
		viewy = thiscam->y;

		if (thiscam->subsector)
			viewsector = thiscam->subsector->sector;
		else
			viewsector = R_PointInSubsector(viewx, viewy)->sector;
	}
	else
	{
		viewx = viewmobj->x;
		viewy = viewmobj->y;

		if (viewmobj->subsector)
			viewsector = viewmobj->subsector->sector;
		else
			viewsector = R_PointInSubsector(viewx, viewy)->sector;
	}

	viewsin = FINESINE(viewangle>>ANGLETOFINESHIFT);
	viewcos = FINECOSINE(viewangle>>ANGLETOFINESHIFT);

	sscount = 0;

	// recalc necessary stuff for mouseaiming
	// slopes are already calculated for the full possible view (which is 4*viewheight).

	if (rendermode == render_soft)
	{
		// clip it in the case we are looking a hardware 90 degrees full aiming
		// (lmps, network and use F12...)
		G_ClipAimingPitch((INT32 *)&aimingangle);

		if (!splitscreen)
			dy = AIMINGTODY(aimingangle) * viewheight/BASEVIDHEIGHT;
		else
			dy = AIMINGTODY(aimingangle) * viewheight*2/BASEVIDHEIGHT;

		yslope = &yslopetab[(3*viewheight/2) - dy];
	}
	centery = (viewheight/2) + dy;
	centeryfrac = centery<<FRACBITS;

	framecount++;
	validcount++;
}

// ================
// R_RenderView
// ================

//                     FAB NOTE FOR WIN32 PORT !! I'm not finished already,
// but I suspect network may have problems with the video buffer being locked
// for all duration of rendering, and being released only once at the end..
// I mean, there is a win16lock() or something that lasts all the rendering,
// so maybe we should release screen lock before each netupdate below..?

void R_RenderPlayerView(player_t *player)
{
	R_SetupFrame(player);

	// Clear buffers.
	R_ClearClipSegs();
	R_ClearDrawSegs();
	R_ClearPlanes();
	R_ClearSprites();

#ifdef FLOORSPLATS
	R_ClearVisibleFloorSplats();
#endif

	if (cv_homremoval.value && player == &players[displayplayer]) // if this is display player 1
		V_DrawFill(0, 0, vid.width, vid.height, 31); // No HOM effect!

	// check for new console commands.
	NetUpdate();

	// The head node is the last node output.

//profile stuff ---------------------------------------------------------
#ifdef TIMING
	mytotal = 0;
	ProfZeroTimer();
#endif
	R_RenderBSPNode((INT32)numnodes - 1);
#ifdef TIMING
	RDMSR(0x10, &mycount);
	mytotal += mycount; // 64bit add

	CONS_Printf("RenderBSPNode: 0x%d %d\n", *((INT32 *)&mytotal + 1), (INT32)mytotal);
#endif
//profile stuff ---------------------------------------------------------

	// Check for new console commands.
	NetUpdate();

	R_DrawPlanes();

	// Check for new console commands.
	NetUpdate();

#ifdef FLOORSPLATS
	R_DrawVisibleFloorSplats();
#endif

	// draw mid texture and sprite
	// And now 3D floors/sides!
	R_DrawMasked();

	// Check for new console commands.
	NetUpdate();
}

// =========================================================================
//                    ENGINE COMMANDS & VARS
// =========================================================================

void R_RegisterEngineStuff(void)
{
	CV_RegisterVar(&cv_gravity);
	CV_RegisterVar(&cv_tailspickup);
	CV_RegisterVar(&cv_soniccd);
	CV_RegisterVar(&cv_allowmlook);
	CV_RegisterVar(&cv_homremoval);

	// Enough for dedicated server
	if (dedicated)
		return;

	CV_RegisterVar(&cv_precipdist);
	CV_RegisterVar(&cv_chasecam);
	CV_RegisterVar(&cv_chasecam2);
	CV_RegisterVar(&cv_shadow);
	CV_RegisterVar(&cv_shadowoffs);
	CV_RegisterVar(&cv_precipdensity);

	CV_RegisterVar(&cv_cam_dist);
	CV_RegisterVar(&cv_cam_still);
	CV_RegisterVar(&cv_cam_height);
	CV_RegisterVar(&cv_cam_speed);
	CV_RegisterVar(&cv_cam_rotate);
	CV_RegisterVar(&cv_cam_rotspeed);

	CV_RegisterVar(&cv_cam2_dist);
	CV_RegisterVar(&cv_cam2_still);
	CV_RegisterVar(&cv_cam2_height);
	CV_RegisterVar(&cv_cam2_speed);
	CV_RegisterVar(&cv_cam2_rotate);
	CV_RegisterVar(&cv_cam2_rotspeed);

	CV_RegisterVar(&cv_showhud);

	// Default viewheight is changeable,
	// initialized to standard viewheight
	CV_RegisterVar(&cv_viewheight);
	CV_RegisterVar (&cv_limitdraw);
	CV_RegisterVar(&cv_grtranslucenthud);

#ifdef HWRENDER
	// GL-specific Commands
	CV_RegisterVar(&cv_grgammablue);
	CV_RegisterVar(&cv_grgammagreen);
	CV_RegisterVar(&cv_grgammared);
	CV_RegisterVar(&cv_grfovchange);
	CV_RegisterVar(&cv_grfog);
	CV_RegisterVar(&cv_grfogcolor);
#endif

#ifdef HWRENDER
	if (rendermode != render_soft && rendermode != render_none)
		HWR_AddCommands();
#endif
}
