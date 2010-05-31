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
/// \brief Archiving: SaveGame I/O, Thinker, Ticker

#include "doomstat.h"
#include "g_game.h"
#include "p_local.h"
#include "z_zone.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "p_polyobj.h"
#include "m_random.h"

tic_t leveltime;

//
// THINKERS
// All thinkers should be allocated by Z_Calloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//

// Both the head and tail of the thinker list.
thinker_t thinkercap;

void Command_Numthinkers_f(void)
{
	INT32 num;
	INT32 count = 0;
	actionf_p1 action;
	thinker_t *think;

	if (gamestate != GS_LEVEL)
	{
		CONS_Printf("%s", text[MUSTBEINLEVEL]);
		return;
	}

	if (COM_Argc() < 2)
	{
		CONS_Printf("numthinkers <#>: Count number of thinkers\n\t1: P_MobjThinker\n\t2: P_RainThinker\n\t3: P_SnowThinker\n\t4: P_NullPrecipThinker\n\t5: T_Friction\n\t6: T_Pusher\n\t7: P_RemoveThinkerDelayed\n");
		return;
	}

	num = atoi(COM_Argv(1));

	switch (num)
	{
		case 1:
			action = (actionf_p1)P_MobjThinker;
			CONS_Printf("Number of P_MobjThinker: ");
			break;
		case 2:
			action = (actionf_p1)P_RainThinker;
			CONS_Printf("Number of P_RainThinker: ");
			break;
		case 3:
			action = (actionf_p1)P_SnowThinker;
			CONS_Printf("Number of P_SnowThinker: ");
			break;
		case 4:
			action = (actionf_p1)P_NullPrecipThinker;
			CONS_Printf("Number of P_NullPrecipThinker: ");
			break;
		case 5:
			action = (actionf_p1)T_Friction;
			CONS_Printf("Number of T_Friction: ");
			break;
		case 6:
			action = (actionf_p1)T_Pusher;
			CONS_Printf("Number of T_Pusher: ");
			break;
		case 7:
			action = (actionf_p1)P_RemoveThinkerDelayed;
			CONS_Printf("Number of P_RemoveThinkerDelayed: ");
			break;
		default:
			CONS_Printf("That is not a valid number.\n");
			return;
	}

	for (think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if (think->function.acp1 != action)
			continue;

		count++;
	}

	CONS_Printf("%d\n", count);
}

void Command_CountMobjs_f(void)
{
	thinker_t *th;
	mobjtype_t i;
	INT32 count;

	if (gamestate != GS_LEVEL)
	{
		CONS_Printf("%s", text[MUSTBEINLEVEL]);
		return;
	}

	if (COM_Argc() >= 2)
	{
		size_t j;
		for (j = 1; j < COM_Argc(); j++)
		{
			i = atoi(COM_Argv(j));
			if (i >= NUMMOBJTYPES)
			{
				CONS_Printf("Object number %d out of range (max %d).\n", i, NUMMOBJTYPES-1);
				continue;
			}

			count = 0;

			for (th = thinkercap.next; th != &thinkercap; th = th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				if (((mobj_t *)th)->type == i)
					count++;
			}

			CONS_Printf("There are %d objects of type %d currently in the level.\n", count, i);
		}
		return;
	}

	CONS_Printf("Count of active objects in level:\n");

	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		count = 0;

		for (th = thinkercap.next; th != &thinkercap; th = th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			if (((mobj_t *)th)->type == i)
				count++;
		}

		CONS_Printf("%d: %d\n", i, count);
	}
	CONS_Printf("Done\n");
}

//
// P_InitThinkers
//
void P_InitThinkers(void)
{
	thinkercap.prev = thinkercap.next = &thinkercap;
}

//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker(thinker_t *thinker)
{
	thinkercap.prev->next = thinker;
	thinker->next = &thinkercap;
	thinker->prev = thinkercap.prev;
	thinkercap.prev = thinker;

	thinker->references = 0;    // killough 11/98: init reference counter to 0
}

//
// killough 11/98:
//
// Make currentthinker external, so that P_RemoveThinkerDelayed
// can adjust currentthinker when thinkers self-remove.

