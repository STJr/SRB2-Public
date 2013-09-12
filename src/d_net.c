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
/// \brief SRB2 network game communication and protocol, all OS independent parts.
//
///	Implement a Sliding window protocol without receiver window
///	(out of order reception)
///	This protocol uses a mix of "goback n" and "selective repeat" implementation
///	The NOTHING packet is sent when connection is idle to acknowledge packets

#include "doomdef.h"
#include "dstrings.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "d_clisrv.h"
#include "z_zone.h"
#include "i_tcp.h"

//
// NETWORKING
//
// gametic is the tic about to be (or currently being) run
// server:
//   maketic is the tic that hasn't had control made for it yet
//   nettics: is the tic for each node
//   firsttictosend: is the lowest value of nettics
// client:
//   neededtic: is the tic needed by the client to run the game
//   firsttictosend: is used to optimize a condition
// normally maketic >= gametic > 0

#define FORCECLOSE 0x8000
tic_t connectiontimeout = (15*TICRATE);

/// \brief network packet
doomcom_t *doomcom = NULL;
/// \brief network packet data, points inside doomcom
doomdata_t *netbuffer = NULL;

FILE *debugfile = NULL; // put some net info in a file during the game

#define MAXREBOUND 8
static doomdata_t reboundstore[MAXREBOUND];
static INT16 reboundsize[MAXREBOUND];
static INT32 rebound_head, rebound_tail;

/// \brief bandwith of netgame
INT32 net_bandwidth;

/// \brief max length per packet
INT16 hardware_MAXPACKETLENGTH;

void (*I_NetGet)(void) = NULL;
void (*I_NetSend)(void) = NULL;
boolean (*I_NetCanSend)(void) = NULL;
boolean (*I_NetCanGet)(void) = NULL;
void (*I_NetCloseSocket)(void) = NULL;
void (*I_NetFreeNodenum)(INT32 nodenum) = NULL;
SINT8 (*I_NetMakeNode)(const char *address) = NULL;
boolean (*I_NetOpenSocket)(void) = NULL;
boolean (*I_Ban) (INT32 node) = NULL;
boolean (*I_Shun) (INT32 node) = NULL;
void (*I_ClearBans)(void) = NULL;
const char *(*I_GetNodeAddress) (INT32 node) = NULL;
const char *(*I_GetBanAddress) (size_t ban) = NULL;
const char *(*I_GetBanMask) (size_t ban) = NULL;
boolean (*I_SetBanAddress) (const char *address, const char *mask) = NULL;
boolean *bannednode = NULL;


// network stats
static tic_t statstarttic;
INT32 getbytes = 0;
INT64 sendbytes = 0;
static INT32 retransmit = 0, duppacket = 0;
static INT32 sendackpacket = 0, getackpacket = 0;
INT32 ticruned = 0, ticmiss = 0;

// globals
INT32 getbps, sendbps;
float lostpercent, duppercent, gamelostpercent;
INT32 packetheaderlength;

boolean Net_GetNetStat(void)
{
	const tic_t t = I_GetTime();
	static INT64 oldsendbyte = 0;
	if (statstarttic+STATLENGTH <= t)
	{
		const tic_t df = t-statstarttic;
		const INT64 newsendbyte = sendbytes - oldsendbyte;
		sendbps = (INT32)(newsendbyte*TICRATE)/df;
		getbps = (getbytes*TICRATE)/df;
		if (sendackpacket)
			lostpercent = 100.0f*(float)retransmit/(float)sendackpacket;
		else
			lostpercent = 0.0f;
		if (getackpacket)
			duppercent = 100.0f*(float)duppacket/(float)getackpacket;
		else
			duppercent = 0.0f;
		if (ticruned)
			gamelostpercent = 100.0f*(float)ticmiss/(float)ticruned;
		else
			gamelostpercent = 0.0f;

		ticmiss = ticruned = 0;
		oldsendbyte = sendbytes;
		getbytes = 0;
		sendackpacket = getackpacket = duppacket = retransmit = 0;
		statstarttic = t;

		return 1;
	}
	return 0;
}

// -----------------------------------------------------------------
// Some structs and functions for acknowledgement of packets
// -----------------------------------------------------------------
#define MAXACKPACKETS 64 // minimum number of nodes
#define MAXACKTOSEND 64
#define URGENTFREESLOTENUM 6
#define ACKTOSENDTIMEOUT (TICRATE/17)

typedef struct
{
	UINT8 acknum;
	UINT8 nextacknum;
	UINT8 destinationnode;
	tic_t senttime;
	UINT16 length;
	UINT16 resentnum;
	union {
		SINT8 raw[MAXPACKETLENGTH];
		doomdata_t data;
	} pak;
} ackpak_t;

typedef enum
{
	CLOSE = 1, // flag is set when connection is closing
} node_flags_t;

// table of packet that was not acknowleged can be resend (the sender window)
static ackpak_t ackpak[MAXACKPACKETS];

