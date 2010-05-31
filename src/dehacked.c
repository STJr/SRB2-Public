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
/// \brief Load dehacked file and change tables and text

#include "doomdef.h"
#include "g_game.h"
#include "sounds.h"
#include "info.h"
#include "d_think.h"
#include "dstrings.h"
#include "m_argv.h"
#include "z_zone.h"
#include "w_wad.h"
#include "m_menu.h"
#include "m_misc.h"
#include "f_finale.h"
#include "dehacked.h"
#include "st_stuff.h"
#include "i_system.h"

#ifdef HWRENDER
#include "hardware/hw_light.h"
#endif

#ifdef PC_DOS
#include <stdio.h> // for snprintf
//int	snprintf(char *str, size_t n, const char *fmt, ...);
int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
#endif

boolean deh_loaded = false;
boolean modcredits = false; // Whether a mod creator's name will show in the credits.

typedef struct undehacked_s
{
	char *undata;
	struct undehacked_s *next;
} undehacked_t;

static UINT16 unsocwad;
static undehacked_t *unsocdata[MAX_WADFILES];
static boolean disableundo = false;

void DEH_WriteUndoline(const char *value, const char *data, undotype_f flags)
{
	const char *eqstr = " = ";
	const char *space = " ";
	const char *pader = eqstr;
	undehacked_t *newdata;

	if (disableundo || !unsocwad)
		return;

	if ((newdata = malloc(sizeof(*newdata))) == NULL)
		I_Error("Out of memory for unsoc line");

	if (flags & UNDO_SPACE)
		pader = space;

	if (flags & UNDO_ENDTEXT && !data)
		data = space;

	if (value)
	{
		const size_t plen = strlen(pader);
		const char *pound = "#";
		char *undata = NULL;
		const size_t elen = strlen(pound);
		size_t vlen = strlen(value), dlen = 0, len = 1;

		if (*(value+vlen-1) == '\n')
			vlen--; // lnet not copy the ending \n

		if (flags & UNDO_ENDTEXT)
			len += elen; // let malloc more space

		if (flags & UNDO_NEWLINE)
			len++; // more space for the beginning \n

		if (data)
		{
			dlen = strlen(data);
			if (flags & UNDO_CUTLINE && *(data+dlen-1) == '\n')
				dlen--; // let not copy the ending \n
			newdata->undata = malloc(vlen+plen+dlen+len);
			newdata->undata[vlen+plen+dlen+len-1] = '\0';
		}
		else
		{
			newdata->undata = malloc(vlen+len);
			newdata->undata[vlen+len-1] = '\0';
		}

		if (newdata->undata)
		{
			undata = newdata->undata;
			*undata = '\0';
		}
		else
		{
			free(newdata);
			I_Error("Out of memory for unsoc data");
		}

		if (flags & UNDO_NEWLINE) // let start with \n
			strcat(undata, "\n");

		strncat(undata, value, vlen);

		if (data) // value+pader+data
			strncat(strncat(undata, pader, plen), data, dlen);

		if (flags & UNDO_ENDTEXT) // let end the text
			strncat(undata, pound, elen);
	}
	else
		newdata->undata = NULL;

	newdata->next = unsocdata[unsocwad];
	unsocdata[unsocwad] = newdata;
}

char *myfgets(char *buf, size_t bufsize, MYFILE *f)
{
	size_t i = 0;
	if (myfeof(f))
		return NULL;
	// we need one byte for a null terminated string
	bufsize--;
	while (i < bufsize && !myfeof(f))
	{
		char c = *f->curpos++;
		if (c != '\r')
			buf[i++] = c;
		if (c == '\n')
			break;
	}
	buf[i] = '\0';

	return buf;
}

static char *myhashfgets(char *buf, size_t bufsize, MYFILE *f)
{
	size_t i = 0;
	if (myfeof(f))
		return NULL;
	// we need one byte for a null terminated string
	bufsize--;
	while (i < bufsize && !myfeof(f))
	{
		char c = *f->curpos++;
		if (c != '\r')
			buf[i++] = c;
		if (c == '#')
			break;
	}
	i++;
	buf[i] = '\0';

	return buf;
}

static INT32 deh_num_warning = 0;

FUNCPRINTF static void deh_warning(const char *first, ...)
{
	va_list argptr;
	XBOXSTATIC char buf[1000];

	va_start(argptr, first);
	vsnprintf(buf, sizeof buf, first, argptr);
	va_end(argptr);

	CONS_Printf("%s\n", buf);

	deh_num_warning++;
}

/* ======================================================================== */
// Load a dehacked file format
/* ======================================================================== */
/* a sample to see
                   Thing 1 (Player)       {           // MT_PLAYER
INT32 doomednum;     ID # = 3232              -1,             // doomednum
INT32 spawnstate;    Initial frame = 32       S_PLAY,         // spawnstate
INT32 spawnhealth;   Hit points = 3232        100,            // spawnhealth
INT32 seestate;      First moving frame = 32  S_PLAY_RUN1,    // seestate
INT32 seesound;      Alert sound = 32         sfx_None,       // seesound
INT32 reactiontime;  Reaction time = 3232     0,              // reactiontime
INT32 attacksound;   Attack sound = 32        sfx_None,       // attacksound
INT32 painstate;     Injury frame = 32        S_PLAY_PAIN,    // painstate
INT32 painchance;    Pain chance = 3232       255,            // painchance
INT32 painsound;     Pain sound = 32          sfx_plpain,     // painsound
INT32 meleestate;    Close attack frame = 32  S_NULL,         // meleestate
INT32 missilestate;  Far attack frame = 32    S_PLAY_ATK1,    // missilestate
INT32 deathstate;    Death frame = 32         S_PLAY_DIE1,    // deathstate
INT32 xdeathstate;   Exploding frame = 32     S_PLAY_XDIE1,   // xdeathstate
INT32 deathsound;    Death sound = 32         sfx_pldeth,     // deathsound
INT32 speed;         Speed = 3232             0,              // speed
INT32 radius;        Width = 211812352        16*FRACUNIT,    // radius
INT32 height;        Height = 211812352       56*FRACUNIT,    // height
INT32 dispoffset;    DispOffset = 0           0,              // dispoffset
INT32 mass;          Mass = 3232              100,            // mass
INT32 damage;        Missile damage = 3232    0,              // damage
INT32 activesound;   Action sound = 32        sfx_None,       // activesound
INT32 flags;         Bits = 3232              MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,
INT32 raisestate;    Respawn frame = 32       S_NULL          // raisestate
                                         }, */

static INT32 searchvalue(const char *s)
{
	while (s[0] != '=' && s[0])
		s++;
	if (s[0] == '=')
		return atoi(&s[1]);
	else
	{
		deh_warning("No value found");
		return 0;
	}
}

#if 0
static float searchfvalue(const char *s)
{
	while (s[0] != '=' && s[0])
		s++;
	if (s[0] == '=')
		return (float)atof(&s[1]);
	else
	{
		deh_warning("No value found");
		return 0;
	}
}
#endif

/*
// Edits an animated texture slot on the array
// Tails 12-27-2003
static void readAnimTex(MYFILE *f, INT32 num)
{
	char s[MAXLINELEN];
	char *word;
	char *word2;
	INT32 i;

	do {
		if (myfgets(s, sizeof s, f) != NULL)
		{
			if (s[0] == '\n') break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';
			// set the value in the appropriate field

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			word2 = strtok(NULL, " = ");
			if (word2)
				strupr(word2);
			else
				break;

			word2[strlen(word2)-1] = '\0';

			i = atoi(word2);

			if (!strcmp(word, "START"))
				strncpy(harddefs[num].startname, word2, 8);
			if (!strcmp(word, "END"))
				strncpy(harddefs[num].endname, word2, 8);
			else if (!strcmp(word, "SPEED")) harddefs[num].speed = i;
			else if (!strcmp(word, "ISTEXTURE")) harddefs[num].istexture = i;

			else deh_warning("readAnimTex %d: unknown word '%s'", num, word);
		}
	} while (s[0] != '\n' && !myfeof(f)); //finish when the line is empty
}
*/

