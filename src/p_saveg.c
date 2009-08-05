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
/// \brief Archiving: SaveGame I/O
/// \todo remove M_Memcpy

#include "doomdef.h"
#include "byteptr.h"
#include "d_main.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_random.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "r_things.h"
#include "r_state.h"
#include "w_wad.h"
#include "y_inter.h"
#include "z_zone.h"
#include "r_main.h"
#include "r_sky.h"
#include "p_polyobj.h"

savedata_t savedata;
byte *save_p;

// Note: This cannot be bigger
// than an unsigned short
typedef enum
{
	RFLAGPOINT = 0x01,
	BFLAGPOINT = 0x02,
	CAPSULE    = 0x04,
	AWAYVIEW   = 0x08,
	FIRSTAXIS  = 0x10,
	SECONDAXIS = 0x20,
} player_saveflags;

//
// P_ArchivePlayer
//
static void P_ArchivePlayer(void)
{
	player_t *player = &players[consoleplayer];

	WRITEBYTE(save_p, player->skincolor);
	WRITEBYTE(save_p, player->skin);

	WRITELONG(save_p, player->score);
	WRITELONG(save_p, player->lives);
	WRITELONG(save_p, player->continues);
}

//
// P_UnArchivePlayer
//
static void P_UnArchivePlayer(void)
{
	savedata.skincolor = READBYTE(save_p);
	savedata.skin = READBYTE(save_p);

	savedata.score = READLONG(save_p);
	savedata.lives = READLONG(save_p);
	savedata.continues = READLONG(save_p);
}

//
// P_NetArchivePlayers
//
static void P_NetArchivePlayers(void)
{
	int i, j, flags;
	size_t q;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (!playeringame[i])
			continue;

		flags = 0;

		WRITEANGLE(save_p, players[i].aiming);
		WRITEANGLE(save_p, players[i].awayviewaiming);
		WRITELONG(save_p, players[i].awayviewtics);
		WRITELONG(save_p, players[i].health);

		WRITELONG(save_p, players[i].currentweapon);
		WRITELONG(save_p, players[i].ringweapons);
		WRITEFIXED(save_p, players[i].tossstrength);

		for (j = 0; j < NUMPOWERS; j++)
			WRITELONG(save_p, players[i].powers[j]);

		WRITEBYTE(save_p, players[i].playerstate);
		WRITELONG(save_p, players[i].pflags);
		WRITEBYTE(save_p, players[i].spectator);

		WRITELONG(save_p, players[i].bonuscount);
		WRITELONG(save_p, players[i].skincolor);
		WRITELONG(save_p, players[i].skin);
		WRITEULONG(save_p, players[i].score);
		WRITELONG(save_p, players[i].dashspeed);
		WRITELONG(save_p, players[i].lives);
		WRITELONG(save_p, players[i].continues);
		WRITELONG(save_p, players[i].xtralife);
		WRITELONG(save_p, players[i].speed);
		WRITELONG(save_p, players[i].jumping);
		WRITEBYTE(save_p, players[i].secondjump);
		WRITELONG(save_p, players[i].fly1);
		WRITEULONG(save_p, players[i].scoreadd);
		WRITEULONG(save_p, players[i].glidetime);
		WRITELONG(save_p, players[i].climbing);
		WRITELONG(save_p, players[i].deadtimer);
		WRITELONG(save_p, players[i].splish);
		WRITEULONG(save_p, players[i].exiting);
		WRITELONG(save_p, players[i].blackow);
		WRITEBYTE(save_p, players[i].homing);

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		WRITEFIXED(save_p, players[i].cmomx); // Conveyor momx
		WRITEFIXED(save_p, players[i].cmomy); // Conveyor momy
		WRITEFIXED(save_p, players[i].rmomx); // "Real" momx (momx - cmomx)
		WRITEFIXED(save_p, players[i].rmomy); // "Real" momy (momy - cmomy)

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		WRITELONG(save_p, players[i].numboxes);
		WRITELONG(save_p, players[i].totalring);
		WRITEULONG(save_p, players[i].realtime);
		WRITEULONG(save_p, players[i].racescore);
		WRITEULONG(save_p, players[i].laps);

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].tagzone);
		WRITELONG(save_p, players[i].taglag);

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].ctfteam);
		WRITEUSHORT(save_p, players[i].gotflag);

		WRITELONG(save_p, players[i].dbginfo);
		WRITELONG(save_p, players[i].emeraldhunt);

		WRITELONG(save_p, players[i].weapondelay);
		WRITELONG(save_p, players[i].tossdelay);
		WRITELONG(save_p, players[i].shielddelay);
		WRITEULONG(save_p, players[i].taunttimer);

		WRITEULONG(save_p, players[i].starposttime);
		WRITELONG(save_p, players[i].starpostx);
		WRITELONG(save_p, players[i].starposty);
		WRITELONG(save_p, players[i].starpostz);
		WRITELONG(save_p, players[i].starpostnum);
		WRITEANGLE(save_p, players[i].starpostangle);
		WRITEULONG(save_p, players[i].starpostbit);

		WRITEANGLE(save_p, players[i].angle_pos);
		WRITEANGLE(save_p, players[i].old_angle_pos);

		WRITELONG(save_p, players[i].flyangle);
		WRITEULONG(save_p, players[i].drilltimer);
		WRITELONG(save_p, players[i].linkcount);
		WRITEULONG(save_p, players[i].linktimer);
		WRITELONG(save_p, players[i].anotherflyangle);
		WRITEULONG(save_p, players[i].nightstime);
		WRITEULONG(save_p, players[i].bumpertime);
		WRITELONG(save_p, players[i].drillmeter);
		WRITEBYTE(save_p, players[i].drilldelay);
		WRITEBYTE(save_p, players[i].bonustime);
		WRITEBYTE(save_p, players[i].mare);

		if (players[i].capsule)
			flags |= CAPSULE;

		if (players[i].awayviewmobj)
			flags |= AWAYVIEW;

		if (players[i].axis1)
			flags |= FIRSTAXIS;

		if (players[i].axis2)
			flags |= SECONDAXIS;

		WRITESHORT(save_p, players[i].lastsidehit);
		WRITESHORT(save_p, players[i].lastlinehit);


		WRITELONG(save_p, players[i].losscount);

		WRITELONG(save_p, players[i].onconveyor);

		WRITEULONG(save_p, players[i].jointime);

		if (i == 0)
		{
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == rflagpoint)
				{
					flags |= RFLAGPOINT;
					break;
				}
			}

			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == bflagpoint)
				{
					flags |= BFLAGPOINT;
					break;
				}
			}
		}

		WRITEUSHORT(save_p, flags);

		if (flags & CAPSULE)
			WRITEULONG(save_p, players[i].capsule->mobjnum);

		if (flags & FIRSTAXIS)
			WRITEULONG(save_p, players[i].axis1->mobjnum);

		if (flags & SECONDAXIS)
			WRITEULONG(save_p, players[i].axis2->mobjnum);

		if (flags & AWAYVIEW)
			WRITEULONG(save_p, players[i].awayviewmobj->mobjnum);

		if (flags & RFLAGPOINT)
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == rflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}

		if (flags & BFLAGPOINT)
			for (q = 0; q < nummapthings; q++)
			{
				if (&mapthings[q] == bflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}
	}
}

