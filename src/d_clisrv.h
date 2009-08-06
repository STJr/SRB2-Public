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
/// \brief high level networking stuff

#ifndef __D_CLISRV__
#define __D_CLISRV__

#include "d_ticcmd.h"
#include "d_netcmd.h"
#include "tables.h"

// more precise version number to compare in network
#define SUBVERSION 004

// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.

// Networking and tick handling related.
#define BACKUPTICS 32
#define MAXTEXTCMD 256
//
// Packet structure
//
typedef enum
{
	PT_NOTHING,       // To send a nop through the network. ^_~
	PT_SERVERCFG,     // Server config used in start game
	                  // (must stay 1 for backwards compatibility).
	                  // This is a positive response to a CLIENTJOIN request.
	PT_CLIENTCMD,     // Ticcmd of the client.
	PT_CLIENTMIS,     // Same as above with but saying resend from.
	PT_CLIENT2CMD,    // 2 cmds in the packet for splitscreen.
	PT_CLIENT2MIS,    // Same as above with but saying resend from
	PT_NODEKEEPALIVE, // Same but without ticcmd and consistancy
	PT_NODEKEEPALIVEMIS,
	PT_SERVERTICS,    // All cmds for the tic.
	PT_SERVERREFUSE,  // Server refuses joiner (reason inside).
	PT_SERVERSHUTDOWN,
	PT_CLIENTQUIT,    // Client closes the connection.

	PT_ASKINFO,       // Anyone can ask info of the server.
	PT_SERVERINFO,    // Send game & server info (gamespy).
	PT_REQUESTFILE,   // Client requests a file transfer
	PT_ASKINFOVIAMS,  // Packet from the MS requesting info be sent to new client.
	                  // If this ID changes, update masterserver definition.

	PT_CANFAIL,       // This is kind of a priority. Anything bigger than CANFAIL
	                  // allows HSendPacket(,true,,) to return false.
	                  // In addition, this packet can't occupy all the available slots.

	PT_FILEFRAGMENT = PT_CANFAIL, // A part of a file.

	PT_TEXTCMD,       // Extra text commands from the client.
	PT_TEXTCMD2,      // Splitscreen text commands.
	PT_CLIENTJOIN,    // Client wants to join; used in start game.
	PT_NODETIMEOUT,   // Packet sent to self if the connection times out.
	NUMPACKETTYPE
} packettype_t;

#if defined(_MSC_VER)
#pragma pack(1)
#endif

// client to server packet
typedef struct
{
	byte client_tic;
	byte resendfrom;
	short consistancy;
	ticcmd_t cmd;
} ATTRPACK clientcmd_pak;

// splitscreen packet
// WARNING: must have the same format of clientcmd_pak, for more easy use
typedef struct
{
	byte client_tic;
	byte resendfrom;
	short consistancy;
	ticcmd_t cmd, cmd2;
} ATTRPACK client2cmd_pak;

#ifdef _MSC_VER
#pragma warning(disable :  4200)
#endif

// Server to client packet
// this packet is too large
typedef struct
{
	byte starttic;
	byte numtics;
	byte numslots; // "Slots filled": Highest player number in use plus one.
	byte padding1[1];
	ticcmd_t cmds[45]; // normally [BACKUPTIC][MAXPLAYERS] but too large
	byte padding2[0];
} ATTRPACK servertics_pak;

typedef struct
{
	byte version; // different versions don't work
	byte padding1[3];
	byte subversion; // contains build version
	byte padding2[3];

	// server launch stuffs
	byte serverplayer;
	byte totalslotnum; // "Slots": highest player number in use plus one.
	byte padding3[2];

	tic_t gametic;
	byte clientnode;
	byte gamestate;
	byte padding4[2];

	ULONG playerdetected; // playeringame vector in bit field
	byte gametype;
	byte modifiedgame;
	char adminplayer; // needs to be signed
	byte netcvarstates[0];
#ifdef __GNUC__
	byte padding5[1];
#endif
} ATTRPACK serverconfig_pak;

typedef struct {
	char fileid;
	byte padding1[3];
	ULONG position;
	USHORT size;
	byte data[0]; // size is variable using hardare_MAXPACKETLENGTH
#ifdef __GNUC__
	byte padding2[2];
#endif
} ATTRPACK filetx_pak;

#ifdef _MSC_VER
#pragma warning(default : 4200)
#endif

typedef struct
{
	byte version; // different versions don't work
	byte padding1[3];
	byte subversion; // contains build version
	byte padding2[3];
	byte localplayers;
	byte mode;
	byte padding3[2];
} ATTRPACK clientconfig_pak;

