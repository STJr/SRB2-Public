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
/// \brief Networking stuff.
///
///	part of layer 4 (transport) (tp4) of the osi model
///	assure the reception of packet and proceed a checksums
///
///	There is a data struct that stores network
///	communication related stuff, and another
///	one that defines the actual packets to
///	be transmitted.

#ifndef __D_NET__
#define __D_NET__

// Max computers in a game.
#define MAXNETNODES 32
#define BROADCASTADDR MAXNETNODES
#define MAXSPLITSCREENPLAYERS 2 // max number of players on a single computer

#define STATLENGTH (TICRATE*2)

// stat of net
extern INT32 ticruned, ticmiss;
extern INT32 getbps, sendbps;
extern float lostpercent, duppercent, gamelostpercent;
extern INT32 packetheaderlength;
boolean Net_GetNetStat(void);
extern INT32 getbytes;
extern INT64 sendbytes; // realtime updated

extern SINT8 nodetoplayer[MAXNETNODES];
extern SINT8 nodetoplayer2[MAXNETNODES]; // say the numplayer for this node if any (splitscreen)
extern UINT8 playerpernode[MAXNETNODES]; // used specialy for scplitscreen
extern boolean nodeingame[MAXNETNODES]; // set false as nodes leave game

void Net_AckTicker(void);
boolean Net_AllAckReceived(void);

// if reliable return true if packet sent, 0 else
boolean HSendPacket(INT32 node, boolean reliable, UINT8 acknum,
	size_t packetlength);
boolean HGetPacket(void);
void D_SetDoomcom(void);
#ifndef NONET
void D_SaveBan(void);
#endif
boolean D_CheckNetGame(void);
void D_CloseConnection(void);
void Net_UnAcknowledgPacket(INT32 node);
void Net_CloseConnection(INT32 node);
void Net_AbortPacketType(UINT8 packettype);
void Net_SendAcks(INT32 node);
void Net_WaitAllAckReceived(UINT32 timeout);
#endif