//
// P_NetUnArchivePlayers
//
static void P_NetUnArchivePlayers(void)
{
	int i, j, flags;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		memset(&players[i], 0, sizeof (player_t));
		if (!playeringame[i])
			continue;

		players[i].aiming = READANGLE(save_p);
		players[i].awayviewaiming = READANGLE(save_p);
		players[i].awayviewtics = READLONG(save_p);
		players[i].health = READLONG(save_p);

		players[i].currentweapon = READLONG(save_p);
		players[i].ringweapons = READLONG(save_p);
		players[i].tossstrength = READFIXED(save_p);

		for (j = 0; j < NUMPOWERS; j++)
			players[i].powers[j] = READLONG(save_p);

		players[i].playerstate = READBYTE(save_p);
		players[i].pflags = READLONG(save_p);
		players[i].spectator = READBYTE(save_p);

		players[i].bonuscount = READLONG(save_p);
		players[i].skincolor = READLONG(save_p);
		players[i].skin = READLONG(save_p);
		players[i].score = READULONG(save_p);
		players[i].dashspeed = READLONG(save_p); // dashing speed
		players[i].lives = READLONG(save_p);
		players[i].continues = READLONG(save_p); // continues that player has acquired
		players[i].xtralife = READLONG(save_p); // Ring Extra Life counter
		players[i].speed = READLONG(save_p); // Player's speed (distance formula of MOMX and MOMY values)
		players[i].jumping = READLONG(save_p); // Jump counter
		players[i].secondjump = READBYTE(save_p);
		players[i].fly1 = READLONG(save_p); // Tails flying
		players[i].scoreadd = READULONG(save_p); // Used for multiple enemy attack bonus
		players[i].glidetime = READULONG(save_p); // Glide counter for thrust
		players[i].climbing = READLONG(save_p); // Climbing on the wall
		players[i].deadtimer = READLONG(save_p); // End game if game over lasts too long
		players[i].splish = READLONG(save_p); // Don't make splish repeat tons
		players[i].exiting = READULONG(save_p); // Exitlevel timer
		players[i].blackow = READLONG(save_p);
		players[i].homing = READBYTE(save_p); // Are you homing?

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		players[i].cmomx = READFIXED(save_p); // Conveyor momx
		players[i].cmomy = READFIXED(save_p); // Conveyor momy
		players[i].rmomx = READFIXED(save_p); // "Real" momx (momx - cmomx)
		players[i].rmomy = READFIXED(save_p); // "Real" momy (momy - cmomy)

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		players[i].numboxes = READLONG(save_p); // Number of item boxes obtained for Race Mode
		players[i].totalring = READLONG(save_p); // Total number of rings obtained for Race Mode
		players[i].realtime = READULONG(save_p); // integer replacement for leveltime
		players[i].racescore = READULONG(save_p); // Total of won categories
		players[i].laps = READULONG(save_p); // Number of laps (optional)

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		players[i].tagzone = READLONG(save_p); // Tag Zone timer
		players[i].taglag = READLONG(save_p); // Don't go back in the tag zone too early

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		players[i].ctfteam = READLONG(save_p); // 1 == Red, 2 == Blue
		players[i].gotflag = READUSHORT(save_p); // 1 == Red, 2 == Blue Do you have the flag?

		players[i].dbginfo = READLONG(save_p); // Debugger
		players[i].emeraldhunt = READLONG(save_p); // # of emeralds found

		players[i].weapondelay = READLONG(save_p);
		players[i].tossdelay = READLONG(save_p);
		players[i].shielddelay = READLONG(save_p);
		players[i].taunttimer = READULONG(save_p);

		players[i].starposttime = READULONG(save_p);
		players[i].starpostx = READLONG(save_p);
		players[i].starposty = READLONG(save_p);
		players[i].starpostz = READLONG(save_p);
		players[i].starpostnum = READLONG(save_p);
		players[i].starpostangle = READANGLE(save_p);
		players[i].starpostbit = READULONG(save_p);

		players[i].angle_pos = READANGLE(save_p);
		players[i].old_angle_pos = READANGLE(save_p);

		players[i].flyangle = READLONG(save_p);
		players[i].drilltimer = READULONG(save_p);
		players[i].linkcount = READLONG(save_p);
		players[i].linktimer = READULONG(save_p);
		players[i].anotherflyangle = READLONG(save_p);
		players[i].nightstime = READULONG(save_p);
		players[i].bumpertime = READULONG(save_p);
		players[i].drillmeter = READLONG(save_p);
		players[i].drilldelay = READBYTE(save_p);
		players[i].bonustime = READBYTE(save_p);
		players[i].mare = READBYTE(save_p);

		players[i].lastsidehit = READSHORT(save_p);
		players[i].lastlinehit = READSHORT(save_p);

		players[i].losscount = READLONG(save_p);

		players[i].onconveyor = READLONG(save_p);

		players[i].jointime = READULONG(save_p);

		flags = READUSHORT(save_p);

		if (flags & CAPSULE)
			players[i].capsule = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & FIRSTAXIS)
			players[i].axis1 = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & SECONDAXIS)
			players[i].axis2 = (mobj_t *)(size_t)READULONG(save_p);

		if (flags & AWAYVIEW)
			players[i].awayviewmobj = (mobj_t *)(size_t)READULONG(save_p);

		if (i == 0)
		{
			if (flags & RFLAGPOINT)
				rflagpoint = &mapthings[READULONG(save_p)];

			if (flags & BFLAGPOINT)
				bflagpoint = &mapthings[READULONG(save_p)];
		}

		players[i].viewheight = cv_viewheight.value<<FRACBITS;

		SetPlayerSkinByNum(i, players[i].skin);
	}
}

#define SD_FLOORHT  0x01
#define SD_CEILHT   0x02
#define SD_FLOORPIC 0x04
#define SD_CEILPIC  0x08
#define SD_LIGHT    0x10
#define SD_SPECIAL  0x20
#define SD_DIFF2    0x40

// diff2 flags
#define SD_FXOFFS    0x01
#define SD_FYOFFS    0x02
#define SD_CXOFFS    0x04
#define SD_CYOFFS    0x08
#define SD_TAG       0x10
#define SD_FLOORANG  0x20
#define SD_CEILANG   0x40

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x80

// diff2 flags
#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08

//
// P_NetArchiveWorld
//
static void P_NetArchiveWorld(void)
{
	size_t i;
	int statsec = 0, statline = 0;
	line_t *li = lines;
	side_t *si;
	byte *put = save_p;

	// reload the map just to see difference
	mapsector_t *ms;
	mapsidedef_t *msd;
	maplinedef_t *mld;
	sector_t *ss = sectors;
	byte diff, diff2;

	ms = W_CacheLumpNum(lastloadedmaplumpnum+ML_SECTORS, PU_CACHE);

	for (i = 0; i < numsectors; i++, ss++, ms++)
	{
		diff = diff2 = 0;
		if (ss->floorheight != SHORT(ms->floorheight)<<FRACBITS)
			diff |= SD_FLOORHT;
		if (ss->ceilingheight != SHORT(ms->ceilingheight)<<FRACBITS)
			diff |= SD_CEILHT;
		//
		// flats
		//
		// P_AddLevelFlat should not add but just return the number
		if (ss->floorpic != P_AddLevelFlat(ms->floorpic, levelflats))
			diff |= SD_FLOORPIC;
		if (ss->ceilingpic != P_AddLevelFlat(ms->ceilingpic, levelflats))
			diff |= SD_CEILPIC;

		if (ss->lightlevel != SHORT(ms->lightlevel))
			diff |= SD_LIGHT;
		if (ss->special != SHORT(ms->special))
			diff |= SD_SPECIAL;

		/// \todo this makes Flat Alignment (linetype 7) increase the savegame size!
		if (ss->floor_xoffs != 0)
			diff2 |= SD_FXOFFS;
		if (ss->floor_yoffs != 0)
			diff2 |= SD_FYOFFS;
		if (ss->ceiling_xoffs != 0)
			diff2 |= SD_CXOFFS;
		if (ss->ceiling_yoffs != 0)
			diff2 |= SD_CYOFFS;
		if (ss->floorpic_angle != 0)
			diff2 |= SD_FLOORANG;
		if (ss->ceilingpic_angle != 0)
			diff2 |= SD_CEILANG;

		if (ss->tag != SHORT(ms->tag))
			diff2 |= SD_TAG;

		if (diff2)
			diff |= SD_DIFF2;

		if (diff)
		{
			statsec++;

			WRITEUSHORT(put, i);
			WRITEBYTE(put, diff);
			if (diff & SD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & SD_FLOORHT)
				WRITEFIXED(put, ss->floorheight);
			if (diff & SD_CEILHT)
				WRITEFIXED(put, ss->ceilingheight);
			if (diff & SD_FLOORPIC)
				WRITEMEM(put, levelflats[ss->floorpic].name, 8);
			if (diff & SD_CEILPIC)
				WRITEMEM(put, levelflats[ss->ceilingpic].name, 8);
			if (diff & SD_LIGHT)
				WRITESHORT(put, ss->lightlevel);
			if (diff & SD_SPECIAL)
				WRITESHORT(put, ss->special);
			if (diff2 & SD_FXOFFS)
				WRITEFIXED(put, ss->floor_xoffs);
			if (diff2 & SD_FYOFFS)
				WRITEFIXED(put, ss->floor_yoffs);
			if (diff2 & SD_CXOFFS)
				WRITEFIXED(put, ss->ceiling_xoffs);
			if (diff2 & SD_CYOFFS)
				WRITEFIXED(put, ss->ceiling_yoffs);
			if (diff2 & SD_TAG)
			{
				WRITESHORT(put, ss->tag);
				WRITELONG(put, ss->firsttag);
				WRITELONG(put, ss->nexttag);
			}
			if (diff2 & SD_FLOORANG)
				WRITEANGLE(put, ss->floorpic_angle);
			if (diff2 & SD_CEILANG)
				WRITEANGLE(put, ss->ceilingpic_angle);
		}
	}

	WRITEUSHORT(put, 0xffff);

	mld = W_CacheLumpNum(lastloadedmaplumpnum+ML_LINEDEFS, PU_CACHE);
	msd = W_CacheLumpNum(lastloadedmaplumpnum+ML_SIDEDEFS, PU_CACHE);
	// do lines
	for (i = 0; i < numlines; i++, mld++, li++)
	{
		diff = diff2 = 0;

		if (li->special != SHORT(mld->special))
			diff |= LD_SPECIAL;

		if (li->sidenum[0] != 0xffff)
		{
			si = &sides[li->sidenum[0]];
			if (si->textureoffset != SHORT(msd[li->sidenum[0]].textureoffset)<<FRACBITS)
				diff |= LD_S1TEXOFF;
			//SoM: 4/1/2000: Some textures are colormaps. Don't worry about invalid textures.
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].toptexture, li->sidenum[0]) != -1
				&& si->toptexture != R_TextureNumForName(msd[li->sidenum[0]].toptexture, li->sidenum[0]))
				diff |= LD_S1TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].bottomtexture, li->sidenum[0]) != -1
				&& si->bottomtexture != R_TextureNumForName(msd[li->sidenum[0]].bottomtexture, li->sidenum[0]))
				diff |= LD_S1BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[0]].midtexture, li->sidenum[0]) != -1
				&& si->midtexture != R_TextureNumForName(msd[li->sidenum[0]].midtexture, li->sidenum[0]))
				diff |= LD_S1MIDTEX;
		}
		if (li->sidenum[1] != 0xffff)
		{
			si = &sides[li->sidenum[1]];
			if (si->textureoffset != SHORT(msd[li->sidenum[1]].textureoffset)<<FRACBITS)
				diff2 |= LD_S2TEXOFF;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].toptexture, li->sidenum[1]) != -1
				&& si->toptexture != R_TextureNumForName(msd[li->sidenum[1]].toptexture, li->sidenum[1]))
				diff2 |= LD_S2TOPTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].bottomtexture, li->sidenum[1]) != -1
				&& si->bottomtexture != R_TextureNumForName(msd[li->sidenum[1]].bottomtexture, li->sidenum[1]))
				diff2 |= LD_S2BOTTEX;
			if (R_CheckTextureNumForName(msd[li->sidenum[1]].midtexture, li->sidenum[1]) != -1
				&& si->midtexture != R_TextureNumForName(msd[li->sidenum[1]].midtexture, li->sidenum[1]))
				diff2 |= LD_S2MIDTEX;
			if (diff2)
				diff |= LD_DIFF2;
		}

		if (diff)
		{
			statline++;
			WRITESHORT(put, (short)i);
			WRITEBYTE(put, diff);
			if (diff & LD_DIFF2)
				WRITEBYTE(put, diff2);
			if (diff & LD_FLAG)
				WRITESHORT(put, li->flags);
			if (diff & LD_SPECIAL)
				WRITESHORT(put, li->special);

			si = &sides[li->sidenum[0]];
			if (diff & LD_S1TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff & LD_S1TOPTEX)
				WRITELONG(put, si->toptexture);
			if (diff & LD_S1BOTTEX)
				WRITELONG(put, si->bottomtexture);
			if (diff & LD_S1MIDTEX)
				WRITELONG(put, si->midtexture);

			si = &sides[li->sidenum[1]];
			if (diff2 & LD_S2TEXOFF)
				WRITEFIXED(put, si->textureoffset);
			if (diff2 & LD_S2TOPTEX)
				WRITELONG(put, si->toptexture);
			if (diff2 & LD_S2BOTTEX)
				WRITELONG(put, si->bottomtexture);
			if (diff2 & LD_S2MIDTEX)
				WRITELONG(put, si->midtexture);
		}
	}
	WRITEUSHORT(put, 0xffff);

	save_p = put;
}

