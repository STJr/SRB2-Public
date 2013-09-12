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
#include "d_player.h"

// more precise version number to compare in network
#define SUBVERSION 7

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
	PT_PLAYERINFO,    // Send information for players in game (gamespy).
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
	PT_CONSISTENCY,   // Packet sent to resync players.
#ifdef NEWPING
	PT_PING,          // Packet sent to tell clients the other client's latency to server.
#endif
	NUMPACKETTYPE
} packettype_t;

#if defined(_MSC_VER)
#pragma pack(1)
#endif

// client to server packet
typedef struct
{
	UINT8 client_tic;
	UINT8 resendfrom;
	INT16 consistancy;
	ticcmd_t cmd;
} ATTRPACK clientcmd_pak;

// splitscreen packet
// WARNING: must have the same format of clientcmd_pak, for more easy use
typedef struct
{
	UINT8 client_tic;
	UINT8 resendfrom;
	INT16 consistancy;
	ticcmd_t cmd, cmd2;
} ATTRPACK client2cmd_pak;

#ifdef _MSC_VER
#pragma warning(disable :  4200)
#endif

// Server to client packet
// this packet is too large
typedef struct
{
	UINT8 starttic;
	UINT8 numtics;
	UINT8 numslots; // "Slots filled": Highest player number in use plus one.
	UINT8 padding1[1];
	ticcmd_t cmds[45]; // normally [BACKUPTIC][MAXPLAYERS] but too large
	UINT8 padding2[0];
} ATTRPACK servertics_pak;

typedef struct
{
	UINT8 version; // different versions don't work
	UINT8 padding1[3];
	UINT8 subversion; // contains build version
	UINT8 padding2[3];

	// server launch stuffs
	UINT8 serverplayer;
	UINT8 totalslotnum; // "Slots": highest player number in use plus one.
	UINT8 padding3[2];

	tic_t gametic;
	UINT8 clientnode;
	UINT8 gamestate;
	UINT8 padding4[2];

	UINT32 playerdetected; // playeringame vector in bit field
	UINT8 gametype;
	UINT8 modifiedgame;
	SINT8 adminplayer; // needs to be signed
	UINT8 netcvarstates[0];
#ifdef __GNUC__
	UINT8 padding5[1];
#endif
} ATTRPACK serverconfig_pak;

typedef struct {
	UINT8 fileid;
	UINT8 padding1[3];
	UINT32 position;
	UINT16 size;
	UINT8 data[0]; // size is variable using hardare_MAXPACKETLENGTH
#ifdef __GNUC__
	UINT8 padding2[2];
#endif
} ATTRPACK filetx_pak;

#ifdef _MSC_VER
#pragma warning(default : 4200)
#endif

typedef struct
{
	UINT8 version; // different versions don't work
	UINT8 padding1[3];
	UINT8 subversion; // contains build version
	UINT8 padding2[3];
	UINT8 localplayers;
	UINT8 mode;
	UINT8 padding3[2];
} ATTRPACK clientconfig_pak;

#define MAXSERVERNAME 32
// this packet is too large
typedef struct
{
	UINT8 version;
	UINT8 subversion;
	UINT8 numberofplayer;
	UINT8 maxplayer;
	UINT8 gametype;
	UINT8 modifiedgame;
	UINT8 cheatsenabled;
	UINT8 isdedicated;
	UINT8 fileneedednum;
	SINT8 adminplayer;
	tic_t time;
	tic_t leveltime;
	char servername[MAXSERVERNAME];
	char mapname[8];
	char maptitle[33];
	unsigned char mapmd5[16];
	UINT8 actnum;
	UINT8 iszone;
	UINT8 fileneeded[915]; // is filled with writexxx (byteptr.h)
} ATTRPACK serverinfo_pak;

typedef struct
{
	char reason[255];
} ATTRPACK serverrefuse_pak;

typedef struct
{
	UINT8 version;
	UINT8 padding1[3];
	tic_t time; // used for ping evaluation
} ATTRPACK askinfo_pak;

typedef struct
{
	char clientaddr[22];
	UINT8 padding1[2];
	tic_t time; // used for ping evaluation
} ATTRPACK msaskinfo_pak;