typedef struct
{
	// ack return to send (like sliding window protocol)
	UINT8 firstacktosend;

	// when no consecutive packets are received we keep in mind what packets
	// we already received in a queue
	UINT8 acktosend_head;
	UINT8 acktosend_tail;
	UINT8 acktosend[MAXACKTOSEND];

	// automatically send keep alive packet when not enough trafic
	tic_t lasttimeacktosend_sent;
	// detect connection lost
	tic_t lasttimepacketreceived;

	// flow control: do not send too many packets with ack
	UINT8 remotefirstack;
	UINT8 nextacknum;

	UINT8 flags;
#ifndef NEWPING
	// jacobson tcp timeout evaluation algorithm (Karn variation)
	fixed_t ping;
	fixed_t varping;
	INT32 timeout; // computed with ping and varping
#endif
} node_t;

static node_t nodes[MAXNETNODES];
#ifndef NEWPING
#define PINGDEFAULT ((200*TICRATE*FRACUNIT)/1000)
#define VARPINGDEFAULT ((50*TICRATE*FRACUNIT)/1000)
#define TIMEOUT(p,v) (p+4*v+FRACUNIT/2)>>FRACBITS;
#else
#define NODETIMEOUT 14 //What the above boiled down to...
#endif

// return <0 if a < b (mod 256)
//         0 if a = n (mod 256)
//        >0 if a > b (mod 256)
// mnemonic: to use it compare to 0: cmpack(a,b)<0 is "a < b" ...
FUNCMATH static INT32 cmpack(UINT8 a, UINT8 b)
{
	register INT32 d = a - b;

	if (d >= 127 || d < -128)
		return -d;
	return d;
}

// return a free acknum and copy netbuffer in the ackpak table
static boolean GetFreeAcknum(UINT8 *freeack, boolean lowtimer)
{
	node_t *node = &nodes[doomcom->remotenode];
	INT32 i, numfreeslote = 0;

	if (cmpack((UINT8)((node->remotefirstack + MAXACKTOSEND) % 256), node->nextacknum) < 0)
	{
		DEBFILE(va("too fast %d %d\n",node->remotefirstack,node->nextacknum));
		return false;
	}

	for (i = 0; i < MAXACKPACKETS; i++)
		if (!ackpak[i].acknum)
		{
			// for low priority packet, make sure let freeslotes so urgents packets can be sent
			numfreeslote++;
			if (netbuffer->packettype >= PT_CANFAIL && numfreeslote < URGENTFREESLOTENUM)
				continue;

			ackpak[i].acknum = node->nextacknum;
			ackpak[i].nextacknum = node->nextacknum;
			node->nextacknum++;
			if (!node->nextacknum)
				node->nextacknum++;
			ackpak[i].destinationnode = (UINT8)(node - nodes);
			ackpak[i].length = doomcom->datalength;
			if (lowtimer)
			{
				// lowtime mean can't be sent now so try it soon as possible
				ackpak[i].senttime = 0;
				ackpak[i].resentnum = 1;
			}
			else
			{
				ackpak[i].senttime = I_GetTime();
				ackpak[i].resentnum = 0;
			}
			M_Memcpy(ackpak[i].pak.raw, netbuffer, ackpak[i].length);

			*freeack = ackpak[i].acknum;

			sendackpacket++; // for stat

			return true;
		}
#ifdef PARANOIA
	if (devparm)
		I_OutputMsg("No more free ackpacket\n");
#endif
	if (netbuffer->packettype < PT_CANFAIL)
		I_Error("Connection lost\n");
	return false;
}

// Get a ack to send in the queu of this node
static UINT8 GetAcktosend(INT32 node)
{
	nodes[node].lasttimeacktosend_sent = I_GetTime();
	return nodes[node].firstacktosend;
}

static void Removeack(INT32 i)
{
	INT32 node = ackpak[i].destinationnode;
#ifndef NEWPING
	fixed_t trueping = (I_GetTime() - ackpak[i].senttime)<<FRACBITS;
	if (ackpak[i].resentnum)
	{
		// +FRACUNIT/2 for round
		nodes[node].ping = (nodes[node].ping*7 + trueping)/8;
		nodes[node].varping = (nodes[node].varping*7 + abs(nodes[node].ping-trueping))/8;
		nodes[node].timeout = TIMEOUT(nodes[node].ping,nodes[node].varping);
	}
	DEBFILE(va("Remove ack %d trueping %d ping %f var %f timeout %d\n",ackpak[i].acknum,trueping>>FRACBITS,(double)FIXED_TO_FLOAT(nodes[node].ping),(double)FIXED_TO_FLOAT(nodes[node].varping),nodes[node].timeout));
#else
	DEBFILE(va("Remove ack %d\n",ackpak[i].acknum));
#endif
	ackpak[i].acknum = 0;
	if (nodes[node].flags & CLOSE)
		Net_CloseConnection(node);
}