static thinker_t *currentthinker;

//
// P_RemoveThinkerDelayed()
//
// Called automatically as part of the thinker loop in P_RunThinkers(),
// on nodes which are pending deletion.
//
// If this thinker has no more pointers referencing it indirectly,
// remove it, and set currentthinker to one node preceeding it, so
// that the next step in P_RunThinkers() will get its successor.
//
void P_RemoveThinkerDelayed(void *pthinker)
{
	thinker_t *thinker = pthinker;
	if (!thinker->references)
	{
		{
			/* Remove from main thinker list */
			thinker_t *next = thinker->next;
			/* Note that currentthinker is guaranteed to point to us,
			 * and since we're freeing our memory, we had better change that. So
			 * point it to thinker->prev, so the iterator will correctly move on to
			 * thinker->prev->next = thinker->next */
			(next->prev = currentthinker = thinker->prev)->next = next;
		}
		Z_Free(thinker);
    }
}

//
// P_RemoveThinker
//
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
// killough 4/25/98:
//
// Instead of marking the function with -1 value cast to a function pointer,
// set the function to P_RemoveThinkerDelayed(), so that later, it will be
// removed automatically as part of the thinker process.
//
void P_RemoveThinker(thinker_t *thinker)
{
	thinker->function.acp1 = P_RemoveThinkerDelayed;
}

/*
 * P_SetTarget
 *
 * This function is used to keep track of pointer references to mobj thinkers.
 * In Doom, objects such as lost souls could sometimes be removed despite
 * their still being referenced. In Boom, 'target' mobj fields were tested
 * during each gametic, and any objects pointed to by them would be prevented
 * from being removed. But this was incomplete, and was slow (every mobj was
 * checked during every gametic). Now, we keep a count of the number of
 * references, and delay removal until the count is 0.
 */

mobj_t *P_SetTarget(mobj_t **mop, mobj_t *targ)
{
	if (*mop)              // If there was a target already, decrease its refcount
		(*mop)->thinker.references--;
if ((*mop = targ) != NULL) // Set new target and if non-NULL, increase its counter
		targ->thinker.references++;
	return targ;
}

//
// P_RunThinkers
//
// killough 4/25/98:
//
// Fix deallocator to stop using "next" pointer after node has been freed
// (a Doom bug).
//
// Process each thinker. For thinkers which are marked deleted, we must
// load the "next" pointer prior to freeing the node. In Doom, the "next"
// pointer was loaded AFTER the thinker was freed, which could have caused
// crashes.
//
// But if we are not deleting the thinker, we should reload the "next"
// pointer after calling the function, in case additional thinkers are
// added at the end of the list.
//
// killough 11/98:
//
// Rewritten to delete nodes implicitly, by making currentthinker
// external and using P_RemoveThinkerDelayed() implicitly.
//
static inline void P_RunThinkers(void)
{
	for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
	{
		if (currentthinker->function.acp1)
			currentthinker->function.acp1(currentthinker);
	}
}

//
// P_DoAutobalanceTeams()
//
// Determine if the teams are unbalanced, and if so, move a player to the other team.
//
static void P_DoAutobalanceTeams(void)
{
	changeteam_union NetPacket;
	UINT16 usvalue;
	INT32 i=0;
	INT32 red=0, blue=0;
	INT32 redarray[MAXPLAYERS], bluearray[MAXPLAYERS];
	INT32 redflagcarrier = 0, blueflagcarrier = 0;
	INT32 totalred = 0, totalblue = 0;

	NetPacket.value.l = NetPacket.value.b = 0;
	memset(redarray, 0, sizeof(redarray));
	memset(bluearray, 0, sizeof(bluearray));

	// Only do it if we have enough room in the net buffer to send it.
	// Otherwise, come back next time and try again.
	if (sizeof(usvalue) > GetFreeXCmdSize())
		return;

	//We have to store the players in an array with the rest of their team.
	//We can then pick a random player to be forced to change teams.
	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].ctfteam)
		{
			if (players[i].ctfteam == 1)
			{
				if (!players[i].gotflag)
				{
					redarray[red] = i; //store the player's node.
					red++;
				}
				else
					redflagcarrier++;
			}
			else
			{
				if (!players[i].gotflag)
				{
					bluearray[blue] = i; //store the player's node.
					blue++;
				}
				else
					blueflagcarrier++;
			}
		}
	}

	totalred = red + redflagcarrier;
	totalblue = blue + blueflagcarrier;

	if ((abs(totalred - totalblue) > cv_autobalance.value))
	{
		if (totalred > totalblue)
		{
			i = M_Random() % red;
			NetPacket.packet.newteam = 2;
			NetPacket.packet.playernum = redarray[i];
			NetPacket.packet.verification = true;
			NetPacket.packet.autobalance = true;

			usvalue  = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}

		if (totalblue > totalred)
		{
			i = M_Random() % blue;
			NetPacket.packet.newteam = 1;
			NetPacket.packet.playernum = bluearray[i];
			NetPacket.packet.verification = true;
			NetPacket.packet.autobalance = true;

			usvalue  = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}
	}
}