typedef struct
{
	UINT8 randomseed;

	//ctf flag stuff
	UINT8 rflagloose;
	UINT8 bflagloose;
	INT32 rfuse;
	INT32 bfuse;
	INT32 rflags2;
	INT32 bflags2;
	INT16 rflagx;
	INT16 rflagy;
	INT16 rflagz;
	INT16 bflagx;
	INT16 bflagy;
	INT16 bflagz;

	//player stuff
	UINT8 playernum;

	UINT8 playerstate; //playerstate_t
	ticcmd_t cmd;
	fixed_t viewz;
	fixed_t viewheight;
	fixed_t deltaviewheight;
	fixed_t bob;
	angle_t aiming;
	angle_t awayviewaiming;
	INT32 phealth;
	INT32 currentweapon;
	INT32 ringweapons;
	fixed_t tossstrength;
	INT32 powers[NUMPOWERS];
	UINT32 pflags; //pflags_t
	INT32 bonuscount;
	INT32 skincolor;
	INT32 skin;
	UINT32 score;
	INT32 dashspeed;
	INT32 normalspeed; //INT32
	INT32 runspeed; //INT32
	INT32 thrustfactor; //INT32
	INT32 accelstart; //INT32
	INT32 acceleration; //INT32
	INT32 charability; //INT32
	INT32 charability2; //INT32
	UINT32 charflags;
	UINT32 thokitem; //mobjtype_t
	UINT32 spinitem; //mobjtype_t
	INT32 actionspd; //INT32
	INT32 mindash; //INT32
	INT32 maxdash; //INT32
	INT32 jumpfactor; //INT32
#ifndef TRANSFIX
	INT32 starttranscolor; //INT32
#endif
	INT32 prefcolor; //INT32
	INT32 lives;
	INT32 continues;
	INT32 xtralife;
	INT32 speed;
	INT32 jumping;
	UINT8 secondjump;
	INT32 fly1;
	UINT32 scoreadd;
	tic_t glidetime;
	INT32 climbing;
	INT32 deadtimer;
	INT32 splish;
	tic_t exiting;
	INT32 blackow;
	UINT8 homing;
	fixed_t cmomx;
	fixed_t cmomy;
	fixed_t rmomx;
	fixed_t rmomy;
	INT32 numboxes;
	INT32 totalring;
	tic_t realtime;
	UINT32 racescore;
	UINT32 laps;
	INT32 ctfteam;
	UINT16 gotflag;
	INT32 dbginfo;
	INT32 emeraldhunt;
	INT32 weapondelay;
	INT32 tossdelay;
	INT32 shielddelay;
	tic_t taunttimer;
	INT32 starpostx;
	INT32 starposty;
	INT32 starpostz;
	INT32 starpostnum;
	tic_t starposttime;
	angle_t starpostangle;
	UINT32 starpostbit;
	angle_t angle_pos;
	angle_t old_angle_pos;
	tic_t bumpertime;
	INT32 flyangle;
	tic_t drilltimer;
	INT32 linkcount;
	tic_t linktimer;
	INT32 anotherflyangle;
	tic_t nightstime;
	INT32 drillmeter;
	UINT8 drilldelay;
	UINT8 bonustime;
	UINT8 mare;
	INT16 lastsidehit, lastlinehit;
	INT32 losscount;
	INT32 onconveyor;
	UINT8 spectator; //boolean
	tic_t jointime;

	//player->mo stuff
	UINT8 hasmo; //boolean

	angle_t angle;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	INT32 friction;
	INT32 movefactor;

	INT32 tics;
	statenum_t statenum;
	INT32 flags;
	INT32 flags2;
	UINT32 eflags;
	INT32 health;

	fixed_t radius;
	fixed_t height;
	UINT16 scale;
	UINT16 destscale;
	UINT8 scalespeed;
} ATTRPACK cons_pak;

// Shorter player information for external use.
typedef struct
{
	UINT8 node;
	char name[MAXPLAYERNAME+1];
	UINT32 address; // sending another string would run us up against MAXPACKETLENGTH
	UINT8 team;
	UINT8 skin;
	UINT8 data; // Color is first four bits, hasflag, isit and issuper have one bit each, the last is unused.
	UINT32 score;
	UINT16 timeinserver; // In seconds.
} ATTRPACK plrinfo;