// we have got a packet proceed the ack request and ack return
static boolean Processackpak(void)
{
	INT32 i;
	boolean goodpacket = true;
	node_t *node = &nodes[doomcom->remotenode];

	// received an ack return, so remove the ack in the list
	if (netbuffer->ackreturn && cmpack(node->remotefirstack, netbuffer->ackreturn) < 0)
	{
		node->remotefirstack = netbuffer->ackreturn;
		// search the ackbuffer and free it
		for (i = 0; i < MAXACKPACKETS; i++)
			if (ackpak[i].acknum && ackpak[i].destinationnode == node - nodes
				&& cmpack(ackpak[i].acknum, netbuffer->ackreturn) <= 0)
			{
				Removeack(i);
			}
	}

	// received a packet with ack, queue it to send the ack back
	if (netbuffer->ack)
	{
		UINT8 ack = netbuffer->ack;
		getackpacket++;
		if (cmpack(ack, node->firstacktosend) <= 0)
		{
			DEBFILE(va("Discard(1) ack %d (duplicated)\n", ack));
			duppacket++;
			goodpacket = false; // discard packet (duplicate)
		}
		else
		{
			// check if it is not already in the queue
			for (i = node->acktosend_tail; i != node->acktosend_head; i = (i+1) % MAXACKTOSEND)
				if (node->acktosend[i] == ack)
				{
					DEBFILE(va("Discard(2) ack %d (duplicated)\n", ack));
					duppacket++;
					goodpacket = false; // discard packet (duplicate)
					break;
				}
			if (goodpacket)
			{
				// is a good packet so increment the acknowledge number,
				// then search for a "hole" in the queue
				UINT8 nextfirstack = (UINT8)(node->firstacktosend + 1);
				if (!nextfirstack)
					nextfirstack = 1;

				if (ack == nextfirstack)
				{
					UINT8 hm1; // head - 1
					boolean change = true;

					node->firstacktosend = nextfirstack++;
					if (!nextfirstack)
						nextfirstack = 1;
					hm1 = (UINT8)((node->acktosend_head-1+MAXACKTOSEND) % MAXACKTOSEND);
					while (change)
					{
						change = false;
						for (i = node->acktosend_tail; i != node->acktosend_head;
							i = (i+1) % MAXACKTOSEND)
						{
							if (cmpack(node->acktosend[i], nextfirstack) <= 0)
							{
								if (node->acktosend[i] == nextfirstack)
								{
									node->firstacktosend = nextfirstack++;
									if (!nextfirstack)
										nextfirstack = 1;
									change = true;
								}
								if (i == node->acktosend_tail)
								{
									node->acktosend[node->acktosend_tail] = 0;
									node->acktosend_tail = (UINT8)((i+1) % MAXACKTOSEND);
								}
								else if (i == hm1)
								{
									node->acktosend[hm1] = 0;
									node->acktosend_head = hm1;
									hm1 = (UINT8)((hm1-1+MAXACKTOSEND) % MAXACKTOSEND);
								}
							}
						}
					}
				}
				else // out of order packet
				{
					// don't increment firsacktosend, put it in asktosend queue
					// will be incremented when the nextfirstack comes (code above)
					UINT8 newhead = (UINT8)((node->acktosend_head+1) % MAXACKTOSEND);
					DEBFILE(va("out of order packet (%d expected)\n", nextfirstack));
					if (newhead != node->acktosend_tail)
					{
						node->acktosend[node->acktosend_head] = ack;
						node->acktosend_head = newhead;
					}
					else // buffer full discard packet, sender will resend it
					{ // we can admit the packet but we will not detect the duplication after :(
						DEBFILE("no more freeackret\n");
						goodpacket = false;
					}
				}
			}
		}
	}
	return goodpacket;
}

// send special packet with only ack on it
void Net_SendAcks(INT32 node)
{
	netbuffer->packettype = PT_NOTHING;
	M_Memcpy(netbuffer->u.textcmd, nodes[node].acktosend, MAXACKTOSEND);
	HSendPacket(node, false, 0, MAXACKTOSEND);
}

static void GotAcks(void)
{
	INT32 i, j;

	for (j = 0; j < MAXACKTOSEND; j++)
		if (netbuffer->u.textcmd[j])
			for (i = 0; i < MAXACKPACKETS; i++)
				if (ackpak[i].acknum && ackpak[i].destinationnode == doomcom->remotenode)
				{
					if (ackpak[i].acknum == netbuffer->u.textcmd[j])
						Removeack(i);
					else
						// nextacknum is first equal to acknum, then when receiving bigger ack
						// there is big chance the packet is lost
						// when resent, nextacknum = nodes[node].nextacknum
						//    will redo the same but with different value
					if (cmpack(ackpak[i].nextacknum, netbuffer->u.textcmd[j]) <= 0
						&& ackpak[i].senttime > 0)
					{
						ackpak[i].senttime--; // hurry up
					}
				}
}