//
// P_DoTeamscrambling()
//
// If a team scramble has been started, scramble one person from the
// pre-made scramble array. Said array is created in TeamScramble_OnChange()
//
void P_DoTeamscrambling(void)
{
	changeteam_union NetPacket;
	UINT16 usvalue;
	NetPacket.value.l = NetPacket.value.b = 0;

	// Only do it if we have enough room in the net buffer to send it.
	// Otherwise, come back next time and try again.
	if (sizeof(usvalue) > GetFreeXCmdSize())
		return;

	if (scramblecount < scrambletotal)
	{
		if (players[scrambleplayers[scramblecount]].ctfteam != scrambleteams[scramblecount])
		{
			NetPacket.packet.newteam = scrambleteams[scramblecount];
			NetPacket.packet.playernum = scrambleplayers[scramblecount];
			NetPacket.packet.verification = true;
			NetPacket.packet.scrambled = true;

			usvalue = SHORT(NetPacket.value.l|NetPacket.value.b);
			SendNetXCmd(XD_TEAMCHANGE, &usvalue, sizeof(usvalue));
		}

		scramblecount++; //Increment, and get to the next player when we come back here next time.
	}
	else
		CV_SetValue(&cv_teamscramble, 0);
}

//
// P_Ticker
//
void P_Ticker(void)
{
	INT32 i;

	//Increment jointime even if paused.
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i])
			++players[i].jointime;

	// Check for pause or menu up in single player
	if (paused || (!netgame && menuactive && !demoplayback))
	{
		objectsdrawn = 0;
		return;
	}

	postimgtype = postimg_none;

	P_MapStart();

	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo)
			P_PlayerThink(&players[i]);

	// Keep track of how long they've been playing!
	totalplaytime++;

///////////////////////
//SPECIAL STAGE STUFF//
///////////////////////

	if (!useNightsSS && gamemap >= sstage_start && gamemap <= sstage_end)
	{
		boolean inwater = false;

		// Can't drown in a special stage
		for (i = 0; i < MAXPLAYERS; i++)
		{
			if (!playeringame[i])
				continue;

			players[i].powers[pw_underwater] = players[i].powers[pw_spacetime] = 0;
		}

		if (sstimer < 15*TICRATE+6 && sstimer > 7 && mapheaderinfo[gamemap-1].speedmusic)
			S_SpeedMusic(1.4f);

		if (sstimer < 7 && sstimer > 0) // The special stage time is up!
		{
			sstimer = 0;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (playeringame[i])
				{
					players[i].exiting = (14*TICRATE)/5 + 1;
					players[i].pflags &= ~PF_GLIDING;
				}

				if (i == consoleplayer)
					S_StartSound(NULL, sfx_lose);
			}

			if (mapheaderinfo[gamemap-1].speedmusic)
				S_SpeedMusic(1.0f);
		}

		if (sstimer > 1) // As long as time isn't up...
		{
			UINT32 ssrings = 0;
			// Count up the rings of all the players and see if
			// they've collected the required amount.
			for (i = 0; i < MAXPLAYERS; i++)
				if (playeringame[i])
				{
					ssrings += (players[i].mo->health-1);

					// If in water, deplete timer 6x as fast.
					if ((players[i].mo->eflags & MFE_TOUCHWATER)
						|| (players[i].mo->eflags & MFE_UNDERWATER))
						inwater = true;
				}

			if (ssrings >= totalrings && totalrings > 0)
			{
				// Halt all the players
				for (i = 0; i < MAXPLAYERS; i++)
					if (playeringame[i])
					{
						players[i].mo->momx = players[i].mo->momy = 0;
						players[i].exiting = (14*TICRATE)/5 + 1;
					}

				sstimer = 0;

				P_GiveEmerald();
			}

			// Decrement the timer
			if (!cv_objectplace.value)
			{
				if (inwater)
					sstimer -= 6;
				else
					sstimer--;
			}
		}
	}