//
// Network packet data.
//
typedef struct
{
	UINT32 checksum;
	UINT8 ack; // if not null the node asks for acknowledgement, the receiver must resend the ack
	UINT8 ackreturn; // the return of the ack number

	UINT8 packettype;
	UINT8 reserved; // padding
	union
	{
		clientcmd_pak clientpak;    //      144 bytes
		client2cmd_pak client2pak;  //      200 bytes
		servertics_pak serverpak;   //   132496 bytes
		serverconfig_pak servercfg; //      784 bytes
		UINT8 textcmd[MAXTEXTCMD+1]; //   66049 bytes
		filetx_pak filetxpak;       //      144 bytes
		clientconfig_pak clientcfg; //      144 bytes
		serverinfo_pak serverinfo;  //     1024 bytes
		serverrefuse_pak serverrefuse; // 65025 bytes
		askinfo_pak askinfo;        //       64 bytes
		msaskinfo_pak msaskinfo;	//       24 bytes
		cons_pak consistency;       //      544 bytes
		plrinfo playerinfo[MAXPLAYERS]; // 1024 bytes
#ifdef NEWPING
		UINT32 pingtable[MAXPLAYERS];//128 bytes
#endif
	} u; // this is needed to pack diff packet types data together
} ATTRPACK doomdata_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

#define MAXSERVERLIST 64 // depends only on the display
typedef struct
{
	SINT8 node;
	serverinfo_pak info;
} serverelem_t;

extern serverelem_t serverlist[MAXSERVERLIST];
extern UINT32 serverlistcount;
extern INT32 mapchangepending;

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
#ifdef NEWPING
#define KICK_MSG_PING_HIGH   6
#endif
#define KICK_MSG_CUSTOM_KICK 7
#define KICK_MSG_CUSTOM_BAN  8

extern boolean server;
extern boolean dedicated; // for dedicated server
extern UINT16 software_MAXPACKETLENGTH;
extern boolean acceptnewnode;
extern SINT8 servernode;

void Command_Ping_f(void);
extern tic_t connectiontimeout;
#ifdef NEWPING
extern UINT16 pingmeasurecount;
extern UINT32 realpingtable[MAXPLAYERS];
extern UINT32 playerpingtable[MAXPLAYERS];
#endif

extern consvar_t cv_joinnextround, cv_allownewplayer, cv_maxplayers, cv_consfailprotect, cv_blamecfail, cv_maxsend;

// used in d_net, the only dependence
tic_t ExpandTics(INT32 low);
void D_ClientServerInit(void);

// initialise the other field
void RegisterNetXCmd(netxcmd_t id, void (*cmd_f)(UINT8 **p, INT32 playernum));
void SendNetXCmd(netxcmd_t id, const void *param, size_t nparam);
void SendNetXCmd2(netxcmd_t id, const void *param, size_t nparam); // splitsreen player

// Create any new ticcmds and broadcast to other players.
void NetUpdate(void);

void SV_StartSinglePlayerServer(void);
boolean SV_SpawnServer(void);
void SV_SpawnPlayer(INT32 playernum, INT32 x, INT32 y, angle_t angle);
void SV_StopServer(void);
void SV_ResetServer(void);
void CL_AddSplitscreenPlayer(void);
void CL_RemoveSplitscreenPlayer(void);
void CL_Reset(void);
void CL_ClearPlayer(INT32 playernum);
void CL_UpdateServerList(boolean internetsearch, INT32 room);
// is there a game running
boolean Playing(void);

// Broadcasts special packets to other players
//  to notify of game exit
void D_QuitNetGame(void);

//? how many ticks to run?
void TryRunTics(tic_t realtic);

// extra data for lmps
boolean AddLmpExtradata(UINT8 **demo_p, INT32 playernum);
void ReadLmpExtraData(UINT8 **demo_pointer, INT32 playernum);

#ifndef NONET
// translate a playername in a player number return -1 if not found and
// print a error message in the console
SINT8 nametonum(const char *name);
#endif

extern char adminpassword[9], motd[254];
extern UINT8 playernode[MAXPLAYERS];
extern UINT8 consfailcount[MAXPLAYERS];

INT32 D_NumPlayers(void);
void D_ResetTiccmds(void);

tic_t GetLag(INT32 node);
UINT8 GetFreeXCmdSize(void);

#endif