static inline void Net_ConnectionTimeout(INT32 node)
{
	// send a very special packet to self (hack the reboundstore queue)
	// main code will handle it
	reboundstore[rebound_head].packettype = PT_NODETIMEOUT;
	reboundstore[rebound_head].ack = 0;
	reboundstore[rebound_head].ackreturn = 0;
	reboundstore[rebound_head].u.textcmd[0] = (UINT8)node;
	reboundsize[rebound_head] = (INT16)(BASEPACKETSIZE + 1);
	rebound_head = (rebound_head+1) % MAXREBOUND;

	// do not redo it quickly (if we do not close connection it is
	// for a good reason!)
	nodes[node].lasttimepacketreceived = I_GetTime();
}

// resend the data if needed
void Net_AckTicker(void)
{
	INT32 i;

	for (i = 0; i < MAXACKPACKETS; i++)
	{
		const INT32 nodei = ackpak[i].destinationnode;
		node_t *node = &nodes[nodei];
#ifdef NEWPING
		if (ackpak[i].acknum && ackpak[i].senttime + NODETIMEOUT < I_GetTime())
#else
		if (ackpak[i].acknum && ackpak[i].senttime + node->timeout < I_GetTime())
#endif
		{
			if (ackpak[i].resentnum > 10 && (node->flags & CLOSE))
			{
				DEBFILE(va("ack %d sent 10 times so connection is supposed lost: node %d\n",
					i, nodei));
				Net_CloseConnection(nodei | FORCECLOSE);

				ackpak[i].acknum = 0;
				continue;
			}
#ifdef NEWPING
			DEBFILE(va("Resend ack %d, %u<%d at %u\n", ackpak[i].acknum, ackpak[i].senttime,
				NODETIMEOUT, I_GetTime()));
#else
			DEBFILE(va("Resend ack %d, %u<%d at %u\n", ackpak[i].acknum, ackpak[i].senttime,
				node->timeout, I_GetTime()));
#endif
			M_Memcpy(netbuffer, ackpak[i].pak.raw, ackpak[i].length);
			ackpak[i].senttime = I_GetTime();
			ackpak[i].resentnum++;
			ackpak[i].nextacknum = node->nextacknum;
			retransmit++; // for stat
			HSendPacket((INT32)(node - nodes), false, ackpak[i].acknum,
				(size_t)(ackpak[i].length - BASEPACKETSIZE));
		}
	}

	for (i = 1; i < MAXNETNODES; i++)
	{
		// this is something like node open flag
		if (nodes[i].firstacktosend)
		{
			// we haven't sent a packet for a long time
			// acknowledge packet if needed
			if (nodes[i].lasttimeacktosend_sent + ACKTOSENDTIMEOUT < I_GetTime())
				Net_SendAcks(i);

			if (!(nodes[i].flags & CLOSE)
				&& nodes[i].lasttimepacketreceived + connectiontimeout < I_GetTime())
			{
				Net_ConnectionTimeout(i);
			}
		}
	}
}

// remove last packet received ack before resending the ackret
// (the higher layer doesn't have room, or something else ....)
void Net_UnAcknowledgPacket(INT32 node)
{
	INT32 hm1 = (nodes[node].acktosend_head-1+MAXACKTOSEND) % MAXACKTOSEND;
	DEBFILE(va("UnAcknowledge node %d\n", node));
	if (!node)
		return;
	if (nodes[node].acktosend[hm1] == netbuffer->ack)
	{
		nodes[node].acktosend[hm1] = 0;
		nodes[node].acktosend_head = (UINT8)hm1;
	}
	else if (nodes[node].firstacktosend == netbuffer->ack)
	{
		nodes[node].firstacktosend--;
		if (!nodes[node].firstacktosend)
			nodes[node].firstacktosend = UINT8_MAX;
	}
	else
	{
		while (nodes[node].firstacktosend != netbuffer->ack)
		{
			nodes[node].acktosend_tail = (UINT8)
				((nodes[node].acktosend_tail-1+MAXACKTOSEND) % MAXACKTOSEND);
			nodes[node].acktosend[nodes[node].acktosend_tail] = nodes[node].firstacktosend;

			nodes[node].firstacktosend--;
			if (!nodes[node].firstacktosend)
				nodes[node].firstacktosend = UINT8_MAX;
		}
		nodes[node].firstacktosend++;
		if (!nodes[node].firstacktosend)
			nodes[node].firstacktosend = 1;
	}
}

boolean Net_AllAckReceived(void)
{
	INT32 i;

	for (i = 0; i < MAXACKPACKETS; i++)
		if (ackpak[i].acknum)
			return false;

	return true;
}

// wait for all ackreturns with timeout in seconds
void Net_WaitAllAckReceived(UINT32 timeout)
{
	tic_t tictac = I_GetTime();
	timeout = tictac + timeout*TICRATE;

	HGetPacket();
	while (timeout > I_GetTime() && !Net_AllAckReceived())
	{
		while (tictac == I_GetTime())
			I_Sleep();
		tictac = I_GetTime();
		HGetPacket();
		Net_AckTicker();
	}
}