/////////////////////////////////////////

	if (runemeraldmanager)
		P_EmeraldManager(); // Power stone mode

	P_RunThinkers();

	// Run any "after all the other thinkers" stuff
	for (i = 0; i < MAXPLAYERS; i++)
		if (playeringame[i] && players[i].mo)
			P_PlayerAfterThink(&players[i]);

	// Run shield positioning
	P_RunShields();

	P_UpdateSpecials();
	P_RespawnSpecials();

	if (cv_objectplace.value)
	{
		objectsdrawn = 0;

		if (P_FreezeObjectplace())
		{
			P_MapEnd();
			return;
		}
	}

	leveltime++;
	timeinmap++;

	if (gametype == GT_TAG)
	{
		// tell the netgame who the initial IT person is.
		// this can't happen on level load, otherwise you have
		// to open the console to see the message.
		if (leveltime == TICRATE)
		{
			for (i=0; i < MAXPLAYERS; i++)
			{
				if (players[i].pflags & PF_TAGIT)
				{
					CONS_Printf("%s is it!\n", player_names[i]); // Tell everyone who is it!
					break;
				}
			}
		}

		//increment survivor scores
		if (leveltime % TICRATE == 0 && leveltime > (hidetime * TICRATE))
		{
			INT32 spectators = 0;

			for (i=0; i < MAXPLAYERS; i++) //count spectators to subtract from the player count.
			{
				if (players[i].spectator)
					spectators++;
			}

			for (i=0; i < MAXPLAYERS; i++)
			{
				if (!(players[i].pflags & PF_TAGIT) && !(players[i].pflags & PF_TAGGED)
					&& !players[i].spectator && playeringame[i] && players[i].playerstate == PST_LIVE)
					P_AddPlayerScore(&players[i], (D_NumPlayers() - spectators) / 2); //points given is the number of participating players divided by two.
			}
		}
	}

	if (gametype == GT_CTF || (gametype == GT_MATCH && cv_matchtype.value))
	{
		// Automatic team balance for CTF and team match
		if (leveltime % (TICRATE * 5) == 0) //only check once per five seconds for the sake of CPU conservation.
		{
			// Do not attempt to autobalance and scramble teams at the same time.
			// Only the server should execute this. No verified admins, please.
			if ((cv_autobalance.value && !cv_teamscramble.value) && cv_allowteamchange.value && server)
				P_DoAutobalanceTeams();
		}

		// Team scramble code for team match and CTF.
		if ((leveltime % (TICRATE/7)) == 0)
		{
			// If we run out of time in the level, the beauty is that
			// the Y_Ticker() team scramble code will pick it up.
			if (cv_teamscramble.value && server)
				P_DoTeamscrambling();
		}
	}

	if (countdowntimer)
	{
		countdowntimer--;
		if (countdowntimer <= 0)
		{
			countdowntimer = 0;
			countdowntimeup = true;
			for (i = 0; i < MAXPLAYERS; i++)
			{
				if (!playeringame[i])
					continue;

				if (!players[i].mo)
					continue;

				P_DamageMobj(players[i].mo, NULL, NULL, 10000);
			}
		}
	}

	if (countdown > 1)
		countdown--;

	if (countdown2)
		countdown2--;

	P_MapEnd();

//	Z_CheckMemCleanup();
}