#define MAXSERVERNAME 32
// this packet is too large
typedef struct
{
	byte version;
	byte subversion;
	byte numberofplayer;
	byte maxplayer;
	byte gametype;
	byte modifiedgame;
	byte fileneedednum;
	char adminplayer; // needs to be signed
	tic_t time;
	char servername[MAXSERVERNAME];
	char mapname[8];
	byte fileneeded[936]; // is filled with writexxx (byteptr.h)
} ATTRPACK serverinfo_pak;

typedef struct
{
	char reason[255];
} ATTRPACK serverrefuse_pak;

typedef struct
{
	byte version;
	byte padding1[3];
	tic_t time; // used for ping evaluation
} ATTRPACK askinfo_pak;

typedef struct
{
	char clientaddr[22];
	byte padding1[2];
	tic_t time; // used for ping evaluation
} ATTRPACK msaskinfo_pak;

//
// Network packet data.
//
typedef struct
{
	unsigned checksum;
	byte ack; // if not null the node asks for acknowledgement, the receiver must resend the ack
	byte ackreturn; // the return of the ack number

	byte packettype;
	byte reserved; // padding
	union
	{
		clientcmd_pak clientpak;    //      144 bytes
		client2cmd_pak client2pak;  //      200 bytes
		servertics_pak serverpak;   //   132496 bytes
		serverconfig_pak servercfg; //      784 bytes
		byte textcmd[MAXTEXTCMD+1]; //    66049 bytes
		filetx_pak filetxpak;       //      144 bytes
		clientconfig_pak clientcfg; //      144 bytes
		serverinfo_pak serverinfo;  // 17338896 bytes
		serverrefuse_pak serverrefuse; // 65025 bytes
		askinfo_pak askinfo;        //       64 bytes
		msaskinfo_pak msaskinfo;	//       24 bytes
	} u; // this is needed to pack diff packet types data together
} ATTRPACK doomdata_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

#define MAXSERVERLIST 64 // depends only on the display
typedef struct
{
	signed char node;
	serverinfo_pak info;
} serverelem_t;

extern serverelem_t serverlist[MAXSERVERLIST];
extern unsigned int serverlistcount;
extern int mapchangepending;

// points inside doomcom
extern doomdata_t *netbuffer;

extern consvar_t cv_playdemospeed;

#define BASEPACKETSIZE ((size_t)&(((doomdata_t *)0)->u))
#define FILETXHEADER ((size_t)((filetx_pak *)0)->data)
#define BASESERVERTICSSIZE ((size_t)&(((doomdata_t *)0)->u.serverpak.cmds[0]))

#define KICK_MSG_GO_AWAY     1
#define KICK_MSG_CON_FAIL    2
#define KICK_MSG_PLAYER_QUIT 3
#define KICK_MSG_TIMEOUT     4
#define KICK_MSG_BANNED      5

extern boolean server;
extern boolean dedicated; // for dedicated server
extern USHORT software_MAXPACKETLENGTH;
extern boolean acceptnewnode;
extern signed char servernode;

void Command_Ping_f(void);
extern tic_t connectiontimeout;

extern consvar_t cv_joinnextround, cv_allownewplayer, cv_maxplayers, cv_consfailprotect, cv_blamecfail, cv_maxsend;

// used in d_net, the only dependence
tic_t ExpandTics(int low);
void D_ClientServerInit(void);

// initialise the other field
void RegisterNetXCmd(netxcmd_t id, void (*cmd_f)(byte **p, int playernum));
void SendNetXCmd(netxcmd_t id, const void *param, size_t nparam);
void SendNetXCmd2(netxcmd_t id, const void *param, size_t nparam); // splitsreen player

// Create any new ticcmds and broadcast to other players.
void NetUpdate(void);

void SV_StartSinglePlayerServer(void);
boolean SV_SpawnServer(void);
void SV_SpawnPlayer(int playernum, int x, int y, angle_t angle);
void SV_StopServer(void);
void SV_ResetServer(void);
void CL_AddSplitscreenPlayer(void);
void CL_RemoveSplitscreenPlayer(void);
void CL_Reset(void);
void CL_ClearPlayer(int playernum);
void CL_UpdateServerList(boolean internetsearch);
// is there a game running
boolean Playing(void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame(void);

//? how many ticks to run?
void TryRunTics(tic_t realtic);

// extra data for lmps
boolean AddLmpExtradata(byte **demo_p, int playernum);
void ReadLmpExtraData(byte **demo_pointer, int playernum);

// translate a playername in a player number return -1 if not found and
// print a error message in the console
char nametonum(const char *name);

extern char adminpassword[9], motd[256];
extern byte playernode[MAXPLAYERS];
extern byte consfailcount[MAXPLAYERS];

int D_NumPlayers(void);
void D_ResetTiccmds(void);

tic_t GetLag(int node);
byte GetFreeXCmdSize(void);

#endif
