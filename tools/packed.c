/*
 *      packed.c
 *
 *      Copyright 2007 Alam Arias <Alam.GBC@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <stdio.h>
#include <stdlib.h>

#define PACKED

#ifdef PACKED
#define ATTRPACK __attribute__ ((packed))
#else
#define ATTRPACK
#endif

#define USHORT unsigned short
#define ULONG unsigned long

typedef unsigned char byte;
typedef ULONG tic_t;

typedef int fixed_t;

#define MAXSERVERNAME 32
#define MAXTEXTCMD 256

typedef struct {
	signed char forwardmove; // *2048 for move
	signed char sidemove; // *2048 for move
	short angleturn; // <<16 for angle delta - SAVED AS A BYTE into demos
	signed short aiming; // mouse aiming, see G_BuildTicCmd
	byte buttons;
#ifdef PACKED
	byte dummy1[1];
#endif
} ATTRPACK ticcmd_t;

typedef struct {
	byte client_tic;
	byte resendfrom;
	short consistancy;
	ticcmd_t cmd;
} ATTRPACK clientcmd_pak;

typedef struct {
	byte client_tic;
	byte resendfrom;
	short consistancy;
	ticcmd_t cmd, cmd2;
} ATTRPACK client2cmd_pak;

typedef struct {
	byte starttic;
	byte numtics;
	byte numslots; // "Slots filled": Highest player number in use plus one.
#ifdef PACKED
	byte dummy1[1];
#endif
	ticcmd_t cmds[45]; // normally [BACKUPTIC][MAXPLAYERS] but too large
#ifdef PACKED
	byte dummy2[0];
#endif
} ATTRPACK servertics_pak;

typedef struct {
	byte version; // different versions don't work
#ifdef PACKED
	byte dummy1[3];
#endif
	ULONG subversion; // contains build version

	// server launch stuffs
	byte serverplayer;
	byte totalslotnum; // "Slots": highest player number in use plus one.
#ifdef PACKED
	byte dummy2[2];
#endif
	tic_t gametic;
	byte clientnode;
	byte gamestate;

#ifdef PACKED
	byte dummy3[2];
#endif
	ULONG playerdetected; // playeringame vector in bit field
	byte gametype;
	byte modifiedgame;
	char adminplayer; // needs to be signed
	byte netcvarstates[0];
#ifdef PACKED
	byte dummy4[1];
#endif
} ATTRPACK serverconfig_pak;

typedef struct {
	char fileid;
#ifdef PACKED
	byte dummy1[3];
#endif
	ULONG position;
	USHORT size;
	byte data[0]; // size is variable using hardare_MAXPACKETLENGTH
#ifdef PACKED
	byte dummy2[2];
#endif
} ATTRPACK filetx_pak;

typedef struct {
	byte version; // different versions don't work
#ifdef PACKED
	byte dummy1[3];
#endif
	ULONG subversion; // contains build version
	byte localplayers;
	byte mode;
#ifdef PACKED
	byte dummy2[2];
#endif
} ATTRPACK clientconfig_pak;

typedef struct {
	byte version;
#ifdef PACKED
	byte dummy1[3];
#endif
	ULONG subversion;
	byte numberofplayer;
	byte maxplayer;
	byte gametype;
	byte modifiedgame;
	char adminplayer; // needs to be signed
#ifdef PACKED
	byte dummy2[3];
#endif
	tic_t time;
	fixed_t load; // unused for the moment
	char mapname[8];
	char servername[MAXSERVERNAME];
	byte fileneedednum;
	byte fileneeded[4096]; // is filled with writexxx (byteptr.h)
#ifdef PACKED
	byte dummy3[3];
#endif
} ATTRPACK serverinfo_pak;

typedef struct {
	char reason[255];
} ATTRPACK serverrefuse_pak;

typedef struct {
	byte version;
#ifdef PACKED
	byte dummy1[3];
#endif
	tic_t time; // used for ping evaluation
} ATTRPACK askinfo_pak;

typedef struct {
	ULONG checksum;
	byte ack; // if not null the node asks for acknowledgement, the receiver must resend the ack
	byte ackreturn; // the return of the ack number

	byte packettype;
	byte reserved; // padding
	union
	{
		clientcmd_pak clientpak;
		client2cmd_pak client2pak;
		servertics_pak serverpak;
		serverconfig_pak servercfg;
		byte textcmd[MAXTEXTCMD+1];
		filetx_pak filetxpak;
		clientconfig_pak clientcfg;
		serverinfo_pak serverinfo;
		serverrefuse_pak serverrefuse;
		askinfo_pak askinfo;
	} u; // this is needed
} ATTRPACK doomdata_t;

static void packcheck(const char *str, const void *start, const void *end, const size_t wantoffset)
{
	const size_t offset = (byte*)end-(byte*)start;
	printf("%s %zu = %zu\n", str, offset, wantoffset);
	if (offset && wantoffset && offset != wantoffset) exit(-1);
}

doomdata_t doombuf;

int main(int argc, char** argv)
{
	size_t i;
	packcheck("checksum    ", &doombuf, &doombuf.checksum,      0);
	packcheck("ack         ", &doombuf, &doombuf.ack,           4);
	packcheck("ackreturn   ", &doombuf, &doombuf.ackreturn,     5);
	packcheck("packettype  ", &doombuf, &doombuf.packettype,    6);
	packcheck("reserved    ", &doombuf, &doombuf.reserved,      7);

	packcheck("clientpak   ", &doombuf.u.clientpak,
	 &doombuf.u.clientpak+sizeof(doombuf.u.clientpak),         144); {
		packcheck(" client_tic ", &doombuf.u.clientpak, &doombuf.u.clientpak.client_tic,  0);
		packcheck(" resendfrom ", &doombuf.u.clientpak, &doombuf.u.clientpak.resendfrom,  1);
		packcheck(" consistancy", &doombuf.u.clientpak, &doombuf.u.clientpak.consistancy, 2);
		packcheck(" cmd        ", &doombuf.u.clientpak, &doombuf.u.clientpak.cmd,         4); //ticmd_t
		packcheck("  forwardmove", &doombuf.u.clientpak.cmd, &doombuf.u.clientpak.cmd.forwardmove, 0);
		packcheck("  sidemove   ", &doombuf.u.clientpak.cmd, &doombuf.u.clientpak.cmd.sidemove,    1);
		packcheck("  angleturn  ", &doombuf.u.clientpak.cmd, &doombuf.u.clientpak.cmd.angleturn,   2);
		packcheck("  aiming     ", &doombuf.u.clientpak.cmd, &doombuf.u.clientpak.cmd.aiming,      4);
		packcheck("  buttons    ", &doombuf.u.clientpak.cmd, &doombuf.u.clientpak.cmd.buttons,     6);
	}
	packcheck("client2pak  ", &doombuf.u.client2pak,
	 &doombuf.u.client2pak+sizeof(doombuf.u.client2pak),       400); { // no todo
		packcheck(" client_tic ", &doombuf.u.client2pak, &doombuf.u.client2pak.client_tic,  0);
		packcheck(" resendfrom ", &doombuf.u.client2pak, &doombuf.u.client2pak.resendfrom,  1);
		packcheck(" consistancy", &doombuf.u.client2pak, &doombuf.u.client2pak.consistancy, 2);
		packcheck(" cmd        ", &doombuf.u.client2pak, &doombuf.u.client2pak.cmd,         4); //ticmd_t
		packcheck(" cmd2       ", &doombuf.u.client2pak, &doombuf.u.client2pak.cmd2,       12); //ticmd_t
	}
	packcheck("serverpak   ", &doombuf.u.serverpak,
	 &doombuf.u.serverpak+sizeof(doombuf.u.serverpak),      132496); {
		packcheck(" starttic", &doombuf.u.serverpak, &doombuf.u.serverpak.starttic, 0);
		packcheck(" numtics ", &doombuf.u.serverpak, &doombuf.u.serverpak.numtics,  1);
		packcheck(" numslots", &doombuf.u.serverpak, &doombuf.u.serverpak.numslots, 2);
		packcheck(" cmds    ", &doombuf.u.serverpak, &doombuf.u.serverpak.cmds,     4); //ticcmd_t[45]
		for (i=0; i < 45; i++)
		{
			//packcheck(" data", &doombuf.u.serverpak.cmds, &doombuf.u.serverpak.cmds[i], i*8);
		}
	}
	packcheck("servercfg   ", &doombuf.u.servercfg,
	 &doombuf.u.servercfg+sizeof(doombuf.u.servercfg),         784); { // done
		packcheck(" version       ", &doombuf.u.servercfg, &doombuf.u.servercfg.version,        0);
		packcheck(" subversion    ", &doombuf.u.servercfg, &doombuf.u.servercfg.subversion,     4);
		packcheck(" serverplayer  ", &doombuf.u.servercfg, &doombuf.u.servercfg.serverplayer,   8);
		packcheck(" totalslotnum  ", &doombuf.u.servercfg, &doombuf.u.servercfg.totalslotnum,   9);
		packcheck(" gametic       ", &doombuf.u.servercfg, &doombuf.u.servercfg.gametic,       12);
		packcheck(" clientnode    ", &doombuf.u.servercfg, &doombuf.u.servercfg.clientnode,    16);
		packcheck(" gamestate     ", &doombuf.u.servercfg, &doombuf.u.servercfg.gamestate,     17);
		packcheck(" playerdetected", &doombuf.u.servercfg, &doombuf.u.servercfg.playerdetected, 20);
		packcheck(" gametype      ", &doombuf.u.servercfg, &doombuf.u.servercfg.gametype,      24);
		packcheck(" modifiedgame  ", &doombuf.u.servercfg, &doombuf.u.servercfg.modifiedgame,  25);
		packcheck(" adminplayer   ", &doombuf.u.servercfg, &doombuf.u.servercfg.adminplayer,   26);
		packcheck(" netcvarstates ", &doombuf.u.servercfg, &doombuf.u.servercfg.netcvarstates, 27);
	}
	packcheck("textcmd     ", &doombuf.u.textcmd,
	&doombuf.u.textcmd+sizeof(doombuf.u.textcmd),            66049); { // packed?
		for (i=0; i < MAXTEXTCMD+1; i++)
		{
			//packcheck(" data", &doombuf.u.textcmd, &doombuf.u.textcmd[i], i); //byte textcmd[MAXTEXTCMD+1];
		}
	}
	packcheck("filetxpak   ", &doombuf.u.filetxpak,
	 &doombuf.u.filetxpak+sizeof(doombuf.u.filetxpak),         144); { // done
		packcheck(" fileid  ", &doombuf.u.filetxpak, &doombuf.u.filetxpak.fileid,   0);
		packcheck(" position", &doombuf.u.filetxpak, &doombuf.u.filetxpak.position, 4);
		packcheck(" size    ", &doombuf.u.filetxpak, &doombuf.u.filetxpak.size,     8);
		packcheck(" data    ", &doombuf.u.filetxpak, &doombuf.u.filetxpak.data,    10);
	}
	packcheck("clientcfg   ", &doombuf.u.clientcfg,
	 &doombuf.u.clientcfg+sizeof(doombuf.u.clientcfg),         144); { // done
		packcheck(" version     ", &doombuf.u.clientcfg, &doombuf.u.clientcfg.version,      0);
		packcheck(" sunversion  ", &doombuf.u.clientcfg, &doombuf.u.clientcfg.subversion,   4);
		packcheck(" localplayers", &doombuf.u.clientcfg, &doombuf.u.clientcfg.localplayers, 8);
		packcheck(" mode        ", &doombuf.u.clientcfg, &doombuf.u.clientcfg.mode,         9);
	}
	packcheck("serverinfo  ", &doombuf.u.serverinfo,
	 &doombuf.u.serverinfo+sizeof(doombuf.u.serverinfo),  17338896); { // not done
		packcheck(" version       ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.version,        0);
		packcheck(" subversion    ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.subversion,     4);
		packcheck(" numberofplayer", &doombuf.u.serverinfo, &doombuf.u.serverinfo.numberofplayer, 8);
		packcheck(" maxplayer     ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.maxplayer,      9);
		packcheck(" gametype      ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.gametype,      10);
		packcheck(" modifiedgame  ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.modifiedgame,  11);
		packcheck(" adminplayer   ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.adminplayer,   12);
		packcheck(" time          ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.time,          16);
		packcheck(" load          ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.load,          20);
		packcheck(" mapname       ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.mapname,       24); //mapname[8[
		packcheck(" servername    ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.servername,    32); //server[MAXSEVERNAME]
		packcheck(" fileneedednum ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.fileneedednum, 64);
		packcheck(" fileneeded    ", &doombuf.u.serverinfo, &doombuf.u.serverinfo.fileneeded,    65); //fileneeded[0]
	}
	packcheck("serverrefuse", &doombuf.u.serverrefuse,
	 &doombuf.u.serverrefuse+sizeof(doombuf.u.serverrefuse), 65025); { // done
		size_t i;
		for (i=0; i < 255; i++)
		{
			//packcheck(" data", &doombuf.u.serverrefuse, &doombuf.u.serverrefuse.reason[i], i);
		}
	}
	packcheck("askinfo     ", &doombuf.u.askinfo,
	 &doombuf.u.askinfo+sizeof(doombuf.u.askinfo),              64); { // done
		packcheck(" version", &doombuf.u.askinfo, &doombuf.u.askinfo.version, 0);
		packcheck(" time   ", &doombuf.u.askinfo, &doombuf.u.askinfo.time,    4);
	}
	return 0;
}