static boolean findFreeSlot(INT32 *num)
{
	// Send the character select entry to a free slot.
	while (*num < 15 && PlayerMenu[*num].status != IT_DISABLED)
		*num = *num+1;

	// No more free slots. :(
	if (*num >= 15)
		return false;

	// Found one! ^_^
	return true;
}

// Reads a player.
// For modifying the character select screen
static void readPlayer(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word;
	char *word2;
	INT32 i;
	boolean slotfound = false;

	DEH_WriteUndoline("PLAYERTEXT", description[num].info, UNDO_ENDTEXT);

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			if (!strcmp(word, "PLAYERTEXT"))
			{
				char *playertext = NULL;

				if (!slotfound && (slotfound = findFreeSlot(&num)) == false)
					return;

				for (i = 0; i < MAXLINELEN-3; i++)
				{
					if (s[i] == '=')
					{
						playertext = &s[i+2];
						break;
					}
				}
				if (playertext)
				{
					strcpy(description[num].info, playertext);
					strcat(description[num].info, myhashfgets(playertext, sizeof (description[num].info), f));
				}
				else
					strcpy(description[num].info, "");

				// For some reason, cutting the string did not work above. Most likely due to strcpy or strcat...
				// It works down here, though.
				{
					INT32 numlines = 0;
					for (i = 0; i < MAXLINELEN-1; i++)
					{
						if (numlines < 7 && description[num].info[i] == '\n')
							numlines++;

						if (numlines >= 7 || description[num].info[i] == '\0' || description[num].info[i] == '#')
							break;
					}
				}
				description[num].info[strlen(description[num].info)-1] = '\0';
				description[num].info[i] = '\0';
				continue;
			}

			word2 = strtok(NULL, " = ");
			if (word2)
				strupr(word2);
			else
				break;

			word2[strlen(word2)-1] = '\0';
			i = atoi(word2);

			if (!strcmp(word, "PLAYERNAME"))
			{
				if (!slotfound && (slotfound = findFreeSlot(&num)) == false)
					return;
				DEH_WriteUndoline(word, description[num].text, UNDO_NONE);
				strlcpy(description[num].text, word2, sizeof (description[num].text));
				PlayerMenu[num].text = description[num].text;
			}
			else if (!strcmp(word, "MENUPOSITION"))
			{
				if (disableundo) // NO SCREWING UP MY MENU, FOOL!
				{
					slotfound = true;
					num = i;
				}
			}
			else if (!strcmp(word, "PICNAME"))
			{
				if (!slotfound && (slotfound = findFreeSlot(&num)) == false)
					return;
				DEH_WriteUndoline(word, &description[num].picname[0], UNDO_NONE);
				strlcpy(description[num].picname, word2, 9);
			}
			else if (!strcmp(word, "STATUS"))
			{
				// Limit the status to only IT_DISABLED and IT_CALL|IT_STRING
				if (i != IT_STRING)
					i = IT_DISABLED;
				else
					i = IT_STRING;

				/*
					You MAY disable previous entries if you so desire...
					But try to enable something that's already enabled and you will be sent to a free slot.

					Because of this, you are allowed to edit any previous entrys you like, but only if you
					signal that you are purposely doing so by disabling and then reenabling the slot.
				*/
				if (i != IT_DISABLED && !slotfound && (slotfound = findFreeSlot(&num)) == false)
					return;
				DEH_WriteUndoline(word, va("%d", PlayerMenu[num].status), UNDO_NONE);
				PlayerMenu[num].status = (INT16)i;
			}
			else if (!strcmp(word, "SKINNAME"))
			{
				// Send to free slot.
				if (!slotfound && (slotfound = findFreeSlot(&num)) == false)
					return;
				DEH_WriteUndoline(word, description[num].skinname, UNDO_NONE);
				strcpy(description[num].skinname, word2);
			}
			else
				deh_warning("readPlayer %d: unknown word '%s'", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty

	if (slotfound)
		DEH_WriteUndoline("MENUPOSITION", va("%d", num), UNDO_NONE);
}

static void readthing(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word;
	char *tmp;
	INT32 value;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			value = searchvalue(s);

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			if (!strcmp(word, "MAPTHINGNUM"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].doomednum), UNDO_NONE);
				mobjinfo[num].doomednum = value;
			}
			else if (!strcmp(word, "SPAWNSTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].spawnstate), UNDO_NONE);
				mobjinfo[num].spawnstate = value;
			}
			else if (!strcmp(word, "SPAWNHEALTH"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].spawnhealth), UNDO_NONE);
				mobjinfo[num].spawnhealth = value;
			}
			else if (!strcmp(word, "SEESTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].seestate), UNDO_NONE);
				mobjinfo[num].seestate = value;
			}
			else if (!strcmp(word, "SEESOUND"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].seesound), UNDO_NONE);
				mobjinfo[num].seesound = value;
			}
			else if (!strcmp(word, "REACTIONTIME"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].reactiontime), UNDO_NONE);
				mobjinfo[num].reactiontime = value;
			}
			else if (!strcmp(word, "ATTACKSOUND"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].attacksound), UNDO_NONE);
				mobjinfo[num].attacksound = value;
			}
			else if (!strcmp(word, "PAINSTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].painstate), UNDO_NONE);
				mobjinfo[num].painstate = value;
			}
			else if (!strcmp(word, "PAINCHANCE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].painchance), UNDO_NONE);
				mobjinfo[num].painchance = value;
			}
			else if (!strcmp(word, "PAINSOUND"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].painsound), UNDO_NONE);
				mobjinfo[num].painsound = value;
			}
			else if (!strcmp(word, "MELEESTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].meleestate), UNDO_NONE);
				mobjinfo[num].meleestate = value;
			}
			else if (!strcmp(word, "MISSILESTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].missilestate), UNDO_NONE);
				mobjinfo[num].missilestate = value;
			}
			else if (!strcmp(word, "DEATHSTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].deathstate), UNDO_NONE);
				mobjinfo[num].deathstate = value;
			}
			else if (!strcmp(word, "DEATHSOUND"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].deathsound), UNDO_NONE);
				mobjinfo[num].deathsound = value;
			}
			else if (!strcmp(word, "XDEATHSTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].xdeathstate), UNDO_NONE);
				mobjinfo[num].xdeathstate = value;
			}
			else if (!strcmp(word, "SPEED"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].speed), UNDO_NONE);
				mobjinfo[num].speed = value;
			}
			else if (!strcmp(word, "RADIUS"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].radius), UNDO_NONE);
				mobjinfo[num].radius = value;
			}
			else if (!strcmp(word, "HEIGHT"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].height), UNDO_NONE);
				mobjinfo[num].height = value;
			}
			else if (!strcmp(word, "DISPOFFSET"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].dispoffset), UNDO_NONE);
				mobjinfo[num].dispoffset = value;
			}
			else if (!strcmp(word, "MASS"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].mass), UNDO_NONE);
				mobjinfo[num].mass = value;
			}
			else if (!strcmp(word, "DAMAGE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].damage), UNDO_NONE);
				mobjinfo[num].damage = value;
			}
			else if (!strcmp(word, "ACTIVESOUND"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].activesound), UNDO_NONE);
				mobjinfo[num].activesound = value;
			}
			else if (!strcmp(word, "FLAGS"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].flags), UNDO_NONE);
				mobjinfo[num].flags = value;
			}
			else if (!strcmp(word, "RAISESTATE"))
			{
				DEH_WriteUndoline(word, va("%d", mobjinfo[num].raisestate), UNDO_NONE);
				mobjinfo[num].raisestate = value;
			}
			else
				deh_warning("Thing %d: unknown word '%s'", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

static void readlevelheader(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word = s;
	char *word2;
	char *tmp;
	INT32 i;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			// First remove trailing newline, if there is one
			tmp = strchr(s, '\n');
			if (tmp)
				*tmp = '\0';

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			// Get the part before the " = "
			tmp = strchr(s, '=');
			*(tmp-1) = '\0';
			strupr(word);

			// Now get the part after
			word2 = tmp += 2;
			strupr(word2);

			i = atoi(word2); // used for numerical settings

			if (!strcmp(word, "LEVELNAME"))
			{
				DEH_WriteUndoline(word, mapheaderinfo[num-1].lvlttl, UNDO_NONE);
				strlcpy(mapheaderinfo[num-1].lvlttl, word2, 33);
			}
			else if (!strcmp(word, "SUBTITLE"))
			{
				DEH_WriteUndoline(word, mapheaderinfo[num-1].subttl, UNDO_NONE);
				strlcpy(mapheaderinfo[num-1].subttl, word2, 33);
			}
			else if (!strcmp(word, "ACT"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].actnum), UNDO_NONE);
				if (i >= 0 && i < 20) // 0 for no act number, TTL1 through TTL19
					mapheaderinfo[num-1].actnum = (UINT8)i;
				else
					deh_warning("Level header %d: invalid act number %d", num, i);
			}
			else if (!strcmp(word, "TYPEOFLEVEL"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].typeoflevel), UNDO_NONE);
				mapheaderinfo[num-1].typeoflevel = (INT16)i;
			}
			else if (!strcmp(word, "NEXTLEVEL"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					i = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].nextlevel), UNDO_NONE);
				mapheaderinfo[num-1].nextlevel = (INT16)i;
			}
			else if (!strcmp(word, "MUSICSLOT"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].musicslot), UNDO_NONE);
				mapheaderinfo[num-1].musicslot = i;
			}
			else if (!strcmp(word, "FORCECHARACTER"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].forcecharacter), UNDO_NONE);
				mapheaderinfo[num-1].forcecharacter = (UINT8)i;
			}
			else if (!strcmp(word, "WEATHER"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].weather), UNDO_NONE);
				mapheaderinfo[num-1].weather = (UINT8)i;
			}
			else if (!strcmp(word, "SKYNUM"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].skynum), UNDO_NONE);
				mapheaderinfo[num-1].skynum = (INT16)i;
			}
			else if (!strcmp(word, "INTERSCREEN"))
			{
				DEH_WriteUndoline(word, mapheaderinfo[num-1].interscreen, UNDO_NONE);
				strncpy(mapheaderinfo[num-1].interscreen, word2, 8);
				// TODO: This needs fixed. DEH_WriteUndoline expects a terminated string.
			}
			else if (!strcmp(word, "SCRIPTNAME"))
			{
				DEH_WriteUndoline(word, mapheaderinfo[num-1].scriptname, UNDO_NONE);
				strlcpy(mapheaderinfo[num-1].scriptname, word2, sizeof (mapheaderinfo[num-1].scriptname));
			}
			else if (!strcmp(word, "SCRIPTISLUMP"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].scriptislump), UNDO_NONE);
				mapheaderinfo[num-1].scriptislump = i;
			}
			else if (!strcmp(word, "PRECUTSCENENUM"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].precutscenenum), UNDO_NONE);
				mapheaderinfo[num-1].precutscenenum = (UINT8)i;
			}
			else if (!strcmp(word, "CUTSCENENUM"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].cutscenenum), UNDO_NONE);
				mapheaderinfo[num-1].cutscenenum = (UINT8)i;
			}
			else if (!strcmp(word, "COUNTDOWN"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].countdown), UNDO_NONE);
				mapheaderinfo[num-1].countdown = (INT16)i;
			}
			else if (!strcmp(word, "NOZONE"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].nozone), UNDO_NONE);
				mapheaderinfo[num-1].nozone = i;
			}
			else if (!strcmp(word, "HIDDEN"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].hideinmenu), UNDO_NONE);
				mapheaderinfo[num-1].hideinmenu = i;
			}
			else if (!strcmp(word, "NOSSMUSIC"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].nossmusic), UNDO_NONE);
				mapheaderinfo[num-1].nossmusic = i;
			}
			else if (!strcmp(word, "SPEEDMUSIC"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].speedmusic), UNDO_NONE);
				mapheaderinfo[num-1].speedmusic = i;
			}
			else if (!strcmp(word, "NORELOAD"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].noreload), UNDO_NONE);
				mapheaderinfo[num-1].noreload = i;
			}
			else if (!strcmp(word, "TIMEATTACK"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].timeattack), UNDO_NONE);
				mapheaderinfo[num-1].timeattack = i;
			}
			else if (!strcmp(word, "LEVELSELECT"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].levelselect), UNDO_NONE);
				mapheaderinfo[num-1].levelselect = i;
			}
			else if (!strcmp(word, "NOPERFECTBONUS"))
			{
				DEH_WriteUndoline(word, va("%d", mapheaderinfo[num-1].noperfectbns), UNDO_NONE);
				mapheaderinfo[num-1].noperfectbns = i;
			}
			else if (!strcmp(word, "RUNSOC"))
			{
				DEH_WriteUndoline(word, mapheaderinfo[num-1].runsoc, UNDO_NONE);
				strlcpy(mapheaderinfo[num-1].runsoc, word2, sizeof (mapheaderinfo[num-1].runsoc));
			}
			else if (!strcmp(word, "PALETTE"))
			{
				DEH_WriteUndoline(word, va("%u", mapheaderinfo[num-1].palette), UNDO_NONE);
				mapheaderinfo[num-1].palette = (UINT16)i;
			}
			else
				deh_warning("Level header %d: unknown word '%s'", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

static void readcutscenescene(MYFILE *f, INT32 num, INT32 scenenum)
{
	XBOXSTATIC char s[MAXLINELEN] = "";
	char *word;
	char *word2;
	INT32 i;
	UINT16 usi;

	DEH_WriteUndoline("SCENETEXT", cutscenes[num].scene[scenenum].text, UNDO_ENDTEXT);

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			if (!strcmp(word, "SCENETEXT"))
			{
				char *scenetext = NULL;
				XBOXSTATIC char buffer[4096] = "";
				for (i = 0; i < MAXLINELEN; i++)
				{
					if (s[i] == '=')
					{
						scenetext = &s[i+2];
						break;
					}
				}

				if (!scenetext)
				{
					Z_Free(cutscenes[num].scene[scenenum].text);
					cutscenes[num].scene[scenenum].text = NULL;
					continue;
				}

				for (i = 0; i < MAXLINELEN; i++)
				{
					if (s[i] == '\0')
					{
						s[i] = '\n';
						s[i+1] = '\0';
						break;
					}
				}

				strcpy(buffer, scenetext);

				strcat(buffer,
					myhashfgets(scenetext, sizeof (buffer)
					- strlen(buffer) - 1, f));

				// A cutscene overwriting another one...
				Z_Free(cutscenes[num].scene[scenenum].text);

				cutscenes[num].scene[scenenum].text = Z_StrDup(buffer);

				continue;
			}

			word2 = strtok(NULL, " = ");
			if (word2)
				strupr(word2);
			else
				break;

			word2[strlen(word2)-1] = '\0';
			i = atoi(word2);
			usi = (UINT16)i;


			if (!strcmp(word, "NUMBEROFPICS"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].numpics), UNDO_NONE);
				cutscenes[num].scene[scenenum].numpics = (UINT8)i;
			}
			else if (!strcmp(word, "PIC1NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[0], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[0], word2, 8);
			}
			else if (!strcmp(word, "PIC2NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[1], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[1], word2, 8);
			}
			else if (!strcmp(word, "PIC3NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[2], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[2], word2, 8);
			}
			else if (!strcmp(word, "PIC4NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[3], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[3], word2, 8);
			}
			else if (!strcmp(word, "PIC5NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[4], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[4], word2, 8);
			}
			else if (!strcmp(word, "PIC6NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[5], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[5], word2, 8);
			}
			else if (!strcmp(word, "PIC7NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[6], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[6], word2, 8);
			}
			else if (!strcmp(word, "PIC8NAME"))
			{
				DEH_WriteUndoline(word, cutscenes[num].scene[scenenum].picname[7], UNDO_NONE);
				strncpy(cutscenes[num].scene[scenenum].picname[7], word2, 8);
			}
			else if (!strcmp(word, "PIC1HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[0]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[0] = i;
			}
			else if (!strcmp(word, "PIC2HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[1]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[1] = i;
			}
			else if (!strcmp(word, "PIC3HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[2]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[2] = i;
			}
			else if (!strcmp(word, "PIC4HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[3]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[3] = i;
			}
			else if (!strcmp(word, "PIC5HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[4]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[4] = i;
			}
			else if (!strcmp(word, "PIC6HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[5]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[5] = i;
			}
			else if (!strcmp(word, "PIC7HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[6]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[6] = i;
			}
			else if (!strcmp(word, "PIC8HIRES"))
			{
				DEH_WriteUndoline(word, va("%d", cutscenes[num].scene[scenenum].pichires[7]), UNDO_NONE);
				cutscenes[num].scene[scenenum].pichires[7] = i;
			}
			else if (!strcmp(word, "PIC1DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[0]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[0] = usi;
			}
			else if (!strcmp(word, "PIC2DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[1]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[1] = usi;
			}
			else if (!strcmp(word, "PIC3DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[2]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[2] = usi;
			}
			else if (!strcmp(word, "PIC4DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[3]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[3] = usi;
			}
			else if (!strcmp(word, "PIC5DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[4]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[4] = usi;
			}
			else if (!strcmp(word, "PIC6DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[5]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[5] = usi;
			}
			else if (!strcmp(word, "PIC7DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[6]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[6] = usi;
			}
			else if (!strcmp(word, "PIC8DURATION"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].picduration[7]), UNDO_NONE);
				cutscenes[num].scene[scenenum].picduration[7] = usi;
			}
			else if (!strcmp(word, "PIC1XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[0]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[0] = usi;
			}
			else if (!strcmp(word, "PIC2XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[1]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[1] = usi;
			}
			else if (!strcmp(word, "PIC3XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[2]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[2] = usi;
			}
			else if (!strcmp(word, "PIC4XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[3]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[3] = usi;
			}
			else if (!strcmp(word, "PIC5XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[4]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[4] = usi;
			}
			else if (!strcmp(word, "PIC6XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[5]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[5] = usi;
			}
			else if (!strcmp(word, "PIC7XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[6]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[6] = usi;
			}
			else if (!strcmp(word, "PIC8XCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].xcoord[7]), UNDO_NONE);
				cutscenes[num].scene[scenenum].xcoord[7] = usi;
			}
			else if (!strcmp(word, "PIC1YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[0]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[0] = usi;
			}
			else if (!strcmp(word, "PIC2YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[1]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[1] = usi;
			}
			else if (!strcmp(word, "PIC3YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[2]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[2] = usi;
			}
			else if (!strcmp(word, "PIC4YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[3]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[3] = usi;
			}
			else if (!strcmp(word, "PIC5YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[4]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[4] = usi;
			}
			else if (!strcmp(word, "PIC6YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[5]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[5] = usi;
			}
			else if (!strcmp(word, "PIC7YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[6]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[6] = usi;
			}
			else if (!strcmp(word, "PIC8YCOORD"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].ycoord[7]), UNDO_NONE);
				cutscenes[num].scene[scenenum].ycoord[7] = usi;
			}
			else if (!strcmp(word, "MUSICSLOT"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].musicslot), UNDO_NONE);
				cutscenes[num].scene[scenenum].musicslot = (musicenum_t)i;
			}
			else if (!strcmp(word, "MUSICLOOP"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].musicloop), UNDO_NONE);
				cutscenes[num].scene[scenenum].musicloop = i;
			}
			else if (!strcmp(word, "TEXTXPOS"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].textxpos), UNDO_NONE);
				cutscenes[num].scene[scenenum].textxpos = usi;
			}
			else if (!strcmp(word, "TEXTYPOS"))
			{
				DEH_WriteUndoline(word, va("%u", cutscenes[num].scene[scenenum].textypos), UNDO_NONE);
				cutscenes[num].scene[scenenum].textypos = usi;
			}
			else
				deh_warning("CutSceneScene %d: unknown word '%s'", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

static void readcutscene(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word;
	char *word2;
	char *tmp;
	INT32 value;
	const INT32 oldnumscenes = cutscenes[num].numscenes;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			word2 = strtok(NULL, " ");
			if (word2)
				value = atoi(word2);
			else
			{
				deh_warning("No value for token %s", word);
				continue;
			}

			if (!strcmp(word, "NUMSCENES"))
			{
				cutscenes[num].numscenes = value;
			}
			else if (!strcmp(word, "SCENE"))
			{
				if (1 <= value && value <= 128)
				{
					readcutscenescene(f, num, value - 1);
					DEH_WriteUndoline(word, word2, UNDO_SPACE|UNDO_CUTLINE);
					DEH_WriteUndoline("NUMSCENES", va("%d", oldnumscenes), UNDO_SPACE);
				}
				else
					deh_warning("Scene number %d out of range", value);

			}
			else
				deh_warning("Cutscene %d: unknown word '%s', Scene <num> expected.", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

static void readunlockable(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word = s;
	char *word2;
	char *tmp;
	INT32 i;

	DEH_WriteUndoline("NEEDEDEMBLEMS", va("%d", customsecretinfo[num].neededemblems), UNDO_NONE);
	DEH_WriteUndoline("NEEDEDTIME", va("%d", customsecretinfo[num].neededtime), UNDO_NONE);
	DEH_WriteUndoline("NEEDEDGRADE", va("%d", customsecretinfo[num].neededgrade), UNDO_NONE);

	customsecretinfo[num].neededemblems =
	customsecretinfo[num].neededtime =
	customsecretinfo[num].neededgrade = 0;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			// First remove trailing newline, if there is one
			tmp = strchr(s, '\n');
			if (tmp)
				*tmp = '\0';

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			// Get the part before the " = "
			tmp = strchr(s, '=');
			*(tmp-1) = '\0';
			strupr(word);

			// Now get the part after
			word2 = tmp += 2;
			strupr(word2);

			i = atoi(word2); // used for numerical settings

			if (!strcmp(word, "NAME"))
			{
				DEH_WriteUndoline(word, customsecretinfo[num].name, UNDO_NONE);
				strlcpy(customsecretinfo[num].name, word2, sizeof (customsecretinfo[num].name));
			}
			else if (!strcmp(word, "OBJECTIVE"))
			{
				DEH_WriteUndoline(word, customsecretinfo[num].objective, UNDO_NONE);
				strlcpy(customsecretinfo[num].objective, word2, sizeof (customsecretinfo[num].objective));
			}
			else if (!strcmp(word, "NEEDEDGRADE"))
			{
				//DEH_WriteUndoline(word, va("%d", customsecretinfo[num].neededgrade), UNDO_NONE);
				customsecretinfo[num].neededgrade = i;
			}
			else if (!strcmp(word, "NEEDEDEMBLEMS"))
			{
				//DEH_WriteUndoline(word, va("%d", customsecretinfo[num].neededemblems), UNDO_NONE);
				customsecretinfo[num].neededemblems = i;
			}
			else if (!strcmp(word, "NEEDEDTIME"))
			{
				//DEH_WriteUndoline(word, va("%d", customsecretinfo[num].neededtime), UNDO_NONE);
				customsecretinfo[num].neededtime = i;
			}
			else if (!strcmp(word, "TYPE"))
			{
				DEH_WriteUndoline(word, va("%d", customsecretinfo[num].type), UNDO_NONE);
				customsecretinfo[num].type = i;
			}
			else if (!strcmp(word, "VAR"))
			{
				if (customsecretinfo[num].type == 1)
				{
					// Support using the actual map name,
					// i.e., Level AB, Level FZ, etc.

					// Convert to map number
					if (word2[0] >= 'A' && word2[0] <= 'Z')
						i = M_MapNumber(word2[0], word2[1]);
				}

				DEH_WriteUndoline(word, va("%d", customsecretinfo[num].variable), UNDO_NONE);
				customsecretinfo[num].variable = i;
			}
			else
				deh_warning("Unlockable %d: unknown word '%s'", num+1, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

static void readhuditem(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word = s;
	char *word2;
	char *tmp;
	INT32 i;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			// First remove trailing newline, if there is one
			tmp = strchr(s, '\n');
			if (tmp)
				*tmp = '\0';

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			// Get the part before the " = "
			tmp = strchr(s, '=');
			*(tmp-1) = '\0';
			strupr(word);

			// Now get the part after
			word2 = tmp += 2;
			strupr(word2);

			i = atoi(word2); // used for numerical settings

			if (!strcmp(word, "X"))
			{
				DEH_WriteUndoline(word, va("%d", hudinfo[num].x), UNDO_NONE);
				hudinfo[num].x = i;
			}
			else if (!strcmp(word, "Y"))
			{
				DEH_WriteUndoline(word, va("%d", hudinfo[num].y), UNDO_NONE);
				hudinfo[num].y = i;
			}
			else
				deh_warning("Level header %d: unknown word '%s'", num, word);
		}
	} while (!myfeof(f)); // finish when the line is empty
}

/*
Sprite number = 10
Sprite subnumber = 32968
Duration = 200
Next frame = 200
*/

/** Action pointer for reading actions from Dehacked lumps.
  */
typedef struct
{
	actionf_t action; ///< Function pointer corresponding to the actual action.
	const char *name; ///< Name of the action in ALL CAPS.
} actionpointer_t;

/** Array mapping action names to action functions.
  * Names must be in ALL CAPS for case insensitive comparisons.
  */
static actionpointer_t actionpointers[] =
{
	{{A_Explode},              "A_EXPLODE"},
	{{A_Pain},                 "A_PAIN"},
	{{A_Fall},                 "A_FALL"},
	{{A_MonitorPop},           "A_MONITORPOP"},
	{{A_Look},                 "A_LOOK"},
	{{A_Chase},                "A_CHASE"},
	{{A_FaceTarget},           "A_FACETARGET"},
	{{A_Scream},               "A_SCREAM"},
	{{A_BossDeath},            "A_BOSSDEATH"},
	{{A_CustomPower},          "A_CUSTOMPOWER"},
	{{A_GiveWeapon},           "A_GIVEWEAPON"},
	{{A_RingShield},           "A_RINGSHIELD"},
	{{A_RingBox},              "A_RINGBOX"},
	{{A_Invincibility},        "A_INVINCIBILITY"},
	{{A_SuperSneakers},        "A_SUPERSNEAKERS"},
	{{A_BunnyHop},             "A_BUNNYHOP"},
	{{A_BubbleSpawn},          "A_BUBBLESPAWN"},
	{{A_BubbleRise},           "A_BUBBLERISE"},
	{{A_BubbleCheck},          "A_BUBBLECHECK"},
	{{A_ExtraLife},            "A_EXTRALIFE"},
	{{A_BombShield},           "A_BOMBSHIELD"},
	{{A_JumpShield},           "A_JUMPSHIELD"},
	{{A_WaterShield},          "A_WATERSHIELD"},
	{{A_ForceShield},          "A_FORCESHIELD"},
	{{A_GravityBox},           "A_GRAVITYBOX"},
	{{A_ScoreRise},            "A_SCORERISE"},
	{{A_ParticleSpawn},        "A_PARTICLESPAWN"},
	{{A_AttractChase},         "A_ATTRACTCHASE"},
	{{A_DropMine},             "A_DROPMINE"},
	{{A_FishJump},             "A_FISHJUMP"},
	{{A_ThrownRing},           "A_THROWNRING"},
	{{A_GrenadeRing},          "A_GRENADERING"},
	{{A_SetSolidSteam},        "A_SETSOLIDSTEAM"},
	{{A_UnsetSolidSteam},      "A_UNSETSOLIDSTEAM"},
	{{A_SignPlayer},           "A_SIGNPLAYER"},
	{{A_JetChase},             "A_JETCHASE"},
	{{A_JetbThink},            "A_JETBTHINK"},
	{{A_JetgThink},            "A_JETGTHINK"},
	{{A_JetgShoot},            "A_JETGSHOOT"},
	{{A_ShootBullet},          "A_SHOOTBULLET"},
	{{A_MinusDigging},         "A_MINUSDIGGING"},
	{{A_MinusPopup},           "A_MINUSPOPUP"},
	{{A_MinusCheck},           "A_MINUSCHECK"},
	{{A_ChickenCheck},         "A_CHICKENCHECK"},
	{{A_MouseThink},           "A_MOUSETHINK"},
	{{A_DetonChase},           "A_DETONCHASE"},
	{{A_CapeChase},            "A_CAPECHASE"},
	{{A_RotateSpikeBall},      "A_ROTATESPIKEBALL"},
	{{A_MaceRotate},           "A_MACEROTATE"},
	{{A_RockSpawn},            "A_ROCKSPAWN"},
	{{A_SnowBall},             "A_SNOWBALL"},
	{{A_CrawlaCommanderThink}, "A_CRAWLACOMMANDERTHINK"},
	{{A_SmokeTrailer},         "A_SMOKETRAILER"},
	{{A_RingExplode},          "A_RINGEXPLODE"},
	{{A_OldRingExplode},       "A_OLDRINGEXPLODE"},
	{{A_MixUp},                "A_MIXUP"},
	{{A_RecyclePowers},        "A_RECYCLEPOWERS"},
	{{A_BossScream},           "A_BOSSSCREAM"},
	{{A_Invinciblerize},       "A_INVINCIBLERIZE"},
	{{A_DeInvinciblerize},     "A_DEINVINCIBLERIZE"},
	{{A_GoopSplat},            "A_GOOPSPLAT"},
	{{A_Boss2PogoSFX},         "A_BOSS2POGOSFX"},
	{{A_BossJetFume},          "A_BOSSJETFUME"},
	{{A_EggmanBox},            "A_EGGMANBOX"},
	{{A_TurretFire},           "A_TURRETFIRE"},
	{{A_SuperTurretFire},      "A_SUPERTURRETFIRE"},
	{{A_TurretStop},           "A_TURRETSTOP"},
	{{A_JetJawRoam},           "A_JETJAWROAM"},
	{{A_JetJawChomp},          "A_JETJAWCHOMP"},
	{{A_PointyThink},          "A_POINTYTHINK"},
	{{A_CheckBuddy},           "A_CHECKBUDDY"},
	{{A_HoodThink},            "A_HOODTHINK"},
	{{A_ArrowCheck},           "A_ARROWCHECK"},
	{{A_SnailerThink},         "A_SNAILERTHINK"},
	{{A_SharpChase},           "A_SHARPCHASE"},
	{{A_SharpSpin},            "A_SHARPSPIN"},
	{{A_VultureVtol},          "A_VULTUREVTOL"},
	{{A_VultureCheck},         "A_VULTURECHECK"},
	{{A_SkimChase},            "A_SKIMCHASE"},
	{{A_1upThinker},           "A_1UPTHINKER"},
	{{A_SkullAttack},          "A_SKULLATTACK"},
	{{A_LobShot},              "A_LOBSHOT"},
	{{A_CannonLook},           "A_CANNONLOOK"},
	{{A_FireShot},             "A_FIRESHOT"},
	{{A_SuperFireShot},        "A_SUPERFIRESHOT"},
	{{A_BossFireShot},         "A_BOSSFIRESHOT"},
	{{A_SparkFollow},          "A_SPARKFOLLOW"},
	{{A_BuzzFly},              "A_BUZZFLY"},
	{{A_GuardChase},           "A_GUARDCHASE"},
	{{A_SetReactionTime},      "A_SETREACTIONTIME"},
	{{A_Boss3TakeDamage},      "A_BOSS3TAKEDAMAGE"},
	{{A_LinedefExecute},       "A_LINEDEFEXECUTE"},
	{{A_PlaySeeSound},         "A_PLAYSEESOUND"},
	{{A_PlayAttackSound},      "A_PLAYATTACKSOUND"},
	{{A_PlayActiveSound},      "A_PLAYACTIVESOUND"},
	{{A_SpawnObjectAbsolute},  "A_SPAWNOBJECTABSOLUTE"},
	{{A_SpawnObjectRelative},  "A_SPAWNOBJECTRELATIVE"},
	{{A_ChangeAngleRelative},  "A_CHANGEANGLERELATIVE"},
	{{A_ChangeAngleAbsolute},  "A_CHANGEANGLEABSOLUTE"},
	{{A_PlaySound},            "A_PLAYSOUND"},
	{{A_FindTarget},           "A_FINDTARGET"},
	{{A_FindTracer},           "A_FINDTRACER"},
	{{A_SetTics},              "A_SETTICS"},
	{{A_SetRandomTics},        "A_SETRANDOMTICS"},
	{{A_ChangeColorRelative},  "A_CHANGECOLORRELATIVE"},
	{{A_ChangeColorAbsolute},  "A_CHANGECOLORABSOLUTE"},
	{{A_MoveRelative},         "A_MOVERELATIVE"},
	{{A_MoveAbsolute},         "A_MOVEABSOLUTE"},
	{{A_Thrust},               "A_THRUST"},
	{{A_ZThrust},              "A_ZTHRUST"},
	{{A_SetTargetsTarget},     "A_SETTARGETSTARGET"},
	{{A_SetObjectFlags},       "A_SETOBJECTFLAGS"},
	{{A_SetObjectFlags2},      "A_SETOBJECTFLAGS2"},
	{{A_RandomState},          "A_RANDOMSTATE"},
	{{A_RandomStateRange},     "A_RANDOMSTATERANGE"},
	{{A_DualAction},           "A_DUALACTION"},
	{{A_RemoteAction},         "A_REMOTEACTION"},
	{{A_ToggleFlameJet},       "A_TOGGLEFLAMEJET"},

	{{NULL},                   "NONE"},

	// This NULL entry must be the last in the list
	{{NULL},                   NULL},
};

static void readframe(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word1;
	char *word2 = NULL;
	char *tmp;
	INT32 i, j;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			for (j = 0; s[j] != '\n'; j++)
			{
				if (s[j] == '=')
				{
					j += 2;
					j = atoi(&s[j]);
					break;
				}
			}

			word1 = strtok(s, " ");
			if (word1)
				strupr(word1);
			else
				break;

			word2 = strtok(NULL, " = ");
			if (word2)
				strupr(word2);
			else
				break;

			word2[strlen(word2)-1] = '\0';
			i = atoi(word2);

			if (!strcmp(word1, "SPRITENUMBER"))
			{
				DEH_WriteUndoline(word1, va("%u", states[num].sprite), UNDO_NONE);
				states[num].sprite = i;
			}
			else if (!strcmp(word1, "SPRITESUBNUMBER"))
			{
				DEH_WriteUndoline(word1, va("%d", states[num].frame), UNDO_NONE);
				states[num].frame = i;
			}
			else if (!strcmp(word1, "DURATION"))
			{
				DEH_WriteUndoline(word1, va("%u", states[num].tics), UNDO_NONE);
				states[num].tics = i;
			}
			else if (!strcmp(word1, "NEXT"))
			{
				DEH_WriteUndoline(word1, va("%d", states[num].nextstate), UNDO_NONE);
				states[num].nextstate = i;
			}
			else if (!strcmp(word1, "VAR1"))
			{
				DEH_WriteUndoline(word1, va("%d", states[num].var1), UNDO_NONE);
				states[num].var1 = i;
			}
			else if (!strcmp(word1, "VAR2"))
			{
				DEH_WriteUndoline(word1, va("%d", states[num].var2), UNDO_NONE);
				states[num].var2 = i;
			}
			else if (!strcmp(word1, "ACTION"))
			{
				size_t z;
				boolean found = false;
				XBOXSTATIC char actiontocompare[32];

				strlcpy(actiontocompare, word2, sizeof (actiontocompare));
				strupr(actiontocompare);

				for (z = 0; z < 32; z++)
				{
					if (actiontocompare[z] == '\n' || actiontocompare[z] == '\r')
					{
						actiontocompare[z] = '\0';
						break;
					}
				}

				for (z = 0; actionpointers[z].name; z++)
				{
					if (actionpointers[z].action.acv == states[num].action.acv)
					{
						DEH_WriteUndoline(word1, actionpointers[z].name, UNDO_NONE);
						break;
					}
				}

				z = 0;
				while (actionpointers[z].name)
				{
					if (!strcmp(actiontocompare, actionpointers[z].name))
					{
						states[num].action = actionpointers[z].action;
						states[num].action.acv = actionpointers[z].action.acv; // assign
						states[num].action.acp1 = actionpointers[z].action.acp1;
						found = true;
						break;
					}
					z++;
				}

				if (!found)
					deh_warning("Unknown action %s", actiontocompare);
			}
			else
				deh_warning("Frame %d: unknown word '%s'", num, word1);
		}
	} while (!myfeof(f));
}

static void readsound(MYFILE *f, INT32 num, const char *savesfxnames[])
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word;
	char *tmp;
	INT32 value;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			value = searchvalue(s);

			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

/*			if (!strcmp(word, "OFFSET"))
			{
				value -= 150360;
				if (value <= 64)
					value /= 8;
				else if (value <= 260)
					value = (value+4)/8;
				else
					value = (value+8)/8;
				if (value >= -1 && value < sfx_freeslot0 - 1)
					S_sfx[num].name = savesfxnames[value+1];
				else
					deh_warning("Sound %d: offset out of bounds", num);
			}
			else */if (!strcmp(word, "SINGULAR"))
			{
				DEH_WriteUndoline(word, va("%d", S_sfx[num].singularity), UNDO_NONE);
				S_sfx[num].singularity = value;
			}
			else if (!strcmp(word, "PRIORITY"))
			{
				DEH_WriteUndoline(word, va("%d", S_sfx[num].priority), UNDO_NONE);
				S_sfx[num].priority = value;
			}
			else if (!strcmp(word, "FLAGS"))
			{
				DEH_WriteUndoline(word, va("%d", S_sfx[num].pitch), UNDO_NONE);
				S_sfx[num].pitch = value;
			}
			else
				deh_warning("Sound %d : unknown word '%s'",num,word);
		}
	} while (!myfeof(f));
	(void)savesfxnames;
}

/** Checks if a game data file name for a mod is good.
 * "Good" means that it contains only alphanumerics, _, and -;
 * ends in ".dat"; has at least one character before the ".dat";
 * and is not "gamedata.dat" (tested case-insensitively).
 *
 * Assumption: that gamedata.dat is the only .dat file that will
 * ever be treated specially by the game.
 *
 * Note: Check for the tail ".dat" case-insensitively since at
 * present, we get passed the filename in all uppercase.
 *
 * \param s Filename string to check.
 * \return True if the filename is good.
 * \sa readmaincfg()
 * \author Graue <graue@oceanbase.org>
 */
static boolean GoodDataFileName(const char *s)
{
	const char *p;
	const char *tail = ".dat";

	for (p = s; *p != '\0'; p++)
		if (!isalnum(*p) && *p != '_' && *p != '-' && *p != '.')
			return false;

	p = s + strlen(s) - strlen(tail);
	if (p <= s) return false; // too short
	if (stricmp(p, tail) != 0) return false; // doesn't end in .dat
	if (stricmp(s, "gamedata.dat") == 0 && !disableundo) return false;

	return true;
}

static void readmaincfg(MYFILE *f)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word = s;
	char *word2;
	char *tmp;
	INT32 value;
	boolean reload = false;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			// First remove trailing newline, if there is one
			tmp = strchr(s, '\n');
			if (tmp)
				*tmp = '\0';

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			// Get the part before the " = "
			tmp = strchr(s, '=');
			*(tmp-1) = '\0';
			strupr(word);

			// Now get the part after
			word2 = tmp += 2;
			strupr(word2);

			value = atoi(word2); // used for numerical settings

			if (!strcmp(word, "SSTAGE_START"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", sstage_start), UNDO_NONE);
				sstage_start = (INT16)value;
				sstage_end = (INT16)(sstage_start+6);
				useNightsSS = false;
			}
			else if (!strcmp(word, "NSSTAGE_START"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", nsstage_start), UNDO_NONE);
				nsstage_start = (INT16)value;
				nsstage_end = (INT16)(nsstage_start+6);
				useNightsSS = true;
			}
			else if (!strcmp(word, "EXECCFG"))
				COM_BufAddText(va("exec %s\n", word2));
			else if (!strcmp(word, "SPSTAGE_START"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", spstage_start), UNDO_NONE);
				spstage_start = (INT16)value;
			}
			else if (!strcmp(word, "SPSTAGE_END"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", spstage_end), UNDO_NONE);
				spstage_end = (INT16)value;
			}
			else if (!strcmp(word, "RACESTAGE_START"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", racestage_start), UNDO_NONE);
				racestage_start = (INT16)value;
			}
			else if (!strcmp(word, "INVULNTICS"))
			{
				DEH_WriteUndoline(word, va("%u", invulntics), UNDO_NONE);
				invulntics = value;
			}
			else if (!strcmp(word, "SNEAKERTICS"))
			{
				DEH_WriteUndoline(word, va("%u", sneakertics), UNDO_NONE);
				sneakertics = value;
			}
			else if (!strcmp(word, "FLASHINGTICS"))
			{
				DEH_WriteUndoline(word, va("%u", flashingtics), UNDO_NONE);
				flashingtics = value;
			}
			else if (!strcmp(word, "TAILSFLYTICS"))
			{
				DEH_WriteUndoline(word, va("%u", tailsflytics), UNDO_NONE);
				tailsflytics = value;
			}
			else if (!strcmp(word, "UNDERWATERTICS"))
			{
				DEH_WriteUndoline(word, va("%u", underwatertics), UNDO_NONE);
				underwatertics = value;
			}
			else if (!strcmp(word, "SPACETIMETICS"))
			{
				DEH_WriteUndoline(word, va("%u", spacetimetics), UNDO_NONE);
				spacetimetics = value;
			}
			else if (!strcmp(word, "EXTRALIFETICS"))
			{
				DEH_WriteUndoline(word, va("%u", extralifetics), UNDO_NONE);
				extralifetics = value;
			}
			else if (!strcmp(word, "GRAVBOOTSTICS"))
			{
				DEH_WriteUndoline(word, va("%u", gravbootstics), UNDO_NONE);
				gravbootstics = value;
			}
			else if (!strcmp(word, "PARALOOPTICS"))
			{
				DEH_WriteUndoline(word, va("%u", paralooptics), UNDO_NONE);
				paralooptics = value;
			}
			else if (!strcmp(word, "HELPERTICS"))
			{
				DEH_WriteUndoline(word, va("%u", helpertics), UNDO_NONE);
				helpertics = value;
			}
			else if (!strcmp(word, "GAMEOVERTICS"))
			{
				DEH_WriteUndoline(word, va("%u", gameovertics), UNDO_NONE);
				gameovertics = value;
			}
			else if (!strcmp(word, "INTROTOPLAY"))
			{
				DEH_WriteUndoline(word, va("%d", introtoplay), UNDO_NONE);
				introtoplay = (UINT8)value;
			}
			else if (!strcmp(word, "LOOPTITLE"))
			{
				DEH_WriteUndoline(word, va("%d", looptitle), UNDO_NONE);
				looptitle = value;
			}
			else if (!strcmp(word, "TITLESCROLLSPEED"))
			{
				DEH_WriteUndoline(word, va("%d", titlescrollspeed), UNDO_NONE);
				titlescrollspeed = value;
			}
			else if (!strcmp(word, "CREDITSCUTSCENE"))
			{
				DEH_WriteUndoline(word, va("%d", creditscutscene), UNDO_NONE);
				creditscutscene = (UINT8)value;
			}
			else if (!strcmp(word, "DISABLESPEEDADJUST"))
			{
				DEH_WriteUndoline(word, va("%d", disableSpeedAdjust), UNDO_NONE);
				disableSpeedAdjust = (UINT8)value;
			}
			else if (!strcmp(word, "GAMEDATA"))
			{
				size_t filenamelen;

				// Check the data filename so that mods
				// can't write arbitrary files.
				if (!GoodDataFileName(word2))
					I_Error("Maincfg: bad data file name '%s'\n", word2);

				G_SaveGameData();
				DEH_WriteUndoline(word, gamedatafilename, UNDO_NONE);
				strlcpy(gamedatafilename, word2, sizeof (gamedatafilename));
				strlwr(gamedatafilename);
				savemoddata = true;

				// Also save a time attack folder
				filenamelen = strlen(gamedatafilename)-4;  // Strip off the extension
				strncpy(timeattackfolder, gamedatafilename, min(filenamelen, sizeof (timeattackfolder)));
				timeattackfolder[min(filenamelen, sizeof (timeattackfolder) - 1)] = '\0';

				strncpy(savegamename, timeattackfolder, sizeof (timeattackfolder));
				strlcat(savegamename, "%u.ssg", sizeof(savegamename));

				reload = true;
			}
			else if (!strcmp(word, "NUMEMBLEMS"))
			{
				DEH_WriteUndoline(word, va("%d", numemblems-2), UNDO_NONE);
				numemblems = value+2;
				if (numemblems > MAXEMBLEMS-2)
					I_Error("Sorry, a maximum of %d emblems is allowed.\n", MAXEMBLEMS-2);
			}
			else if (!strcmp(word, "RESETDATA"))
			{
				DEH_WriteUndoline(word, "0", UNDO_TODO); /// \todo
				P_ResetData(value);
			}
			else if (!strcmp(word, "CUSTOMVERSION"))
			{
				DEH_WriteUndoline(word, customversionstring, UNDO_NONE);
				strlcpy(customversionstring, word2, sizeof (customversionstring));
			}
			else
				deh_warning("Maincfg: unknown word '%s'", word);
		}
	} while (!myfeof(f));

	if (reload)
		G_LoadGameData();
}

static void reademblemdata(MYFILE *f, INT32 num)
{
	XBOXSTATIC char s[MAXLINELEN];
	char *word;
	char *word2;
	char *tmp;
	INT32 value;

	do
	{
		if (myfgets(s, sizeof (s), f))
		{
			if (s[0] == '\n')
				break;

			tmp = strchr(s, '#');
			if (tmp)
				*tmp = '\0';

			value = searchvalue(s);
			word = strtok(s, " ");
			if (word)
				strupr(word);
			else
				break;

			word2 = strtok(NULL, " ");
			if (word2)
				strupr(word2);
			else
				break;

			if (!strcmp(word, "X"))
			{
				DEH_WriteUndoline(word, va("%d", emblemlocations[num-1].x), UNDO_NONE);
				emblemlocations[num-1].x = (INT16)value;
			}
			else if (!strcmp(word, "Y"))
			{
				DEH_WriteUndoline(word, va("%d", emblemlocations[num-1].y), UNDO_NONE);
				emblemlocations[num-1].y = (INT16)value;
			}
			else if (!strcmp(word, "Z"))
			{
				DEH_WriteUndoline(word, va("%d", emblemlocations[num-1].z), UNDO_NONE);
				emblemlocations[num-1].z = (INT16)value;
			}
			else if (!strcmp(word, "PLAYERNUM"))
			{
				DEH_WriteUndoline(word, va("%d", emblemlocations[num-1].player), UNDO_NONE);
				emblemlocations[num-1].player = (UINT8)value;
			}
			else if (!strcmp(word, "MAPNUM"))
			{
				// Support using the actual map name,
				// i.e., Level AB, Level FZ, etc.

				// Convert to map number
				if (word2[0] >= 'A' && word2[0] <= 'Z')
					value = M_MapNumber(word2[0], word2[1]);

				DEH_WriteUndoline(word, va("%d", emblemlocations[num-1].level), UNDO_NONE);
				emblemlocations[num-1].level = (INT16)value;
			}
			else
				deh_warning("Emblem: unknown word '%s'", word);
		}
	} while (!myfeof(f));
}

static void DEH_LoadDehackedFile(MYFILE *f)
{
	XBOXSTATIC char s[1000];
	char *word;
	char *word2;
	INT32 i;
	// do a copy of this for cross references probleme
	XBOXSTATIC actionf_t saveactions[NUMSTATES];
	XBOXSTATIC const char *savesprnames[NUMSPRITES];
	XBOXSTATIC const char *savesfxnames[NUMSFX];

	deh_num_warning = 0;
	// save values for cross reference
	for (i = 0; i < NUMSTATES; i++)
		saveactions[i] = states[i].action;
	for (i = 0; i < NUMSPRITES; i++)
		savesprnames[i] = sprnames[i];
	for (i = 0; i < NUMSFX; i++)
		savesfxnames[i] = S_sfx[i].name;

	// it doesn't test the version of SRB2 and version of dehacked file
	while (!myfeof(f))
	{
		XBOXSTATIC char origpos[32];
		INT32 size = 0;
		char *traverse;

		myfgets(s, sizeof (s), f);
		if (s[0] == '\n' || s[0] == '#')
			continue;

		traverse = s;

		while (traverse[0] != '\n')
		{
			traverse++;
			size++;
		}

		strncpy(origpos, s, size);
		origpos[size] = '\0';

		if (NULL != (word = strtok(s, " ")))
			strupr(word);
		if (word)
		{
			word2 = strtok(NULL, " ");
			if (word2)
			{
				strupr(word2);
				i = atoi(word2);

				if (!strcmp(word, "THING"))
				{
					if (i < NUMMOBJTYPES && i >= 0)
						readthing(f, i);
					else
						deh_warning(text[THING_NOTEXIST], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "MODBY"))
				{
					const INT32 mod = 18;
					strcpy(credits[mod].fakenames[0], origpos+6);
					strcpy(credits[mod].realnames[0], origpos+6);
					credits[mod].numnames = 1;
					strcpy(&credits[mod].header[0], text[MOD_BY]);
					modcredits = true;
				}
/*				else if (!strcmp(word, "ANIMTEX"))
				{
					readAnimTex(f, i);
				}*/
				else if (!strcmp(word, "CHARACTER"))
				{
					if (i < 15)
						readPlayer(f, i);
					else
						deh_warning(text[CHAR_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "LIGHT"))
				{
				}
				else if (!strcmp(word, "SPRITE"))
				{
				}
				else if (!strcmp(word, "LEVEL"))
				{
					// Support using the actual map name,
					// i.e., Level AB, Level FZ, etc.

					// Convert to map number
					if (word2[0] >= 'A' && word2[0] <= 'Z')
						i = M_MapNumber(word2[0], word2[1]);

					if (i > 0 && i <= NUMMAPS)
						readlevelheader(f, i);
					else
						deh_warning(text[LEVEL_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "CUTSCENE"))
				{
					if (i > 0 && i < 129)
						readcutscene(f, i - 1);
					else
						deh_warning(text[CUTSCENE_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "UNLOCKABLE"))
				{
					if (i > 0 && i < 16)
						readunlockable(f, i - 1);
					else
						deh_warning(text[UNLOCKABLE_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "FRAME"))
				{
					if (i < NUMSTATES && i >= 0)
						readframe(f, i);
					else
						deh_warning(text[FRAME_NOTEXIST], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
/*				else if (!strcmp(word, "POINTER"))
				{
					word = strtok(NULL, " "); // get frame
					word = strtok(NULL, ")");
					if (word)
					{
						i = atoi(word);
						if (i < NUMSTATES && i >= 0)
						{
							if (myfgets(s, sizeof (s), f))
								states[i].action = saveactions[searchvalue(s)];
						}
						else
							deh_warning("Pointer: Frame %d doesn't exist", i);
					}
					else
						deh_warning("pointer (Frame %d) : missing ')'", i);
				}*/
				else if (!strcmp(word, "SOUND"))
				{
					if (i < NUMSFX && i >= 0)
						readsound(f, i, savesfxnames);
					else
						deh_warning(text[SOUND_NOTEXIST], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
/*				else if (!strcmp(word, "SPRITE"))
				{
					if (i < NUMSPRITES && i >= 0)
					{
						if (myfgets(s, sizeof (s), f))
						{
							INT32 k;
							k = (searchvalue(s) - 151328)/8;
							if (k >= 0 && k < NUMSPRITES)
								sprnames[i] = savesprnames[k];
							else
								deh_warning("Sprite %d: offset out of bound", i);
						}
					}
					else
						deh_warning("Sprite %d doesn't exist",i);
				}*/
				else if (!strcmp(word, "HUDITEM"))
				{
					if (i >= 0 && i < NUMHUDITEMS)
						readhuditem(f, i);
					else
						deh_warning(text[HUDITEM_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "EMBLEM"))
				{
					if (i > 0 && i <= MAXEMBLEMS)
						reademblemdata(f, i);
					else
						deh_warning(text[EMBLEM_OUTOFRANGE], i);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "MAINCFG"))
				{
					readmaincfg(f);
					DEH_WriteUndoline(word, word2, UNDO_HEADER);
				}
				else if (!strcmp(word, "SRB2"))
				{
					INT32 ver = searchvalue(strtok(NULL, "\n"));
					if (ver != 200)
					{
						deh_warning(text[WRONG_VERSION_WARNING], ver);
						deh_warning("%s", text[SUPPORTED_VERSION]);
					}
					//DEH_WriteUndoline(word, va("%d", ver), UNDO_NONE);
				}
				else
					deh_warning(text[UNKNOWN_WORD], word);
			}
			else
				deh_warning(text[MISSING_ARGUMENT], word);
		}
		else
			deh_warning(text[MISSING_WORD], s);
	} // end while
	if (deh_num_warning)
	{
		CONS_Printf(text[WARNING_IN_SOC_LUMP], deh_num_warning,
			deh_num_warning == 1 ? "" : "s");
		if (devparm)
			while (!I_GetKey())
				I_OsPolling();
	}

	deh_loaded = true;
}

// read dehacked lump in a wad (there is special trick for for deh
// file that are converted to wad in w_wad.c)
void DEH_LoadDehackedLumpPwad(UINT16 wad, UINT16 lump)
{
	MYFILE f;
	unsocwad = wad;
	f.size = W_LumpLengthPwad(wad,lump);
	f.data = Z_Malloc(f.size + 1, PU_STATIC, NULL);
	W_ReadLumpPwad(wad, lump, f.data);
	f.curpos = f.data;
	f.data[f.size] = 0;
	DEH_LoadDehackedFile(&f);
	DEH_WriteUndoline(va("# uload for wad: %u, lump: %u", wad, lump), NULL, UNDO_DONE);
	Z_Free(f.data);
}

void DEH_LoadDehackedLump(lumpnum_t lumpnum)
{
	DEH_LoadDehackedLumpPwad(WADFILENUM(lumpnum),LUMPNUM(lumpnum));
}

#define DUMPUNDONE

// read (un)dehacked lump in wad's memory
void DEH_UnloadDehackedWad(UINT16 wad)
{
	undehacked_t *tmp, *curundo = unsocdata[wad];
	MYFILE f;
	size_t len = 0;
	char *data;
#ifdef DUMPUNDONE
	FILE *UNDO = fopen("undo.soc", "wt");
#endif
	CONS_Printf("%s", text[UNLOADING_SOC_EDITS]);
	while (curundo)
	{
		data = curundo->undata;
		curundo = curundo->next;
		if (data)
			len += strlen(data);
		len += 1;
#ifdef DUMPUNDONE
		if (UNDO)
		{
			if (data)
				fprintf(UNDO, "%s\n", data);
			else
				fprintf(UNDO, "\n");
		}
#endif
	}
#ifndef DUMPUNDONE
	if (UNDO) fclose(UNDO);
#endif
	if (!len) return;
	f.size = len;
	data = f.data = Z_Malloc(f.size + 1, PU_STATIC, NULL);
	curundo = unsocdata[wad];
	unsocdata[wad] = NULL;
	while (curundo)
	{
		tmp = curundo;
		curundo = curundo->next;
		if (tmp->undata)
			data += sprintf(data, "%s\n", tmp->undata);
		else
			data += sprintf(data, "\n");
		if (tmp->undata) free(tmp->undata);
		free(tmp);
	}
	f.curpos = f.data;
	f.data[f.size] = 0;
	disableundo = true;
	DEH_LoadDehackedFile(&f);
	disableundo = false;
	Z_Free(f.data);
}