static void InitNode(INT32 node)
{
	nodes[node].acktosend_head = nodes[node].acktosend_tail = 0;
#ifndef NEWPING
	nodes[node].ping = PINGDEFAULT;
	nodes[node].varping = VARPINGDEFAULT;
	nodes[node].timeout = TIMEOUT(nodes[node].ping,nodes[node].varping);
#endif
	nodes[node].firstacktosend = 0;
	nodes[node].nextacknum = 1;
	nodes[node].remotefirstack = 0;
	nodes[node].flags = 0;
}

static void InitAck(void)
{
	INT32 i;

	for (i = 0; i < MAXACKPACKETS; i++)
		ackpak[i].acknum = 0;

	for (i = 0; i < MAXNETNODES; i++)
		InitNode(i);
}

void Net_AbortPacketType(UINT8 packettype)
{
	INT32 i;
	for (i = 0; i < MAXACKPACKETS; i++)
		if (ackpak[i].acknum && (ackpak[i].pak.data.packettype == packettype
			|| packettype == UINT8_MAX))
		{
			ackpak[i].acknum = 0;
		}
}

// -----------------------------------------------------------------
// end of acknowledge function
// -----------------------------------------------------------------

// remove a node, clear all ack from this node and reset askret
void Net_CloseConnection(INT32 node)
{
	INT32 i;
	boolean forceclose = (node & FORCECLOSE) != 0;
	node &= ~FORCECLOSE;

	if (!node)
		return;

	nodes[node].flags |= CLOSE;

	// try to Send ack back (two army problem)
	if (GetAcktosend(node))
	{
		Net_SendAcks(node);
		Net_SendAcks(node);
	}

	// check if we are waiting for an ack from this node
	for (i = 0; i < MAXACKPACKETS; i++)
		if (ackpak[i].acknum && ackpak[i].destinationnode == node)
		{
			if (!forceclose)
				return; // connection will be closed when ack is returned
			else
				ackpak[i].acknum = 0;
		}

	InitNode(node);
	AbortSendFiles(node);
	I_NetFreeNodenum(node);
}

//
// Checksum
//
static UINT32 NetbufferChecksum(void)
{
	UINT32 c = 0x1234567;
	const INT32 l = doomcom->datalength - 4;
	const UINT8 *buf = (UINT8 *)netbuffer + 4;
	INT32 i;

	for (i = 0; i < l; i++, buf++)
		c += (*buf) * (i+1);

	return LONG(c);
}

#ifdef DEBUGFILE

static void fprintfstring(char *s, size_t len)
{
	INT32 mode = 0;
	size_t i;

	for (i = 0; i < len; i++)
		if (s[i] < 32)
		{
			if (!mode)
			{
				fprintf(debugfile, "[%d", (UINT8)s[i]);
				mode = 1;
			}
			else
				fprintf(debugfile, ",%d", (UINT8)s[i]);
		}
		else
		{
			if (mode)
			{
				fprintf(debugfile, "]");
				mode = 0;
			}
			fprintf(debugfile, "%c", s[i]);
		}
	if (mode)
		fprintf(debugfile, "]");
	fprintf(debugfile, "\n");
}

static const char *packettypename[NUMPACKETTYPE] =
{
	"NOTHING",
	"SERVERCFG",
	"CLIENTCMD",
	"CLIENTMIS",
	"CLIENT2CMD",
	"CLIENT2MIS",
	"NODEKEEPALIVE",
	"NODEKEEPALIVEMIS",
	"SERVERTICS",
	"SERVERREFUSE",
	"SERVERSHUTDOWN",
	"CLIENTQUIT",

	"ASKINFO",
	"SERVERINFO",
	"REQUESTFILE",
	"ASKINFOVIAMS",

	"FILEFRAGMENT",
	"TEXTCMD",
	"TEXTCMD2",
	"CLIENTJOIN",
	"NODETIMEOUT",
};