//
// P_NetUnArchiveWorld
//
static void P_NetUnArchiveWorld(void)
{
	ULONG i;
	line_t *li;
	side_t *si;
	byte *get;
	byte diff, diff2;

	get = save_p;

	for (;;)
	{
		i = READUSHORT(get);

		if (i == 0xffff)
			break;

		if (i > numsectors)
			I_Error("Invalid sector number %lu from server", i);

		diff = READBYTE(get);
		if (diff & SD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;

		if (diff & SD_FLOORHT)
			sectors[i].floorheight = READFIXED(get);
		if (diff & SD_CEILHT)
			sectors[i].ceilingheight = READFIXED(get);
		if (diff & SD_FLOORPIC)
		{
			sectors[i].floorpic = P_AddLevelFlat((char *)get, levelflats);
			get += 8;
		}
		if (diff & SD_CEILPIC)
		{
			sectors[i].ceilingpic = P_AddLevelFlat((char *)get, levelflats);
			get += 8;
		}
		if (diff & SD_LIGHT)
			sectors[i].lightlevel = READSHORT(get);
		if (diff & SD_SPECIAL)
			sectors[i].special = READSHORT(get);

		if (diff2 & SD_FXOFFS)
			sectors[i].floor_xoffs = READFIXED(get);
		if (diff2 & SD_FYOFFS)
			sectors[i].floor_yoffs = READFIXED(get);
		if (diff2 & SD_CXOFFS)
			sectors[i].ceiling_xoffs = READFIXED(get);
		if (diff2 & SD_CYOFFS)
			sectors[i].ceiling_yoffs = READFIXED(get);
		if (diff2 & SD_TAG)
		{
			short tag;
			tag = READSHORT(get);
			sectors[i].firsttag = READLONG(get);
			sectors[i].nexttag = READLONG(get);
			P_ChangeSectorTag(i, tag);
		}
		if (diff2 & SD_FLOORANG)
			sectors[i].floorpic_angle  = READANGLE(get);
		if (diff2 & SD_CEILANG)
			sectors[i].ceilingpic_angle = READANGLE(get);
	}

	for (;;)
	{
		i = READUSHORT(get);

		if (i == 0xffff)
			break;
		if (i > numlines)
			I_Error("Invalid line number %lu from server", i);

		diff = READBYTE(get);
		li = &lines[i];

		if (diff & LD_DIFF2)
			diff2 = READBYTE(get);
		else
			diff2 = 0;
		if (diff & LD_FLAG)
			li->flags = READSHORT(get);
		if (diff & LD_SPECIAL)
			li->special = READSHORT(get);

		si = &sides[li->sidenum[0]];
		if (diff & LD_S1TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff & LD_S1TOPTEX)
			si->toptexture = READLONG(get);
		if (diff & LD_S1BOTTEX)
			si->bottomtexture = READLONG(get);
		if (diff & LD_S1MIDTEX)
			si->midtexture = READLONG(get);

		si = &sides[li->sidenum[1]];
		if (diff2 & LD_S2TEXOFF)
			si->textureoffset = READFIXED(get);
		if (diff2 & LD_S2TOPTEX)
			si->toptexture = READLONG(get);
		if (diff2 & LD_S2BOTTEX)
			si->bottomtexture = READLONG(get);
		if (diff2 & LD_S2MIDTEX)
			si->midtexture = READLONG(get);
	}

	save_p = get;
}

//
// Thinkers
//

typedef enum
{
	MD_SPAWNPOINT  = 0x0000001,
	MD_POS         = 0x0000002,
	MD_TYPE        = 0x0000004,
	MD_MOM         = 0x0000008,
	MD_RADIUS      = 0x0000010,
	MD_HEIGHT      = 0x0000020,
	MD_FLAGS       = 0x0000040,
	MD_HEALTH      = 0x0000080,
	MD_RTIME       = 0x0000100,
	MD_STATE       = 0x0000200,
	MD_TICS        = 0x0000400,
	MD_SPRITE      = 0x0000800,
	MD_FRAME       = 0x0001000,
	MD_EFLAGS      = 0x0002000,
	MD_PLAYER      = 0x0004000,
	MD_MOVEDIR     = 0x0008000,
	MD_MOVECOUNT   = 0x0010000,
	MD_THRESHOLD   = 0x0020000,
	MD_LASTLOOK    = 0x0040000,
	MD_TARGET      = 0x0080000,
	MD_TRACER      = 0x0100000,
	MD_FRICTION    = 0x0200000,
	MD_MOVEFACTOR  = 0x0400000,
	MD_FLAGS2      = 0x0800000,
	MD_FUSE        = 0x1000000,
	MD_WATERTOP    = 0x2000000,
	MD_WATERBOTTOM = 0x4000000,
	MD_SCALE       = 0x8000000,
	MD_DSCALE      = 0x10000000,
} mobj_diff_t;

typedef enum
{
	tc_mobj,
	tc_ceiling,
	tc_floor,
	tc_flash,
	tc_strobe,
	tc_glow,
	tc_fireflicker,
	tc_thwomp,
	tc_camerascanner,
	tc_elevator,
	tc_continuousfalling,
	tc_bouncecheese,
	tc_startcrumble,
	tc_marioblock,
	tc_spikesector,
	tc_floatsector,
	tc_bridgethinker,
	tc_crushceiling,
	tc_scroll,
	tc_friction,
	tc_pusher,
	tc_laserflash,
	tc_lightfade,
	tc_executor,
	tc_raisesector,
	tc_noenemies,
	tc_eachtime,
	tc_disappear,
#ifdef POLYOBJECTS
	tc_polyrotate, // haleyjd 03/26/06: polyobjects
	tc_polymove,
	tc_polywaypoint,
	tc_polyslidedoor,
	tc_polyswingdoor,
	tc_polyflag,
#endif
	tc_end
} specials_e;

//
// SaveSpecialLevelThinker
//
// Saves a levelspecthink_t thinker
//
static void SaveSpecialLevelThinker(thinker_t *th, byte type)
{
	levelspecthink_t *specthinker;
	WRITEBYTE(save_p, type);
	specthinker = M_Memcpy(save_p, th, sizeof (*specthinker));
	save_p += sizeof (*specthinker);
	specthinker->sector = (sector_t *)(specthinker->sector - sectors);
	specthinker->sourceline = (line_t *)(specthinker->sourceline - lines);
	if (specthinker->activator)
		specthinker->activator = (mobj_t *)(size_t)specthinker->activator->mobjnum;
}

//
// P_NetArchiveThinkers
//
//
static void P_NetArchiveThinkers(void)
{
	thinker_t *th;
	mobj_t *mobj;
	ULONG diff;

	// save off the current thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)th;

			// Ignore stationary hoops - these will be respawned from mapthings.
			if (mobj->type == MT_HOOP)
				continue;

			// These are NEVER saved.
			if (mobj->type == MT_HOOPCOLLIDE)
				continue;

			// This hoop has already been collected.
			if (mobj->type == MT_HOOPCENTER && mobj->threshold == 4242)
				continue;

			if (mobj->spawnpoint && (mobj->info->doomednum != -1 || mobj->type == MT_HOOPCENTER))
			{
				// spawnpoint is not modified but we must save it since it is an identifier
				diff = MD_SPAWNPOINT;

				if (mobj->type != MT_HOOPCENTER)
				{
					if ((mobj->x != mobj->spawnpoint->x << FRACBITS) ||
						(mobj->y != mobj->spawnpoint->y << FRACBITS) ||
						(mobj->angle != (angle_t)(ANG45 * (mobj->spawnpoint->angle/45))))
						diff |= MD_POS;

					if (mobj->info->doomednum != mobj->spawnpoint->type)
						diff |= MD_TYPE;
				}
			}
			else
				diff = MD_POS | MD_TYPE; // not a map spawned thing so make it from scratch

			// not the default but the most probable
			if (mobj->momx != 0 || mobj->momy != 0 || mobj->momz != 0)
				diff |= MD_MOM;
			if (mobj->radius != mobj->info->radius)
				diff |= MD_RADIUS;
			if (mobj->height != mobj->info->height)
				diff |= MD_HEIGHT;
			if (mobj->flags != mobj->info->flags)
				diff |= MD_FLAGS;
			diff |= MD_FLAGS2; // Force saving of flags2
			if (mobj->health != mobj->info->spawnhealth)
				diff |= MD_HEALTH;
			if (mobj->reactiontime != mobj->info->reactiontime)
				diff |= MD_RTIME;
			if ((statenum_t)(mobj->state-states) != mobj->info->spawnstate)
				diff |= MD_STATE;
			if (mobj->tics != mobj->state->tics)
				diff |= MD_TICS;
			if (mobj->sprite != mobj->state->sprite)
				diff |= MD_SPRITE;
			if (mobj->frame != mobj->state->frame)
				diff |= MD_FRAME;
			if (mobj->eflags)
				diff |= MD_EFLAGS;
			if (mobj->player)
				diff |= MD_PLAYER;

			if (mobj->movedir)
				diff |= MD_MOVEDIR;
			if (mobj->movecount)
				diff |= MD_MOVECOUNT;
			if (mobj->threshold)
				diff |= MD_THRESHOLD;
			if (mobj->lastlook != -1)
				diff |= MD_LASTLOOK;
			if (mobj->target)
				diff |= MD_TARGET;
			if (mobj->tracer)
				diff |= MD_TRACER;
			if (mobj->friction != ORIG_FRICTION)
				diff |= MD_FRICTION;
			if (mobj->movefactor != ORIG_FRICTION_FACTOR)
				diff |= MD_MOVEFACTOR;
			if (mobj->fuse)
				diff |= MD_FUSE;
			if (mobj->watertop)
				diff |= MD_WATERTOP;
			if (mobj->waterbottom)
				diff |= MD_WATERBOTTOM;
			if (mobj->scale != 100)
				diff |= MD_SCALE;
			if (mobj->destscale != mobj->scale)
				diff |= MD_DSCALE;

			WRITEBYTE(save_p, tc_mobj);
			WRITEULONG(save_p, diff);

			// save pointer, at load time we will search this pointer to reinitilize pointers
			WRITEULONG(save_p, (ULONG)(size_t)mobj);

			WRITEFIXED(save_p, mobj->z); // Force this so 3dfloor problems don't arise.
			WRITEFIXED(save_p, mobj->floorz);
			WRITEFIXED(save_p, mobj->ceilingz);

			if (diff & MD_SPAWNPOINT)
			{
				size_t z;

				for (z = 0; z < nummapthings; z++)
					if (&mapthings[z] == mobj->spawnpoint)
						WRITESHORT(save_p, z);

				if (mobj->type == MT_HOOPCENTER)
					continue;
			}

			if (diff & MD_TYPE)
				WRITEULONG(save_p, mobj->type);
			if (diff & MD_POS)
			{
				WRITEFIXED(save_p, mobj->x);
				WRITEFIXED(save_p, mobj->y);
				WRITEANGLE(save_p, mobj->angle);
			}
			if (diff & MD_MOM)
			{
				WRITEFIXED(save_p, mobj->momx);
				WRITEFIXED(save_p, mobj->momy);
				WRITEFIXED(save_p, mobj->momz);
			}
			if (diff & MD_RADIUS)
				WRITEFIXED(save_p, mobj->radius);
			if (diff & MD_HEIGHT)
				WRITEFIXED(save_p, mobj->height);
			if (diff & MD_FLAGS)
				WRITELONG(save_p, mobj->flags);
			if (diff & MD_FLAGS2)
				WRITELONG(save_p, mobj->flags2);
			if (diff & MD_HEALTH)
				WRITELONG(save_p, mobj->health);
			if (diff & MD_RTIME)
				WRITELONG(save_p, mobj->reactiontime);
			if (diff & MD_STATE)
				WRITEUSHORT(save_p, mobj->state-states);
			if (diff & MD_TICS)
				WRITELONG(save_p, mobj->tics);
			if (diff & MD_SPRITE)
				WRITEUSHORT(save_p, mobj->sprite);
			if (diff & MD_FRAME)
				WRITEULONG(save_p, mobj->frame);
			if (diff & MD_EFLAGS)
				WRITEULONG(save_p, mobj->eflags);
			if (diff & MD_PLAYER)
				WRITEBYTE(save_p, mobj->player-players);
			if (diff & MD_MOVEDIR)
				WRITEANGLE(save_p, mobj->movedir);
			if (diff & MD_MOVECOUNT)
				WRITELONG(save_p, mobj->movecount);
			if (diff & MD_THRESHOLD)
				WRITELONG(save_p, mobj->threshold);
			if (diff & MD_LASTLOOK)
				WRITELONG(save_p, mobj->lastlook);
			if (diff & MD_TARGET)
				WRITEULONG(save_p, mobj->target->mobjnum);
			if (diff & MD_TRACER)
				WRITEULONG(save_p, mobj->tracer->mobjnum);
			if (diff & MD_FRICTION)
				WRITELONG(save_p, mobj->friction);
			if (diff & MD_MOVEFACTOR)
				WRITELONG(save_p, mobj->movefactor);
			if (diff & MD_FUSE)
				WRITELONG(save_p, mobj->fuse);
			if (diff & MD_WATERTOP)
				WRITELONG(save_p, mobj->watertop);
			if (diff & MD_WATERBOTTOM)
				WRITELONG(save_p, mobj->waterbottom);
			if (diff & MD_SCALE)
				WRITEUSHORT(save_p, mobj->scale);
			if (diff & MD_DSCALE)
				WRITEUSHORT(save_p, mobj->destscale);

			WRITEBYTE(save_p, mobj->scalespeed);

			WRITEULONG(save_p, mobj->mobjnum);
		}
#ifdef PARANOIA
		else if (th->function.acp1 == (actionf_p1)P_RainThinker
			|| th->function.acp1 == (actionf_p1)P_SnowThinker);
#endif
		else if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
		{
			ceiling_t *ceiling;
			WRITEBYTE(save_p, tc_ceiling);
			ceiling = M_Memcpy(save_p, th, sizeof (*ceiling));
			save_p += sizeof (*ceiling);
			ceiling->sector = (sector_t *)(ceiling->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_CrushCeiling)
		{
			ceiling_t *ceiling;
			WRITEBYTE(save_p, tc_crushceiling);
			ceiling = M_Memcpy(save_p, th, sizeof (*ceiling));
			save_p += sizeof (*ceiling);
			ceiling->sector = (sector_t *)(ceiling->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MoveFloor)
		{
			floormove_t *savefloor;
			WRITEBYTE(save_p, tc_floor);
			savefloor = M_Memcpy(save_p, th, sizeof (*savefloor));
			save_p += sizeof (*savefloor);
			savefloor->sector = (sector_t *)(savefloor->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LightningFlash)
		{
			lightflash_t *flash;
			WRITEBYTE(save_p, tc_flash);
			flash = M_Memcpy(save_p, th, sizeof (*flash));
			save_p += sizeof (*flash);
			flash->sector = (sector_t *)(flash->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
		{
			strobe_t *strobe;
			WRITEBYTE(save_p, tc_strobe);
			strobe = M_Memcpy(save_p, th, sizeof (*strobe));
			save_p += sizeof (*strobe);
			strobe->sector = (sector_t *)(strobe->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Glow)
		{
			glow_t *glow;
			WRITEBYTE(save_p, tc_glow);
			glow = M_Memcpy(save_p, th, sizeof (*glow));
			save_p += sizeof (*glow);
			glow->sector = (sector_t *)(glow->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_FireFlicker)
		{
			fireflicker_t *fireflicker;
			WRITEBYTE(save_p, tc_fireflicker);
			fireflicker = M_Memcpy(save_p, th, sizeof (*fireflicker));
			save_p += sizeof (*fireflicker);
			fireflicker->sector = (sector_t *)(fireflicker->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MoveElevator)
		{
			elevator_t *elevator;
			WRITEBYTE(save_p, tc_elevator);
			elevator = M_Memcpy(save_p, th, sizeof (*elevator));
			save_p += sizeof (*elevator);
			elevator->sector = (sector_t *)(elevator->sector - sectors);
			elevator->sourceline = (line_t *)(elevator->sourceline - lines);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ContinuousFalling)
		{
			SaveSpecialLevelThinker(th, tc_continuousfalling);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ThwompSector)
		{
			SaveSpecialLevelThinker(th, tc_thwomp);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_NoEnemiesSector)
		{
			SaveSpecialLevelThinker(th, tc_noenemies);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_EachTimeThinker)
		{
			SaveSpecialLevelThinker(th, tc_eachtime);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_RaiseSector)
		{
			SaveSpecialLevelThinker(th, tc_raisesector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_CameraScanner)
		{
			elevator_t *elevator;
			WRITEBYTE(save_p, tc_camerascanner);
			elevator = M_Memcpy(save_p, th, sizeof (*elevator));
			save_p += sizeof (*elevator);
			elevator->sector = (sector_t *)(elevator->sector - sectors);
			elevator->actionsector = (sector_t *)(elevator->actionsector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Scroll)
		{
			WRITEBYTE(save_p, tc_scroll);
			M_Memcpy(save_p, th, sizeof (scroll_t));
			save_p += sizeof (scroll_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Friction)
		{
			WRITEBYTE(save_p, tc_friction);
			M_Memcpy(save_p, th, sizeof (friction_t));
			save_p += sizeof (friction_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Pusher)
		{
			WRITEBYTE(save_p, tc_pusher);
			M_Memcpy(save_p, th, sizeof (pusher_t));
			save_p += sizeof (pusher_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_BounceCheese)
		{
			SaveSpecialLevelThinker(th, tc_bouncecheese);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_StartCrumble)
		{
			elevator_t *elevator;
			WRITEBYTE(save_p, tc_startcrumble);
			elevator = M_Memcpy(save_p, th, sizeof (*elevator));
			save_p += sizeof (*elevator);
			elevator->sector = (sector_t *)(elevator->sector - sectors);
			elevator->sourceline = (line_t *)(elevator->sourceline - lines);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_MarioBlock)
		{
			SaveSpecialLevelThinker(th, tc_marioblock);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_SpikeSector)
		{
			SaveSpecialLevelThinker(th, tc_spikesector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_FloatSector)
		{
			SaveSpecialLevelThinker(th, tc_floatsector);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_BridgeThinker)
		{
			SaveSpecialLevelThinker(th, tc_bridgethinker);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LaserFlash)
		{
			laserthink_t *laser;
			WRITEBYTE(save_p, tc_laserflash);
			laser = M_Memcpy(save_p, th, sizeof (*laser));
			save_p += sizeof (*laser);
			laser->ffloor = (ffloor_t *)(laser->ffloor - laser->sector->ffloors);
			laser->sector = (sector_t *)(laser->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_LightFade)
		{
			lightlevel_t *ll;
			WRITEBYTE(save_p, tc_lightfade);
			ll = M_Memcpy(save_p, th, sizeof (*ll));
			save_p += sizeof (*ll);
			ll->sector = (sector_t *)(ll->sector - sectors);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_ExecutorDelay)
		{
			executor_t *e;
			WRITEBYTE(save_p, tc_executor);
			e = M_Memcpy(save_p, th, sizeof (*e));
			save_p += sizeof (*e);
			e->caller = (mobj_t *)(size_t)e->caller->mobjnum;
			e->line = (line_t *)(e->line - lines);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_Disappear)
		{
			WRITEBYTE(save_p, tc_disappear);
			M_Memcpy(save_p, th, sizeof (disappear_t));
			save_p += sizeof (disappear_t);
			continue;
		}
#ifdef POLYOBJECTS
		else if (th->function.acp1 == (actionf_p1)T_PolyObjRotate)
		{
			WRITEBYTE(save_p, tc_polyrotate);
			M_Memcpy(save_p, th, sizeof (polyrotate_t));
			save_p += sizeof (polyrotate_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjMove)
		{
			WRITEBYTE(save_p, tc_polymove);
			M_Memcpy(save_p, th, sizeof (polymove_t));
			save_p += sizeof (polymove_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjWaypoint)
		{
			WRITEBYTE(save_p, tc_polywaypoint);
			M_Memcpy(save_p, th, sizeof (polywaypoint_t));
			save_p += sizeof (polywaypoint_t);
			continue;
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyDoorSlide)
		{
			WRITEBYTE(save_p, tc_polyslidedoor);
			M_Memcpy(save_p, th, sizeof (polyslidedoor_t));
			save_p += sizeof (polyslidedoor_t);
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyDoorSwing)
		{
			WRITEBYTE(save_p, tc_polyswingdoor);
			M_Memcpy(save_p, th, sizeof (polyswingdoor_t));
			save_p += sizeof (polyswingdoor_t);
		}
		else if (th->function.acp1 == (actionf_p1)T_PolyObjFlag)
		{
			WRITEBYTE(save_p, tc_polyflag);
			M_Memcpy(save_p, th, sizeof (polymove_t));
			save_p += sizeof (polymove_t);
			continue;
		}
#endif
#ifdef PARANOIA
		else if (th->function.acv != P_RemoveThinkerDelayed) // wait garbage collection
			I_Error("unknown thinker type %p", th->function.acp1);
#endif
	}

	WRITEBYTE(save_p, tc_end);
}

// Now save the pointers, tracer and target, but at load time we must
// relink to this; the savegame contains the old position in the pointer
// field copyed in the info field temporarily, but finally we just search
// for the old position and relink to it.
static mobj_t *FindNewPosition(ULONG oldposition)
{
	thinker_t *th;
	mobj_t *mobj;

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mobj = (mobj_t *)th;
		if (mobj->mobjnum == oldposition)
			return mobj;
	}
	if (devparm)
		CONS_Printf("\2not found\n");
	DEBFILE("not found\n");
	return NULL;
}

//
// LoadSpecialLevelThinker
//
// Loads a levelspecthink_t from a save game
//
// floorOrCeiling:
//		0 - Don't set
//		1 - Floor Only
//		2 - Ceiling Only
//		3 - Both
//
static void LoadSpecialLevelThinker(int floorOrCeiling, actionf_p1 thinker)
{
	levelspecthink_t *specthinker = M_Memcpy(Z_Malloc(sizeof (*specthinker), PU_LEVEL, NULL), save_p, sizeof (levelspecthink_t));
	save_p += sizeof (levelspecthink_t);
	specthinker->sector = &sectors[(size_t)specthinker->sector];
	specthinker->sourceline = &lines[(size_t)specthinker->sourceline];

	// NOTE: Unarchiving of 'activator' must be done at bottom of P_NetUnArchiveThinkers!

	if (floorOrCeiling & 2)
		specthinker->sector->ceilingdata = specthinker;
	if (floorOrCeiling & 1)
		specthinker->sector->floordata = specthinker;

	specthinker->thinker.function.acp1 = thinker;
	P_AddThinker(&specthinker->thinker);
}

//
// P_NetUnArchiveThinkers
//
static void P_NetUnArchiveThinkers(void)
{
	thinker_t *currentthinker;
	thinker_t *next;
	mobj_t *mobj;
	ULONG diff;
	int i;
	byte tclass;
	boolean restoreNum = false;
	fixed_t z, floorz, ceilingz;

	// remove all the current thinkers
	currentthinker = thinkercap.next;
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = next)
	{
		next = currentthinker->next;

		mobj = (mobj_t *)currentthinker;
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
			P_RemoveSavegameMobj((mobj_t *)currentthinker); // item isn't saved, don't remove it
		else
			Z_Free(currentthinker);
	}

	// we don't want the removed mobjs to come back
	iquetail = iquehead = 0;
	P_InitThinkers();

	// read in saved thinkers
	for (;;)
	{
		tclass = READBYTE(save_p);

		if (tclass == tc_end)
			break; // leave the saved thinker reading loop

		switch (tclass)
		{
			case tc_mobj:
				diff = READULONG(save_p);

				next = (void *)(size_t)READULONG(save_p);

				z = READFIXED(save_p); // Force this so 3dfloor problems don't arise.
				floorz = READFIXED(save_p);
				ceilingz = READFIXED(save_p);

				if (diff & MD_SPAWNPOINT)
				{
					short spawnpointnum = READSHORT(save_p);

					if (mapthings[spawnpointnum].type == 1705) // NiGHTS Hoop special case
					{
						P_SpawnHoopsAndRings(&mapthings[spawnpointnum]);
						continue;
					}

					mobj = Z_Calloc(sizeof (*mobj), PU_LEVEL, NULL);

					mobj->spawnpoint = &mapthings[spawnpointnum];
					mapthings[spawnpointnum].mobj = mobj;
				}
				else
					mobj = Z_Calloc(sizeof (*mobj), PU_LEVEL, NULL);

				mobj->z = z;
				mobj->floorz = floorz;
				mobj->ceilingz = ceilingz;

				if (diff & MD_TYPE)
					mobj->type = READULONG(save_p);
				else
				{
					for (i = 0; i < NUMMOBJTYPES; i++)
						if (mobj->spawnpoint && mobj->spawnpoint->type == mobjinfo[i].doomednum)
							break;
					if (i == NUMMOBJTYPES)
					{
						CONS_Printf("found mobj with unknown map thing type %d\n",
							mobj->spawnpoint->type);
						I_Error("Savegame corrupted");
					}
					mobj->type = i;
				}
				mobj->info = &mobjinfo[mobj->type];
				if (diff & MD_POS)
				{
					mobj->x = READFIXED(save_p);
					mobj->y = READFIXED(save_p);
					mobj->angle = READANGLE(save_p);
				}
				else
				{
					mobj->x = mobj->spawnpoint->x << FRACBITS;
					mobj->y = mobj->spawnpoint->y << FRACBITS;
					mobj->angle = ANG45 * (mobj->spawnpoint->angle/45); /// \bug unknown
				}
				if (diff & MD_MOM)
				{
					mobj->momx = READFIXED(save_p);
					mobj->momy = READFIXED(save_p);
					mobj->momz = READFIXED(save_p);
				} // otherwise they're zero, and the memset took care of it

				if (diff & MD_RADIUS)
					mobj->radius = READFIXED(save_p);
				else
					mobj->radius = mobj->info->radius;
				if (diff & MD_HEIGHT)
					mobj->height = READFIXED(save_p);
				else
					mobj->height = mobj->info->height;
				if (diff & MD_FLAGS)
					mobj->flags = READLONG(save_p);
				else
					mobj->flags = mobj->info->flags;
				mobj->flags2 = READLONG(save_p);
				if (diff & MD_HEALTH)
					mobj->health = READLONG(save_p);
				else
					mobj->health = mobj->info->spawnhealth;
				if (diff & MD_RTIME)
					mobj->reactiontime = READLONG(save_p);
				else
					mobj->reactiontime = mobj->info->reactiontime;

				if (diff & MD_STATE)
					mobj->state = &states[READUSHORT(save_p)];
				else
					mobj->state = &states[mobj->info->spawnstate];
				if (diff & MD_TICS)
					mobj->tics = READLONG(save_p);
				else
					mobj->tics = mobj->state->tics;
				if (diff & MD_SPRITE)
					mobj->sprite = READUSHORT(save_p);
				else
					mobj->sprite = mobj->state->sprite;
				if (diff & MD_FRAME)
					mobj->frame = READULONG(save_p);
				else
					mobj->frame = mobj->state->frame;
				if (diff & MD_EFLAGS)
					mobj->eflags = READULONG(save_p);
				if (diff & MD_PLAYER)
				{
					i = READBYTE(save_p);
					mobj->player = &players[i];
					mobj->player->mo = mobj;
					// added for angle prediction
					if (consoleplayer == i)
						localangle = mobj->angle;
					if (secondarydisplayplayer == i)
						localangle2 = mobj->angle;
				}
				if (diff & MD_MOVEDIR)
					mobj->movedir = READANGLE(save_p);
				if (diff & MD_MOVECOUNT)
					mobj->movecount = READLONG(save_p);
				if (diff & MD_THRESHOLD)
					mobj->threshold = READLONG(save_p);
				if (diff & MD_LASTLOOK)
					mobj->lastlook = READLONG(save_p);
				else
					mobj->lastlook = -1;
				if (diff & MD_TARGET)
					mobj->target = (mobj_t *)(size_t)READULONG(save_p);
				if (diff & MD_TRACER)
					mobj->tracer = (mobj_t *)(size_t)READULONG(save_p);
				if (diff & MD_FRICTION)
					mobj->friction = READLONG(save_p);
				else
					mobj->friction = ORIG_FRICTION;
				if (diff & MD_MOVEFACTOR)
					mobj->movefactor = READLONG(save_p);
				else
					mobj->movefactor = ORIG_FRICTION_FACTOR;
				if (diff & MD_FUSE)
					mobj->fuse = READLONG (save_p);
				if (diff & MD_WATERTOP)
					mobj->watertop = READLONG(save_p);
				if (diff & MD_WATERBOTTOM)
					mobj->waterbottom = READLONG(save_p);
				if (diff & MD_SCALE)
					mobj->scale = READUSHORT(save_p);
				else
					mobj->scale = 100;
				if (diff & MD_DSCALE)
					mobj->destscale = READUSHORT(save_p);
				else
					mobj->destscale = mobj->scale;

				mobj->scalespeed = READBYTE(save_p);

				// now set deductable field
				/// \todo save this too
				mobj->skin = NULL;

				// set sprev, snext, bprev, bnext, subsector
				P_SetThingPosition(mobj);

				mobj->mobjnum = READULONG(save_p);

				if (mobj->player)
				{
					if (mobj->eflags & MFE_VERTICALFLIP)
						mobj->player->viewz = mobj->z + mobj->height - mobj->player->viewheight;
					else
						mobj->player->viewz = mobj->player->mo->z + mobj->player->viewheight;
				}

				mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
				P_AddThinker(&mobj->thinker);

				mobj->info = (mobjinfo_t *)next; // temporarily, set when leave this function
				break;

			case tc_ceiling:
				{
					ceiling_t *ceiling = M_Memcpy(Z_Malloc(sizeof (*ceiling), PU_LEVEL, NULL), save_p, sizeof (*ceiling));
					save_p += sizeof (*ceiling);
					ceiling->sector = &sectors[(size_t)ceiling->sector];
					ceiling->sector->ceilingdata = ceiling;

					if (ceiling->thinker.function.acp1)
						ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

					P_AddThinker(&ceiling->thinker);
				}
				break;

			case tc_crushceiling:
				{
					ceiling_t *ceiling = M_Memcpy(Z_Malloc(sizeof (*ceiling), PU_LEVEL, NULL), save_p, sizeof (*ceiling));
					save_p += sizeof (*ceiling);
					ceiling->sector = &sectors[(size_t)ceiling->sector];
					ceiling->sector->ceilingdata = ceiling;

					if (ceiling->thinker.function.acp1)
						ceiling->thinker.function.acp1 = (actionf_p1)T_CrushCeiling;

					P_AddThinker(&ceiling->thinker);
				}
				break;

			case tc_floor:
				{
					floormove_t *loadfloor = M_Memcpy(Z_Malloc(sizeof (*loadfloor), PU_LEVEL, NULL), save_p, sizeof (*loadfloor));
					save_p += sizeof (*loadfloor);
					loadfloor->sector = &sectors[(size_t)loadfloor->sector];
					loadfloor->sector->floordata = loadfloor;
					loadfloor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
					P_AddThinker(&loadfloor->thinker);
				}
				break;

			case tc_flash:
				{
					lightflash_t *flash = M_Memcpy(Z_Malloc(sizeof (*flash), PU_LEVEL, NULL), save_p, sizeof (*flash));
					save_p += sizeof (*flash);
					flash->sector = &sectors[(size_t)flash->sector];
					flash->sector->lightingdata = flash;
					flash->thinker.function.acp1 = (actionf_p1)T_LightningFlash;
					P_AddThinker(&flash->thinker);
				}
				break;

			case tc_strobe:
				{
					strobe_t *strobe = M_Memcpy(Z_Malloc(sizeof (*strobe), PU_LEVEL, NULL), save_p, sizeof (*strobe));
					save_p += sizeof (*strobe);
					strobe->sector = &sectors[(size_t)strobe->sector];
					strobe->sector->lightingdata = strobe;
					strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
					P_AddThinker(&strobe->thinker);
				}
				break;

			case tc_glow:
				{
					glow_t *glow = M_Memcpy(Z_Malloc(sizeof (*glow), PU_LEVEL, NULL), save_p, sizeof (*glow));
					save_p += sizeof (*glow);
					glow->sector = &sectors[(size_t)glow->sector];
					glow->sector->lightingdata = glow;
					glow->thinker.function.acp1 = (actionf_p1)T_Glow;
					P_AddThinker(&glow->thinker);
				}
				break;

			case tc_fireflicker:
				{
					fireflicker_t *ff = M_Memcpy(Z_Malloc(sizeof (*ff), PU_LEVEL, NULL), save_p, sizeof (*ff));
					save_p += sizeof (*ff);
					ff->sector = &sectors[(size_t)ff->sector];
					ff->sector->lightingdata = ff;
					ff->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
					P_AddThinker(&ff->thinker);
				}
				break;

			case tc_elevator:
				{
					elevator_t *elevator = M_Memcpy(Z_Malloc(sizeof (*elevator), PU_LEVEL, NULL), save_p, sizeof (elevator_t));
					save_p += sizeof (elevator_t);
					elevator->sector = &sectors[(size_t)elevator->sector];
					elevator->sector->floordata = elevator;
					elevator->sector->ceilingdata = elevator;
					elevator->sourceline = &lines[(size_t)elevator->sourceline];
					elevator->thinker.function.acp1 = (actionf_p1)T_MoveElevator;
					P_AddThinker(&elevator->thinker);
				}
				break;

			case tc_continuousfalling:
				LoadSpecialLevelThinker(3, (actionf_p1)T_ContinuousFalling);
				break;

			case tc_thwomp:
				LoadSpecialLevelThinker(3, (actionf_p1)T_ThwompSector);
				break;

			case tc_noenemies:
				LoadSpecialLevelThinker(0, (actionf_p1)T_NoEnemiesSector);
				break;

			case tc_eachtime:
				LoadSpecialLevelThinker(0, (actionf_p1)T_EachTimeThinker);
				break;

			case tc_raisesector:
				LoadSpecialLevelThinker(0, (actionf_p1)T_RaiseSector);
				break;

			/// \todo rewrite all the shit that uses an elevator_t but isn't an elevator
			/// \note working on it!
			case tc_camerascanner:
				{
					elevator_t *elevator = M_Memcpy(Z_Malloc(sizeof (*elevator), PU_LEVEL, NULL), save_p, sizeof (elevator_t));
					save_p += sizeof (elevator_t);
					elevator->sector = &sectors[(size_t)elevator->sector];
					elevator->actionsector = &sectors[(size_t)elevator->actionsector];
					elevator->thinker.function.acp1 = (actionf_p1)T_CameraScanner;
					P_AddThinker(&elevator->thinker);
				}
				break;

			case tc_bouncecheese:
				LoadSpecialLevelThinker(2, (actionf_p1)T_BounceCheese);
				break;

			case tc_startcrumble:
				{
					elevator_t *elevator = M_Memcpy(Z_Malloc(sizeof (*elevator), PU_LEVEL, NULL), save_p, sizeof (elevator_t));
					save_p += sizeof (elevator_t);
					elevator->sector = &sectors[(size_t)elevator->sector];
					elevator->sourceline = &lines[(size_t)elevator->sourceline];
					elevator->sector->floordata = elevator;
					elevator->thinker.function.acp1 = (actionf_p1)T_StartCrumble;
					P_AddThinker(&elevator->thinker);
				}
				break;

			case tc_marioblock:
				LoadSpecialLevelThinker(3, (actionf_p1)T_MarioBlock);
				break;

			case tc_spikesector:
				LoadSpecialLevelThinker(0, (actionf_p1)T_SpikeSector);
				break;

			case tc_floatsector:
				LoadSpecialLevelThinker(0, (actionf_p1)T_FloatSector);
				break;

			case tc_bridgethinker:
				LoadSpecialLevelThinker(3, (actionf_p1)T_BridgeThinker);
				break;

			case tc_laserflash:
				{
					laserthink_t *laser = M_Memcpy(Z_Malloc(sizeof (*laser), PU_LEVEL, NULL), save_p, sizeof (laserthink_t));
					save_p += sizeof (laserthink_t);
					laser->sector = &sectors[(size_t)laser->sector];
					laser->ffloor = &(laser->sector->ffloors)[(size_t)laser->ffloor];
					laser->thinker.function.acp1 = (actionf_p1)T_LaserFlash;
					P_AddThinker(&laser->thinker);
				}
				break;

			case tc_lightfade:
				{
					lightlevel_t *ll = M_Memcpy(Z_Malloc(sizeof (*ll), PU_LEVEL, NULL), save_p, sizeof (lightlevel_t));
					save_p += sizeof (lightlevel_t);
					ll->sector = &sectors[(size_t)ll->sector];
					ll->sector->lightingdata = ll;
					ll->thinker.function.acp1 = (actionf_p1)T_LightFade;
					P_AddThinker(&ll->thinker);
				}
				break;

			case tc_executor:
				{
					executor_t *e = M_Memcpy(Z_Malloc(sizeof (*e), PU_LEVEL, NULL), save_p, sizeof (executor_t));
					save_p += sizeof (executor_t);
					e->line = &lines[(size_t)e->line];
					restoreNum = true;
//					e->caller = FindNewPosition((size_t)e->caller); // Restored further down
					e->thinker.function.acp1 = (actionf_p1)T_ExecutorDelay;
					P_AddThinker(&e->thinker);
				}
				break;

			case tc_disappear:
				{
					disappear_t *disappear = M_Memcpy(Z_Malloc(sizeof (*disappear), PU_LEVEL, NULL), save_p, sizeof (disappear_t));
					save_p += sizeof (disappear_t);
					disappear->thinker.function.acp1 = (actionf_p1)T_Disappear;
					P_AddThinker(&disappear->thinker);
				}
				break;
#ifdef POLYOBJECTS
			case tc_polyrotate:
				{
					polyrotate_t *polyrot = M_Memcpy(Z_Malloc(sizeof (*polyrot), PU_LEVEL, NULL), save_p, sizeof (polyrotate_t));
					save_p += sizeof (polyrotate_t);
					polyrot->thinker.function.acp1 = (actionf_p1)T_PolyObjRotate;
					P_AddThinker(&polyrot->thinker);
				}
				break;

			case tc_polymove:
				{
					polymove_t *polymove = M_Memcpy(Z_Malloc(sizeof (*polymove), PU_LEVEL, NULL), save_p, sizeof (polymove_t));
					save_p += sizeof (polymove_t);
					polymove->thinker.function.acp1 = (actionf_p1)T_PolyObjMove;
					P_AddThinker(&polymove->thinker);
				}
				break;

			case tc_polywaypoint:
				{
					polywaypoint_t *polywaypoint = M_Memcpy(Z_Malloc(sizeof (*polywaypoint), PU_LEVEL, NULL), save_p, sizeof (polywaypoint_t));
					save_p += sizeof (polywaypoint_t);
					polywaypoint->thinker.function.acp1 = (actionf_p1)T_PolyObjWaypoint;
					P_AddThinker(&polywaypoint->thinker);
				}
				break;

			case tc_polyslidedoor:
				{
					polyslidedoor_t *psldoor = M_Memcpy(Z_Malloc(sizeof (*psldoor), PU_LEVEL, NULL), save_p, sizeof (polyslidedoor_t));
					save_p += sizeof (polyslidedoor_t);
					psldoor->thinker.function.acp1 = (actionf_p1)T_PolyDoorSlide;
					P_AddThinker(&psldoor->thinker);
				}
				break;

			case tc_polyswingdoor:
				{
					polyswingdoor_t *pswdoor = M_Memcpy(Z_Malloc(sizeof (*pswdoor), PU_LEVEL, NULL), save_p, sizeof (polyswingdoor_t));
					save_p += sizeof (polyswingdoor_t);
					pswdoor->thinker.function.acp1 = (actionf_p1)T_PolyDoorSwing;
					P_AddThinker(&pswdoor->thinker);
				}
				break;
			case tc_polyflag:
				{
					polymove_t *polyflag = M_Memcpy(Z_Malloc(sizeof (*polyflag), PU_LEVEL, NULL), save_p, sizeof (polymove_t));
					save_p += sizeof (polymove_t);
					polyflag->thinker.function.acp1 = (actionf_p1)T_PolyObjFlag;
					P_AddThinker(&polyflag->thinker);
				}
				break;
#endif
			case tc_scroll:
				{
					scroll_t *scroll = M_Memcpy(Z_Malloc(sizeof (*scroll), PU_LEVEL, NULL), save_p, sizeof (scroll_t));
					save_p += sizeof (scroll_t);
					scroll->thinker.function.acp1 = (actionf_p1)T_Scroll;
					P_AddThinker(&scroll->thinker);
				}
				break;

			case tc_friction:
				{
					friction_t *friction = M_Memcpy(Z_Malloc(sizeof (*friction), PU_LEVEL, NULL), save_p, sizeof (friction_t));
					save_p += sizeof (friction_t);
					friction->thinker.function.acp1 = (actionf_p1)T_Friction;
					P_AddThinker(&friction->thinker);
				}
				break;

			case tc_pusher:
				{
					pusher_t *pusher = M_Memcpy(Z_Malloc(sizeof (*pusher), PU_LEVEL, NULL), save_p, sizeof (pusher_t));
					save_p += sizeof (pusher_t);
					pusher->thinker.function.acp1 = (actionf_p1)T_Pusher;
					pusher->source = P_GetPushThing(pusher->affectee);
					P_AddThinker(&pusher->thinker);
				}
				break;

			default:
				I_Error("P_UnarchiveSpecials: Unknown tclass %d in savegame", tclass);
		}
	}

	if (restoreNum)
	{
		for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
			currentthinker = currentthinker->next)
		{
			if (currentthinker->function.acp1 == (actionf_p1)T_ExecutorDelay)
			{
				((executor_t *)currentthinker)->caller = FindNewPosition((ULONG)(size_t)((executor_t *)currentthinker)->caller);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// haleyjd 03/26/06: PolyObject saving code
//
#ifdef POLYOBJECTS
static inline void P_ArchivePolyObj(polyobj_t *po)
{
	memcpy(save_p, &po->id, sizeof(po->id));
	save_p += sizeof(po->id);

	memcpy(save_p, &po->angle, sizeof(po->angle));
	save_p += sizeof(po->angle);

	memcpy(save_p, &po->spawnSpot, sizeof(po->spawnSpot));
	save_p += sizeof(po->spawnSpot);
}

static inline void P_UnArchivePolyObj(polyobj_t *po)
{
	int id;
	unsigned int angle;
	degenmobj_t spawnSpot;

	// nullify all polyobject thinker pointers;
	// the thinkers themselves will fight over who gets the field
	// when they first start to run.
	po->thinker = NULL;

	memcpy(&id, save_p, sizeof(id));
	save_p += sizeof(id);

	memcpy(&angle, save_p, sizeof(angle));
	save_p += sizeof(angle);

	memcpy(&spawnSpot, save_p, sizeof(spawnSpot));
	save_p += sizeof(spawnSpot);

	// if the object is bad or isn't in the id hash, we can do nothing more
	// with it, so return now
	if (po->isBad || po != Polyobj_GetForNum(id))
		return;

	// rotate and translate polyobject
	Polyobj_MoveOnLoad(po, angle, spawnSpot.x, spawnSpot.y);
}

static inline void P_ArchivePolyObjects(void)
{
	int i;

	// save number of polyobjects
	memcpy(save_p, &numPolyObjects, sizeof(numPolyObjects));
	save_p += sizeof(numPolyObjects);

	for (i = 0; i < numPolyObjects; ++i)
		P_ArchivePolyObj(&PolyObjects[i]);
}

static inline void P_UnArchivePolyObjects(void)
{
	int i, numSavedPolys;

	memcpy(&numSavedPolys, save_p, sizeof(numSavedPolys));
	save_p += sizeof(numSavedPolys);

	if (numSavedPolys != numPolyObjects)
		I_Error("P_UnArchivePolyObjects: polyobj count inconsistency\n");

	for (i = 0; i < numSavedPolys; ++i)
		P_UnArchivePolyObj(&PolyObjects[i]);
}
#endif
//
// P_FinishMobjs
//
static void P_FinishMobjs(void)
{
	thinker_t *currentthinker;
	mobj_t *mobj;

	// put info field there real value
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
		currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)currentthinker;
			mobj->info = &mobjinfo[mobj->type];
		}
	}
}

static void P_RelinkPointers(void)
{
	thinker_t *currentthinker;
	mobj_t *mobj;
	ULONG temp;

	// use info field (value = oldposition) to relink mobjs
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap;
		currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)currentthinker;

			if (mobj->type == MT_HOOP || mobj->type == MT_HOOPCOLLIDE || mobj->type == MT_HOOPCENTER)
				continue;

			if (mobj->tracer)
			{
				temp = (ULONG)(size_t)mobj->tracer;
				mobj->tracer = NULL;
				if (!P_SetTarget(&mobj->tracer, FindNewPosition(temp)))
					CONS_Printf("tracer not found on %d\n", mobj->type);
			}
			if (mobj->target)
			{
				temp = (ULONG)(size_t)mobj->target;
				mobj->target = NULL;
				if (!P_SetTarget(&mobj->target, FindNewPosition(temp)))
					CONS_Printf("target not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->capsule)
			{
				temp = (ULONG)(size_t)mobj->player->capsule;
				mobj->player->capsule = NULL;
				if (!P_SetTarget(&mobj->player->capsule, FindNewPosition(temp)))
					CONS_Printf("capsule not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->axis1)
			{
				temp = (ULONG)(size_t)mobj->player->axis1;
				mobj->player->axis1 = NULL;
				if (!P_SetTarget(&mobj->player->axis1, FindNewPosition(temp)))
					CONS_Printf("axis1 not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->axis2)
			{
				temp = (ULONG)(size_t)mobj->player->axis2;
				mobj->player->axis2 = NULL;
				if (!P_SetTarget(&mobj->player->axis2, FindNewPosition(temp)))
					CONS_Printf("axis2 not found on %d\n", mobj->type);
			}
			if (mobj->player && mobj->player->awayviewmobj)
			{
				temp = (ULONG)(size_t)mobj->player->awayviewmobj;
				mobj->player->awayviewmobj = NULL;
				if (!P_SetTarget(&mobj->player->awayviewmobj, FindNewPosition(temp)))
					CONS_Printf("awayviewmobj not found on %d\n", mobj->type);
			}
		}
	}
}

//
// P_NetArchiveSpecials
//
static void P_NetArchiveSpecials(void)
{
	size_t i, z;

	// itemrespawn queue for deathmatch
	i = iquetail;
	while (iquehead != i)
	{
		for (z = 0; z < nummapthings; z++)
		{
			if (&mapthings[z] == itemrespawnque[i])
			{
				WRITELONG(save_p, z);
				break;
			}
		}
		WRITELONG(save_p, itemrespawntime[i]);
		i = (i + 1) & (ITEMQUESIZE-1);
	}

	// end delimiter
	WRITEULONG(save_p, 0xffffffff);

	// Sky number
	WRITELONG(save_p, globallevelskynum);

	// Current global weather type
	WRITEBYTE(save_p, globalweather);
}

//
// P_NetUnArchiveSpecials
//
static void P_NetUnArchiveSpecials(void)
{
	size_t i;
	int j;

	// BP: added save itemrespawn queue for deathmatch
	iquetail = iquehead = 0;
	while ((i = READULONG(save_p)) != 0xffffffff)
	{
		itemrespawnque[iquehead] = &mapthings[i];
		itemrespawntime[iquehead++] = READLONG(save_p);
	}

	j = READLONG(save_p);
	if (j != globallevelskynum)
	{
		globallevelskynum = levelskynum = j;
		P_SetupLevelSky(levelskynum);
	}

	globalweather = READBYTE(save_p);

	if (globalweather)
	{
		if (curWeather == globalweather)
			curWeather = PRECIP_NONE;

		P_SwitchWeather(globalweather);
	}
	else // PRECIP_NONE
	{
		if (curWeather != PRECIP_NONE)
			P_SwitchWeather(globalweather);
	}
}

// =======================================================================
//          Misc
// =======================================================================
static void P_ArchiveMisc(void)
{
	if (gamecomplete)
		WRITESHORT(save_p, 24);
	else
		WRITESHORT(save_p, gamemap);

	lastmapsaved = gamemap;

	WRITEUSHORT(save_p, emeralds+357);
	WRITESTRINGN(save_p, timeattackfolder, sizeof(timeattackfolder));
}

static boolean P_UnArchiveSPGame(short mapoverride)
{
	char testname[sizeof(timeattackfolder)];

	gamemap = READSHORT(save_p);

	if (mapoverride != 0)
	{
		gamemap = mapoverride;
		gamecomplete = true;
	}
	else
		gamecomplete = false;

	lastmapsaved = gamemap;

	tokenlist = 0;
	token = 0;

	savedata.emeralds = (USHORT)(READUSHORT(save_p)-357);

	READSTRINGN(save_p, testname, sizeof(testname));

	if (strcmp(testname, timeattackfolder))
	{
		if (modifiedgame)
			I_Error("Save game not for this modification.");
		else
			I_Error("This save file is for a particular mod, it cannot be used with the regular game.");
	}

	memset(playeringame, 0, sizeof(*playeringame));
	playeringame[consoleplayer] = true;

	P_UnArchivePlayer();

	G_DeferedInitNew(false, G_BuildMapName(gamemap), savedata.skin, false, true);
	COM_BufAddText("dummyconsvar 1\n"); // G_DeferedInitNew doesn't do this

	return true;
}

static void P_NetArchiveMisc(void)
{
	ULONG pig = 0;
	int i, j;

	WRITESHORT(save_p, gamemap);
	WRITESHORT(save_p, gamestate);

	WRITEULONG(save_p, tokenlist);

	for (i = 0; i < MAXPLAYERS; i++)
		pig |= (playeringame[i] != 0)<<i;

	WRITEULONG(save_p, pig);

	for (i = 0; i < MAXPLAYERS; i++)
		for (j = 0; j < S_PLAY_SUPERTRANS9+1; j++)
			WRITELONG(save_p, playerstatetics[i][j]);

	WRITEULONG(save_p, leveltime);
	WRITEULONG(save_p, totalrings);
	WRITESHORT(save_p, lastmap);

	WRITEUSHORT(save_p, emeralds);

	WRITEULONG(save_p, token);
	WRITELONG(save_p, sstimer);
	WRITEULONG(save_p, bluescore);
	WRITEULONG(save_p, redscore);
	WRITEULONG(save_p, blueflagloose);
	WRITEULONG(save_p, redflagloose);

	WRITESHORT(save_p, autobalance);
	WRITESHORT(save_p, teamscramble);

	for (i = 0; i < MAXPLAYERS; i++)
		WRITESHORT(save_p, scrambleplayers[i]);

	for (i = 0; i < MAXPLAYERS; i++)
		WRITESHORT(save_p, scrambleteams[i]);

	WRITESHORT(save_p, scrambletotal);
	WRITESHORT(save_p, scramblecount);

	WRITEULONG(save_p, countdown);
	WRITEULONG(save_p, countdown2);

	WRITEFIXED(save_p, gravity);

	WRITEULONG(save_p, countdowntimer);
	WRITEBYTE(save_p, countdowntimeup);

	WRITEBYTE(save_p, P_GetRandIndex());

	WRITELONG(save_p, matchtype);
	WRITELONG(save_p, tagtype);

	WRITEULONG(save_p, hidetime);

	// Is it paused?
	if (paused)
		WRITEBYTE(save_p, 0x2f);
	else
		WRITEBYTE(save_p, 0x2e);
}

static boolean P_NetUnArchiveMisc(void)
{
	ULONG pig;
	int i, j;

	gamemap = READSHORT(save_p);
	G_SetGamestate(READSHORT(save_p));

	tokenlist = READULONG(save_p);

	pig = READULONG(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
	{
		playeringame[i] = (pig & (1<<i)) != 0;
		players[i].playerstate = PST_REBORN;
	}

	if (!P_SetupLevel(gamemap, true))
		return false;
	// the "\2" instead of NULL is a hackishly hackish hack to avoid loading precipitation

	for (i = 0; i < MAXPLAYERS; i++)
		for (j = 0; j < S_PLAY_SUPERTRANS9+1; j++)
			playerstatetics[i][j] = READLONG(save_p);

	// get the time
	leveltime = READULONG(save_p);
	totalrings = READULONG(save_p);
	lastmap = READSHORT(save_p);

	emeralds = READUSHORT(save_p);

	token = READULONG(save_p);
	sstimer = READLONG(save_p);
	bluescore = READULONG(save_p);
	redscore = READULONG(save_p);
	blueflagloose = READULONG(save_p);
	redflagloose = READULONG(save_p);

	autobalance = READSHORT(save_p);
	teamscramble = READSHORT(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
		scrambleplayers[i] = READSHORT(save_p);

	for (i = 0; i < MAXPLAYERS; i++)
		scrambleteams[i] = READSHORT(save_p);

	scrambletotal = READSHORT(save_p);
	scramblecount = READSHORT(save_p);

	countdown = READULONG(save_p);
	countdown2 = READULONG(save_p);

	gravity = READFIXED(save_p);

	countdowntimer = READULONG(save_p);
	countdowntimeup = READBYTE(save_p);

	P_SetRandIndex(READBYTE(save_p));

	matchtype = READLONG(save_p);
	tagtype = READLONG(save_p);

	hidetime = READULONG(save_p);

	// Is it paused?
	if (READBYTE(save_p) == 0x2f)
		paused = true;

	return true;
}

void P_SaveGame(void)
{
	P_ArchiveMisc();
	P_ArchivePlayer();

	WRITEBYTE(save_p, 0x1d); // consistency marker
}

void P_SaveNetGame(void)
{
	thinker_t *th;
	mobj_t *mobj;
	int i = 0;

	CV_SaveNetVars(&save_p);
	P_NetArchiveMisc();

	// Assign the mobjnumber for pointer tracking
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
		{
			mobj = (mobj_t *)th;
			mobj->mobjnum = i++;
		}
	}

	P_NetArchivePlayers();
	P_NetArchiveWorld();
#ifdef POLYOBJECTS
	P_ArchivePolyObjects();
#endif
	P_NetArchiveThinkers();
	P_NetArchiveSpecials();

	WRITEBYTE(save_p, 0x1d); // consistency marker
}

boolean P_LoadGame(short mapoverride)
{
	if (gamestate == GS_INTERMISSION)
		Y_EndIntermission();
	G_SetGamestate(GS_NULL); // should be changed in P_UnArchiveMisc

	if (!P_UnArchiveSPGame(mapoverride))
		return false;

	return READBYTE(save_p) == 0x1d;
}

boolean P_LoadNetGame(void)
{
	CV_LoadNetVars(&save_p);
	if (!P_NetUnArchiveMisc())
		return false;
	P_NetUnArchivePlayers();
	P_NetUnArchiveWorld();
#ifdef POLYOBJECTS
	P_UnArchivePolyObjects();
#endif
	P_NetUnArchiveThinkers();
	P_NetUnArchiveSpecials();
	P_RelinkPointers();
	P_FinishMobjs();

	// The precipitation would normally be spawned in P_SetupLevel, which is called by
	// P_NetUnArchiveMisc above. However, that would place it up before P_NetUnArchiveThinkers,
	// so the thinkers would be deleted later. Therefore, P_SetupLevel will *not* spawn
	// precipitation when loading a netgame save. Instead, precip has to be spawned here.
	// This is done in P_NetUnArchiveSpecials now.

	return READBYTE(save_p) == 0x1d;
}