static void DebugPrintpacket(const char *header)
{
	fprintf(debugfile, "%-12s (node %d,ack %d,ackret %d,size %d) type(%d) : %s\n",
		header, doomcom->remotenode, netbuffer->ack, netbuffer->ackreturn, doomcom->datalength,
		netbuffer->packettype, packettypename[netbuffer->packettype]);

	switch (netbuffer->packettype)
	{
		case PT_ASKINFO:
		case PT_ASKINFOVIAMS:
			fprintf(debugfile, "    time %u\n", (tic_t)LONG(netbuffer->u.askinfo.time)				);
			break;
		case PT_CLIENTJOIN:
			fprintf(debugfile, "    number %d mode %d\n", netbuffer->u.clientcfg.localplayers,
				netbuffer->u.clientcfg.mode);
			break;
		case PT_SERVERTICS:
			fprintf(debugfile, "    firsttic %u ply %d tics %d ntxtcmd %"PRIdS"\n    ",
				(UINT32)ExpandTics(netbuffer->u.serverpak.starttic), netbuffer->u.serverpak.numslots,
				netbuffer->u.serverpak.numtics,
				(size_t)(&((UINT8 *)netbuffer)[doomcom->datalength] - (UINT8 *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numslots*netbuffer->u.serverpak.numtics]));
			fprintfstring((char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numslots*netbuffer->u.serverpak.numtics],(size_t)(
				&((UINT8 *)netbuffer)[doomcom->datalength] - (UINT8 *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numslots*netbuffer->u.serverpak.numtics]));
			break;
		case PT_CONSISTENCY:
			fprintf(debugfile, "    randomseed %d playernum %d hasmo %d\n",
				netbuffer->u.consistency.randomseed, netbuffer->u.consistency.playernum, netbuffer->u.consistency.hasmo);
			fprintf(debugfile, "    x %d y %d z %d momx %d momy %d momz %d\n",
				netbuffer->u.consistency.x, netbuffer->u.consistency.y, netbuffer->u.consistency.z,
				netbuffer->u.consistency.momx, netbuffer->u.consistency.momy, netbuffer->u.consistency.momz);
			fprintf(debugfile, "    angle %d health %d eflags %d flags %d flags2 %d\n",
				netbuffer->u.consistency.angle, netbuffer->u.consistency.health, netbuffer->u.consistency.eflags,
				netbuffer->u.consistency.flags, netbuffer->u.consistency.flags2);
			fprintf(debugfile, "    friction %d movefactor %d tics %d statenum %d\n",
				netbuffer->u.consistency.friction, netbuffer->u.consistency.movefactor,
				netbuffer->u.consistency.tics, (INT32)netbuffer->u.consistency.statenum);
		case PT_CLIENTCMD:
		case PT_CLIENT2CMD:
		case PT_CLIENTMIS:
		case PT_CLIENT2MIS:
		case PT_NODEKEEPALIVE:
		case PT_NODEKEEPALIVEMIS:
			fprintf(debugfile, "    tic %4u resendfrom %u\n",
				(UINT32)ExpandTics(netbuffer->u.clientpak.client_tic),
				(UINT32)ExpandTics (netbuffer->u.clientpak.resendfrom));
			break;
		case PT_TEXTCMD:
		case PT_TEXTCMD2:
			fprintf(debugfile, "    length %d\n    ", netbuffer->u.textcmd[0]);
			fprintfstring((char *)netbuffer->u.textcmd+1, netbuffer->u.textcmd[0]);
			break;
		case PT_SERVERCFG:
			fprintf(debugfile, "    playermask %x playerslots %d clientnode %d serverplayer %d "
				"gametic %u gamestate %d gametype %d modifiedgame %d\n",
				(UINT32)LONG(netbuffer->u.servercfg.playerdetected),
				netbuffer->u.servercfg.totalslotnum, netbuffer->u.servercfg.clientnode,
				netbuffer->u.servercfg.serverplayer, (UINT32)LONG(netbuffer->u.servercfg.gametic),
				netbuffer->u.servercfg.gamestate, netbuffer->u.servercfg.gametype,
				netbuffer->u.servercfg.modifiedgame);
			break;
		case PT_SERVERINFO:
			fprintf(debugfile, "    '%s' player %d/%d, map %s, filenum %d, time %u \n",
				netbuffer->u.serverinfo.servername, netbuffer->u.serverinfo.numberofplayer,
				netbuffer->u.serverinfo.maxplayer, netbuffer->u.serverinfo.mapname,
				netbuffer->u.serverinfo.fileneedednum,
				(UINT32)LONG(netbuffer->u.serverinfo.time));
			fprintfstring((char *)netbuffer->u.serverinfo.fileneeded,
				(UINT8)((UINT8 *)netbuffer + doomcom->datalength
				- (UINT8 *)netbuffer->u.serverinfo.fileneeded));
			break;
		case PT_SERVERREFUSE:
			fprintf(debugfile, "    reason %s\n", netbuffer->u.serverrefuse.reason);
			break;
		case PT_FILEFRAGMENT:
			fprintf(debugfile, "    fileid %d datasize %d position %u\n",
				netbuffer->u.filetxpak.fileid, (UINT16)SHORT(netbuffer->u.filetxpak.size),
				(UINT32)LONG(netbuffer->u.filetxpak.position));
			break;
		case PT_REQUESTFILE:
		default: // write as a raw packet
			fprintfstring((char *)netbuffer->u.textcmd,
				(UINT8)((UINT8 *)netbuffer + doomcom->datalength - (UINT8 *)netbuffer->u.textcmd));
			break;
	}
}
#endif

//
// HSendPacket
//
boolean HSendPacket(INT32 node, boolean reliable, UINT8 acknum, size_t packetlength)
{
	doomcom->datalength = (INT16)(packetlength + BASEPACKETSIZE);
	if (node == 0) // packet is to go back to us
	{
		if ((rebound_head+1) % MAXREBOUND == rebound_tail)
		{
#ifdef PARANOIA
			CONS_Printf("No more rebound buf\n");
#endif
			return false;
		}
		M_Memcpy(&reboundstore[rebound_head], netbuffer,
			doomcom->datalength);
		reboundsize[rebound_head] = doomcom->datalength;
		rebound_head = (rebound_head+1) % MAXREBOUND;
#ifdef DEBUGFILE
		if (debugfile)
		{
			doomcom->remotenode = (INT16)node;
			DebugPrintpacket("SENDLOCAL");
		}
#endif
		return true;
	}

	if (!netgame)
		I_Error("Tried to transmit to another node");

	// do this before GetFreeAcknum because this function backup
	// the current packet
	doomcom->remotenode = (INT16)node;
	if (doomcom->datalength <= 0)
	{
		DEBFILE("HSendPacket: nothing to send\n");
#ifdef DEBUGFILE
		if (debugfile)
			DebugPrintpacket("TRISEND");
#endif
		return false;
	}

	if (node < MAXNETNODES) // can be a broadcast
		netbuffer->ackreturn = GetAcktosend(node);
	else
		netbuffer->ackreturn = 0;
	if (reliable)
	{
		if (I_NetCanSend && !I_NetCanSend())
		{
			if (netbuffer->packettype < PT_CANFAIL)
				GetFreeAcknum(&netbuffer->ack, true);

			DEBFILE("HSendPacket: Out of bandwidth\n");
			return false;
		}
		else if (!GetFreeAcknum(&netbuffer->ack, false))
			return false;
	}
	else
		netbuffer->ack = acknum;

	netbuffer->checksum = NetbufferChecksum();
	sendbytes += packetheaderlength + doomcom->datalength; // for stat

	// simulate internet :)
	if (true || rand()<(INT32)RAND_MAX/5)
	{
#ifdef DEBUGFILE
		if (debugfile)
			DebugPrintpacket("SEND");
#endif
		I_NetSend();
	}
#ifdef DEBUGFILE
	else if (debugfile)
		DebugPrintpacket("NOTSEND");
#endif
	return true;
}

//
// HGetPacket
// Returns false if no packet is waiting
// Check Datalength and checksum
//
boolean HGetPacket(void)
{
	// get a packet from self
	if (rebound_tail != rebound_head)
	{
		M_Memcpy(netbuffer, &reboundstore[rebound_tail], reboundsize[rebound_tail]);
		doomcom->datalength = reboundsize[rebound_tail];
		if (netbuffer->packettype == PT_NODETIMEOUT)
			doomcom->remotenode = netbuffer->u.textcmd[0];
		else
			doomcom->remotenode = 0;

		rebound_tail = (rebound_tail+1) % MAXREBOUND;
#ifdef DEBUGFILE
		if (debugfile)
			DebugPrintpacket("GETLOCAL");
#endif
		return true;
	}

	if (!netgame)
		return false;

	while(true)
	{
		I_NetGet();

		if (doomcom->remotenode == -1)
			return false;

		getbytes += packetheaderlength + doomcom->datalength; // for stat

		if (doomcom->remotenode >= MAXNETNODES)
		{
			DEBFILE(va("receive packet from node %d !\n", doomcom->remotenode));
			continue;
		}

		nodes[doomcom->remotenode].lasttimepacketreceived = I_GetTime();

		if (netbuffer->checksum != NetbufferChecksum())
		{
			DEBFILE("Bad packet checksum\n");
			if (I_Shun)
				I_Shun(doomcom->remotenode);
			Net_CloseConnection(doomcom->remotenode);
			continue;
		}

#ifdef DEBUGFILE
		if (debugfile)
			DebugPrintpacket("GET");
#endif

		// proceed the ack and ackreturn field
		if (!Processackpak())
			continue; // discarded (duplicated)

		// a packet with just ackreturn
		if (netbuffer->packettype == PT_NOTHING)
		{
			GotAcks();
			continue;
		}
	break;
	}

	return true;
}

static void Internal_Get(void)
{
	doomcom->remotenode = -1;
}

FUNCNORETURN static ATTRNORETURN void Internal_Send(void)
{
	I_Error("Send without netgame\n");
}

static void Internal_FreeNodenum(INT32 nodenum)
{
	(void)nodenum;
}

void D_SetDoomcom(void)
{
	if (doomcom) return;
	doomcom = Z_Calloc(sizeof (doomcom_t), PU_STATIC, NULL);
	doomcom->id = DOOMCOM_ID;
	doomcom->numslots = doomcom->numnodes = 1;
	doomcom->gametype = 0;
	doomcom->consoleplayer = 0;
	doomcom->extratics = 0;
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
boolean D_CheckNetGame(void)
{
	boolean ret = false;

	InitAck();
	rebound_tail = rebound_head = 0;

	statstarttic = I_GetTime();

	I_NetGet = Internal_Get;
	I_NetSend = Internal_Send;
	I_NetCanSend = NULL;
	I_NetCloseSocket = NULL;
	I_NetFreeNodenum = Internal_FreeNodenum;
	I_NetMakeNode = NULL;

	hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
	net_bandwidth = 30000;
	// I_InitNetwork sets doomcom and netgame
	// check and initialize the network driver
	multiplayer = false;

	// only dos version with external driver will return true
	netgame = I_InitNetwork();
	if (!netgame && !I_NetOpenSocket)
	{
		D_SetDoomcom();
		netgame = I_InitTcpNetwork();
	}

	if (netgame)
		ret = true;
	if (!server && netgame)
		netgame = false;
	server = true; // WTF? server always true???
		// no! The deault mode is server. Client is set elsewhere
		// when the client executes connect command.
	doomcom->ticdup = 1;

	if (M_CheckParm("-extratic"))
	{
		if (M_IsNextParm())
			doomcom->extratics = (INT16)atoi(M_GetNextParm());
		else
			doomcom->extratics = 1;
		CONS_Printf(text[SET_EXTRATICS], doomcom->extratics);
	}

	if (M_CheckParm("-bandwidth"))
	{
		if (M_IsNextParm())
		{
			net_bandwidth = atoi(M_GetNextParm());
			if (net_bandwidth < 1000)
				net_bandwidth = 1000;
			if (net_bandwidth > 100000)
				hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
			CONS_Printf(text[SET_BANDWIDTH], net_bandwidth);
		}
		else
			I_Error("usage: -bandwidth <byte_per_sec>");
	}

	software_MAXPACKETLENGTH = hardware_MAXPACKETLENGTH;
	if (M_CheckParm("-packetsize"))
	{
		if (M_IsNextParm())
		{
			INT32 p = atoi(M_GetNextParm());
			if (p < 75)
				p = 75;
			if (p > hardware_MAXPACKETLENGTH)
				p = hardware_MAXPACKETLENGTH;
			software_MAXPACKETLENGTH = (UINT16)p;
		}
		else
			I_Error("usage: -packetsize <bytes_per_packet>");
	}

	if (netgame)
		multiplayer = true;

	if (doomcom->id != DOOMCOM_ID)
		I_Error("Doomcom buffer invalid!");
	if (doomcom->numnodes > MAXNETNODES)
		I_Error("Too many nodes (%d), max:%d", doomcom->numnodes, MAXNETNODES);

	netbuffer = (doomdata_t *)(void *)&doomcom->data;

#ifdef DEBUGFILE
#ifdef _arch_dreamcast
	//debugfile = stderr;
	if (debugfile)
			CONS_Printf("debug output to: strerr\n");
#else
	if (M_CheckParm("-debugfile"))
	{
		char filename[20];
		INT32 k = doomcom->consoleplayer - 1;
		if (M_IsNextParm())
			k = atoi(M_GetNextParm()) - 1;
		while (!debugfile && k < MAXPLAYERS)
		{
			k++;
			sprintf(filename, "debug%d.txt", k);
			debugfile = fopen(filename, "w");
		}
		if (debugfile)
			CONS_Printf(text[DEBUG_OUTPUT], filename);
		else
			CONS_Printf(text[NODEBUG_OUTPUT], filename);
	}
#endif
#endif

	D_ClientServerInit();

	return ret;
}

void Command_Ping_f(void)
{
#ifndef NEWPING
	if(server)
	{
#endif
		INT32 i;
		for (i = 0; i < MAXPLAYERS;i++)
		{
#ifndef NEWPING
			const INT32 node = playernode[i];
			if (playeringame[i] && node != 0)
				CONS_Printf(text[CMD_PING], i, player_names[i],
				GetLag(node), G_TicsToMilliseconds(GetLag(node)));
#else
			if (playeringame[i] && i != 0)
				CONS_Printf(text[CMD_PING], i, player_names[i], playerpingtable[i]);
#endif
		}
#ifndef NEWPING
	}
	else CONS_Printf("%s", text[YOUARENOTTHESERVER]);
#endif
}

void D_CloseConnection(void)
{
	INT32 i;

	if (netgame)
	{
		// wait the ackreturn with timout of 5 Sec
		Net_WaitAllAckReceived(5);

		// close all connection
		for (i = 0; i < MAXNETNODES; i++)
			Net_CloseConnection(i|FORCECLOSE);

		InitAck();

		if (I_NetCloseSocket)
			I_NetCloseSocket();

		I_NetGet = Internal_Get;
		I_NetSend = Internal_Send;
		I_NetCanSend = NULL;
		I_NetCloseSocket = NULL;
		I_NetFreeNodenum = Internal_FreeNodenum;
		I_NetMakeNode = NULL;
		netgame = false;
		addedtogame = false;
	}
}
