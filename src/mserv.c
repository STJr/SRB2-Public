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
/// \brief Commands used for communicate with the master server

#ifdef __GNUC__
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#endif

#if !defined (UNDER_CE)
#include <time.h>
#endif

#if (defined (NOMD5) || defined (NOMSERV)) && !defined (NONET)
#define NONET
#endif

#ifndef NONET
#if (defined (_WIN32) || defined (_WIN32_WCE)) && !defined (_XBOX)
#define RPC_NO_WINDOWS_H
#include <winsock.h>     // socket(),...
#else
#ifdef __OS2__
#include <sys/types.h>
#endif // __OS2__

#ifdef HAVE_LWIP
#include <lwip/inet.h>
#include <kos/net.h>
#include <lwip/lwip.h>
#define ioctl lwip_ioctl
#else
#include <arpa/inet.h>
#ifdef __APPLE_CC__
#define _BSD_SOCKLEN_T_
#endif
#include <sys/socket.h> // socket(),...
#include <netinet/in.h> // sockaddr_in
#ifndef _arch_dreamcast
#include <netdb.h> // gethostbyname(),...
#include <sys/ioctl.h>
#endif
#endif

#ifdef _arch_dreamcast
#include "sdl/SRB2DC/dchelp.h"
#endif

#include <sys/time.h> // timeval,... (TIMEOUT)
#include <errno.h>
#endif // _WIN32/_WIN32_WCE

#ifdef __OS2__
#include <errno.h>
#endif // __OS2__
#endif // !NONET

#include "doomstat.h"
#include "doomdef.h"
#include "command.h"
#include "i_net.h"
#include "console.h"
#include "mserv.h"
#include "d_net.h"
#include "i_tcp.h"
#include "i_system.h"
#include "byteptr.h"
#include "m_menu.h"
#include "m_argv.h" // Alam is going to kill me <3

#ifdef _WIN32_WCE
#include "sdl/SRB2CE/cehelp.h"
#endif

// ================================ DEFINITIONS ===============================

#define PACKET_SIZE 1024


#define  MS_NO_ERROR               0
#define  MS_SOCKET_ERROR        -201
#define  MS_CONNECT_ERROR       -203
#define  MS_WRITE_ERROR         -210
#define  MS_READ_ERROR          -211
#define  MS_CLOSE_ERROR         -212
#define  MS_GETHOSTBYNAME_ERROR -220
#define  MS_GETHOSTNAME_ERROR   -221
#define  MS_TIMEOUT_ERROR       -231

// see master server code for the values
#define ADD_SERVER_MSG           101
#define REMOVE_SERVER_MSG        103
#define ADD_SERVERv2_MSG         104
#define GET_SERVER_MSG           200
#define GET_SHORT_SERVER_MSG     205
#define ASK_SERVER_MSG           206
#define ANSWER_ASK_SERVER_MSG    207
#define ASK_SERVER_MSG           206
#define ANSWER_ASK_SERVER_MSG    207
#define GET_MOTD_MSG             208
#define SEND_MOTD_MSG            209
#define GET_ROOMS_MSG			 210
#define SEND_ROOMS_MSG			 211
#define GET_ROOMS_HOST_MSG		 212
#define GET_VERSION_MSG			 213
#define SEND_VERSION_MSG		 214
#define GET_BANNED_MSG			 215 // Someone's been baaaaaad!
#define PING_SERVER_MSG			 216

#define HEADER_SIZE (sizeof (INT32)*4)

#define HEADER_MSG_POS    0
#define IP_MSG_POS       16
#define PORT_MSG_POS     32
#define HOSTNAME_MSG_POS 40


#if defined(_MSC_VER)
#pragma pack(1)
#endif

/** A message to be exchanged with the master server.
  */
typedef struct
{
	INT32 id;                  ///< Unused?
	INT32 type;                ///< Type of message.
	INT32 room;                ///< Because everyone needs a roomie.
	UINT32 length;             ///< Length of the message.
	char buffer[PACKET_SIZE]; ///< Actual contents of the message.
} ATTRPACK msg_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

typedef struct Copy_CVarMS_t
{
	char ip[64];
	char port[8];
	char name[64];
} Copy_CVarMS_s;
static Copy_CVarMS_s registered_server;
static time_t MSLastPing;

#if defined(_MSC_VER)
#pragma pack(1)
#endif
typedef struct
{
	char ip[16];         // Big enough to hold a full address.
	UINT16 port;
	UINT8 padding1[2];
	tic_t time;
} ATTRPACK ms_holepunch_packet_t;
#if defined(_MSC_VER)
#pragma pack()
#endif

// win32 or djgpp
#if defined (_WIN32) || defined (_WIN32_WCE) || defined (__DJGPP__)
#define ioctl ioctlsocket
#define close closesocket
#ifdef WATTCP
#define strerror strerror_s
#endif
#if defined (_WIN32) || defined (_WIN32_WCE)
#undef errno
#define errno h_errno // some very strange things happen when not using h_error
#endif
#endif

#if !defined (__APPLE_CC__)  && !defined(HAVE_LWIP) && (defined (_WIN32) || defined (_WIN32_WCE) || defined (__OS2__) || defined (SOLARIS) || defined(_arch_dreamcast)) && !defined (NONET)
// it seems windows doesn't define that... maybe some other OS? OS/2
static INT32 inet_aton(const char *hostname, struct in_addr *addr)
{
	return (addr->s_addr = inet_addr(hostname)) != htonl(INADDR_NONE);
}
#endif

static void Command_Listserv_f(void);
static void InternetServer_OnChange(void);
static void MasterServer_OnChange(void);
static void ServerName_OnChange(void);

#define DEF_PORT "28900"
consvar_t cv_internetserver = {"internetserver", "No", CV_CALL, CV_YesNo, InternetServer_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_masterserver = {"masterserver", "ms.srb2.org:"DEF_PORT, CV_SAVE, NULL, MasterServer_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername = {"servername", "SRB2 server", CV_SAVE, NULL, ServerName_OnChange, 0, NULL, NULL, 0, 0, NULL};

INT32 oldroomnum = 0;

static enum { MSCS_NONE, MSCS_WAITING, MSCS_REGISTERED, MSCS_FAILED } con_state = MSCS_NONE;

static INT32 msnode = -1;
#define current_port sock_port

#if (defined (_WIN32) || defined (_WIN32_WCE) || defined (_WIN32)) && !defined (NONET)
typedef SOCKET SOCKET_TYPE;
#define BADSOCKET INVALID_SOCKET
#define ERRSOCKET (SOCKET_ERROR)
#else
#if defined (__unix__) || defined (__APPLE__) || defined (__HAIKU__)
typedef int SOCKET_TYPE;
#else
typedef unsigned long SOCKET_TYPE;
#endif
#define BADSOCKET (SOCKET_TYPE)(~0)
#define ERRSOCKET (-1)
#endif

#if defined (WATTCP) || defined (_WIN32)
typedef int socklen_t;
#endif

#ifndef NONET
static SOCKET_TYPE socket_fd = BADSOCKET; // WINSOCK socket
static struct sockaddr_in addr;
static struct timeval select_timeout;
static fd_set wset;
static size_t recvfull(SOCKET_TYPE s, char *buf, size_t len, int flags);
#endif

// Room list is an external variable now.
// Avoiding having to get info ten thousand times...
msg_rooms_t room_list[NUM_LIST_ROOMS+1]; // +1 for easy test

/** Adds variables and commands relating to the master server.
  *
  * \sa cv_internetserver, cv_masterserver, cv_servername,
  *     Command_Listserv_f
  */
void AddMServCommands(void)
{
	CV_RegisterVar(&cv_internetserver);
	CV_RegisterVar(&cv_masterserver);
	CV_RegisterVar(&cv_servername);
	COM_AddCommand("listserv", Command_Listserv_f);
}

/** Closes the connection to the master server.
  *
  * \todo Fix for Windows?
  */
static void CloseConnection(void)
{
#ifndef NONET
	if (socket_fd != (SOCKET_TYPE)ERRSOCKET && socket_fd != BADSOCKET)
		close(socket_fd);
	socket_fd = BADSOCKET;
#endif
}

//
// MS_Write():
//
static INT32 MS_Write(msg_t *msg)
{
#ifdef NONET
	msg = NULL;
	return MS_WRITE_ERROR;
#else
	size_t len;

	if (msg->length == 0)
		msg->length = (INT32)strlen(msg->buffer);
	len = msg->length + HEADER_SIZE;

	msg->type = htonl(msg->type);
	msg->length = htonl(msg->length);
	msg->room = htonl(msg->room);

	if ((size_t)send(socket_fd, (char *)msg, (int)len, 0) != len)
		return MS_WRITE_ERROR;
	return 0;
#endif
}

//
// MS_Read():
//
static INT32 MS_Read(msg_t *msg)
{
#ifdef NONET
	msg = NULL;
	return MS_READ_ERROR;
#else
	if (recvfull(socket_fd, (char *)msg, HEADER_SIZE, 0) != HEADER_SIZE)
		return MS_READ_ERROR;

	msg->type = ntohl(msg->type);
	msg->length = ntohl(msg->length);
	msg->room = ntohl(msg->room);

	if (!msg->length) // fix a bug in Windows 2000
		return 0;

	if (recvfull(socket_fd, (char *)msg->buffer, msg->length, 0) != msg->length)
		return MS_READ_ERROR;
	return 0;
#endif
}

/** Gets a list of game servers from the master server.
  */
static INT32 GetServersList(void)
{
	msg_t msg;
	INT32 count = 0;

	msg.type = GET_SERVER_MSG;
	msg.length = 0;
	msg.room = 0;
	if (MS_Write(&msg) < 0)
		return MS_WRITE_ERROR;

	while (MS_Read(&msg) >= 0)
	{
		if (!msg.length)
		{
			if (!count)
				CONS_Printf("No servers currently running.\n");
			return MS_NO_ERROR;
		}
		count++;
		CONS_Printf("%s",msg.buffer);
	}

	return MS_READ_ERROR;
}

/** Get the MOTD from the master server.
  */
static inline INT32 GetMSMOTD(void)
{
	msg_t msg;
	INT32 count = 0;

	msg.type = GET_MOTD_MSG;
	msg.length = 0;
	if (MS_Write(&msg) < 0)
		return MS_WRITE_ERROR;

	while (MS_Read(&msg) >= 0)
	{
		if (!msg.length)
		{
			if (!count)
				CONS_Printf("No servers currently running.\n");
			return MS_NO_ERROR;
		}
		count++;
		CONS_Printf("%s",msg.buffer);
	}

	return MS_READ_ERROR;
}

//
// MS_GetIP()
//
#ifndef NONET
static INT32 MS_GetIP(const char *hostname)
{
	struct hostent *host_ent;
	if (!inet_aton(hostname, (void *)&addr.sin_addr))
	{
		/// \todo only when we are connected to the Internet, or use a non blocking call
		host_ent = gethostbyname(hostname);
		if (!host_ent)
			return MS_GETHOSTBYNAME_ERROR;
		M_Memcpy(&addr.sin_addr, host_ent->h_addr_list[0], sizeof (struct in_addr));
	}
	return 0;
}
#endif

//
// MS_Connect()
//
static INT32 MS_Connect(const char *ip_addr, const char *str_port, INT32 async)
{
#ifdef NONET
	str_port = ip_addr = NULL;
	async = MS_CONNECT_ERROR;
	return async;
#else
	socklen_t j = (socklen_t)sizeof(addr);
//	I_InitTcpNetwork(); this is already done on startup in D_SRB2Main()
	if (!I_InitTcpDriver()) // this is done only if not already done
		return MS_SOCKET_ERROR;
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == BADSOCKET || socket_fd == (SOCKET_TYPE)ERRSOCKET)
		return MS_SOCKET_ERROR;

	if (MS_GetIP(ip_addr) == MS_GETHOSTBYNAME_ERROR)
		return MS_GETHOSTBYNAME_ERROR;
	addr.sin_port = htons((UINT16)atoi(str_port));

	if (async) // do asynchronous connection
	{
#ifdef WATTCP
		char res = 1;
#else
		unsigned long res = 1;
#endif

		ioctl(socket_fd, FIONBIO, &res);

		if (connect(socket_fd, (void *)&addr, j) == ERRSOCKET)
		{
#ifdef _WIN32 // humm, on win32/win64 it doesn't work with EINPROGRESS (stupid windows)
			if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
			if (errno != EINPROGRESS)
#endif
			{
				con_state = MSCS_FAILED;
				CloseConnection();
				return MS_CONNECT_ERROR;
			}
		}
		con_state = MSCS_WAITING;
		FD_ZERO(&wset);
		FD_SET(socket_fd, &wset);
		select_timeout.tv_sec = 0, select_timeout.tv_usec = 0;
	}
	else if (connect(socket_fd, (void *)&addr, j) == ERRSOCKET)
		return MS_CONNECT_ERROR;
	return 0;
#endif
}

#define NUM_LIST_SERVER MAXSERVERLIST
const msg_server_t *GetShortServersList(INT32 room)
{
	static msg_server_t server_list[NUM_LIST_SERVER+1]; // +1 for easy test
	msg_t msg;
	INT32 i;

	// updated now
	oldroomnum = room;

	// we must be connected to the master server before writing to it
	if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		M_StartMessage("There was a problem connecting to\nthe Master Server", NULL, MM_NOTHING);
		return NULL;
	}

	msg.type = GET_SHORT_SERVER_MSG;
	msg.length = 0;
	msg.room = room;
	if (MS_Write(&msg) < 0)
		return NULL;

	for (i = 0; i < NUM_LIST_SERVER && MS_Read(&msg) >= 0; i++)
	{
		if (!msg.length)
		{
			server_list[i].header.buffer[0] = 0;
			CloseConnection();
			return server_list;
		}
		M_Memcpy(&server_list[i], msg.buffer, sizeof (msg_server_t));
		server_list[i].header.buffer[0] = 1;
	}
	CloseConnection();
	if (i == NUM_LIST_SERVER)
	{
		server_list[i].header.buffer[0] = 0;
		return server_list;
	}
	else
		return NULL;
}

INT32 GetRoomsList(boolean hosting)
{
	static msg_ban_t banned_info[1];
	msg_t msg;
	INT32 i;

	// we must be connected to the master server before writing to it
	if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		M_StartMessage("There was a problem connecting to\nthe Master Server", NULL, MM_NOTHING);
		return -1;
	}

	if (hosting)
		msg.type = GET_ROOMS_HOST_MSG;
	else
		msg.type = GET_ROOMS_MSG;
	msg.length = 0;
	msg.room = 0;
	if (MS_Write(&msg) < 0)
	{
		room_list[0].id = 1;
		strcpy(room_list[0].motd,"Master Server Offline.");
		strcpy(room_list[0].name,"Offline");
		return -1;
	}

	for (i = 0; i < NUM_LIST_ROOMS && MS_Read(&msg) >= 0; i++)
	{
		if(msg.type == GET_BANNED_MSG)
		{
			char banmsg[1000];
			M_Memcpy(&banned_info[0], msg.buffer, sizeof (msg_ban_t));
			if (hosting)
				sprintf(banmsg, "You have been banned from\nhosting netgames.\n\nUnder the following IP Range:\n%s - %s\n\nFor the following reason:\n%s\n\nYour ban will expire on:\n%s",banned_info[0].ipstart,banned_info[0].ipend,banned_info[0].reason,banned_info[0].endstamp);
			else
				sprintf(banmsg, "You have been banned from\njoining netgames.\n\nUnder the following IP Range:\n%s - %s\n\nFor the following reason:\n%s\n\nYour ban will expire on:\n%s",banned_info[0].ipstart,banned_info[0].ipend,banned_info[0].reason,banned_info[0].endstamp);
			M_StartMessage(banmsg, NULL, MM_NOTHING);
			cv_internetserver.value = false;
			return -2;
		}
		if (!msg.length)
		{
			room_list[i].header.buffer[0] = 0;
			CloseConnection();
			return 1;
		}
		M_Memcpy(&room_list[i], msg.buffer, sizeof (msg_rooms_t));
		room_list[i].header.buffer[0] = 1;
	}
	CloseConnection();
	if (i == NUM_LIST_ROOMS)
	{
		room_list[i].header.buffer[0] = 0;
		return 1;
	}
	else
	{
		room_list[0].id = 1;
		strcpy(room_list[0].motd,"Master Server Offline.");
		strcpy(room_list[0].name,"Offline");
		return -1;
	}
}

#ifdef UPDATE_ALERT
const char *GetMODVersion(void)
{
	static msg_t msg;


	// we must be connected to the master server before writing to it
	if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		M_StartMessage("There was a problem connecting to\nthe Master Server", NULL, MM_NOTHING);
		return NULL;
	}

	msg.type = GET_VERSION_MSG;
	msg.length = sizeof MODVERSION;
	msg.room = MODID; // Might as well use it for something.
	sprintf(msg.buffer,"%d",MODVERSION);
	if (MS_Write(&msg) < 0)
		return NULL;

	MS_Read(&msg);
	CloseConnection();

	if(strcmp(msg.buffer,"NULL") != 0)
	{
		return msg.buffer;
	}
	else
		return NULL;
}
#endif

/** Gets a list of game servers. Called from console.
  */
static void Command_Listserv_f(void)
{
	if (con_state == MSCS_WAITING)
	{
		CONS_Printf("Not yet registered to the master server.\n");
		return;
	}

	CONS_Printf("Retrieving server list...\n");

	if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}

	if (GetServersList())
		CONS_Printf("cannot get server list\n");

	CloseConnection();
}

FUNCMATH static const char *int2str(INT32 n)
{
	INT32 i;
	static char res[16];

	res[15] = '\0';
	res[14] = (char)((char)(n%10)+'0');
	for (i = 13; (n /= 10); i--)
		res[i] = (char)((char)(n%10)+'0');

	return &res[i+1];
}

#ifndef NONET
static INT32 ConnectionFailed(void)
{
	con_state = MSCS_FAILED;
	CONS_Printf("Connection to master server failed\n");
	CloseConnection();
	return MS_CONNECT_ERROR;
}
#endif

/** Tries to register the local game server on the master server.
  */
static INT32 AddToMasterServer(boolean firstadd)
{
#ifdef NONET
	(void)firstadd;
#else
	static INT32 retry = 0;
	int i, res;
	socklen_t j;
	msg_t msg;
	msg_server_t *info = (msg_server_t *)msg.buffer;
	INT32 room = -1;
	fd_set tset;
	time_t timestamp = time(NULL);
	UINT32 signature, tmp;
	const char *insname;

	M_Memcpy(&tset, &wset, sizeof (tset));
	res = select(255, NULL, &tset, NULL, &select_timeout);
	if (res != ERRSOCKET && !res)
	{
		if (retry++ > 30) // an about 30 second timeout
		{
			retry = 0;
			CONS_Printf("Timeout on masterserver\n");
			MSLastPing = timestamp;
			return ConnectionFailed();
		}
		return MS_CONNECT_ERROR;
	}
	retry = 0;
	if (res == ERRSOCKET)
	{
		if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
		{
			CONS_Printf("Mastserver error on select #%u: %s\n", errno, strerror(errno));
			MSLastPing = timestamp;
			return ConnectionFailed();
		}
	}

	// so, the socket is writable, but what does that mean, that the connection is
	// ok, or bad... let see that!
	j = (socklen_t)sizeof (i);
	getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char *)&i, &j);
	if (i) // it was bad
	{
		CONS_Printf("Masterserver getsockopt error #%u: %s\n", errno, strerror(errno));
		MSLastPing = timestamp;
		return ConnectionFailed();
	}

	if (dedicated && (M_CheckParm("-room") && M_IsNextParm()))
	{
		room = atoi(M_GetNextParm());
		if(room == 0)
			room = -1;
	}
	else if(dedicated)
		room = -1;
	else
		room = cv_chooseroom.value;

	for(signature = 0, insname = cv_servername.string; *insname; signature += *insname++);
	tmp = (UINT32)(signature * (size_t)&MSLastPing);
	signature *= tmp;
	signature &= 0xAAAAAAAA;
	M_Memcpy(&info->header.signature, &signature, sizeof (UINT32));

	strcpy(info->ip, "");
	strcpy(info->port, int2str(current_port));
	strcpy(info->name, cv_servername.string);
	M_Memcpy(&info->room, & room, sizeof (INT32));
	sprintf(info->version, "%d.%d.%d", VERSION/100, VERSION%100, SUBVERSION);
	strcpy(registered_server.name, cv_servername.string);

	if(firstadd)
		msg.type = ADD_SERVER_MSG;
	else
		msg.type = PING_SERVER_MSG;

	msg.length = (UINT32)sizeof (msg_server_t);
	msg.room = 0;
	if (MS_Write(&msg) < 0)
	{
		MSLastPing = timestamp;
		return ConnectionFailed();
	}

	if(con_state != MSCS_REGISTERED)
		CONS_Printf("Master Server Updated Successfully!\n");

	MSLastPing = timestamp;
	con_state = MSCS_REGISTERED;
	CloseConnection();
#endif
	return MS_NO_ERROR;
}

static INT32 RemoveFromMasterSever(void)
{
	msg_t msg;
	msg_server_t *info = (msg_server_t *)msg.buffer;

	strcpy(info->header.buffer, "");
	strcpy(info->ip, "");
	strcpy(info->port, int2str(current_port));
	strcpy(info->name, registered_server.name);
	sprintf(info->version, "%d.%d.%d", VERSION/100, VERSION%100, SUBVERSION);

	msg.type = REMOVE_SERVER_MSG;
	msg.length = (UINT32)sizeof (msg_server_t);
	msg.room = 0;
	if (MS_Write(&msg) < 0)
		return MS_WRITE_ERROR;

	return MS_NO_ERROR;
}

const char *GetMasterServerPort(void)
{
	const char *t = cv_masterserver.string;

	while ((*t != ':') && (*t != '\0'))
		t++;

	if (*t)
		return ++t;
	else
		return DEF_PORT;
}

/** Gets the IP address of the master server. Actually, it seems to just
  * return the hostname, instead; the lookup is done elsewhere.
  *
  * \return Hostname of the master server, without port number on the end.
  * \todo Rename function?
  */
const char *GetMasterServerIP(void)
{
	static char str_ip[64];
	char *t = str_ip;

	if (strstr(cv_masterserver.string, "srb2.ssntails.org:28910")
	 || strstr(cv_masterserver.string, "srb2.servegame.org:28910")
	 || strstr(cv_masterserver.string, "srb2.servegame.org:28900")
	   )
	{
		// replace it with the current default one
		CV_Set(&cv_masterserver, cv_masterserver.defaultvalue);
	}

	strcpy(t, cv_masterserver.string);

	while ((*t != ':') && (*t != '\0'))
		t++;
	*t = '\0';

	return str_ip;
}

void MSOpenUDPSocket(void)
{
#ifndef NONET
	if (I_NetMakeNode)
	{
		// If it's already open, there's nothing to do.
		if (msnode < 0)
		{
			char hostname[24];

			MS_GetIP(GetMasterServerIP());

			sprintf(hostname, "%s:%d",
#ifdef _arch_dreamcast
				inet_ntoa(*(UINT32 *)&addr.sin_addr),
#else
				inet_ntoa(addr.sin_addr),
#endif
				atoi(GetMasterServerPort())+1);
			msnode = I_NetMakeNode(hostname);
		}
	}
	else
#endif
		msnode = -1;
}

void MSCloseUDPSocket(void)
{
	if (msnode != INT16_MAX) I_NetFreeNodenum(msnode);
	msnode = -1;
}

void RegisterServer(void)
{
	if (con_state == MSCS_REGISTERED || con_state == MSCS_WAITING)
			return;

	CONS_Printf("Registering this server to the master server...\n");

	strcpy(registered_server.ip, GetMasterServerIP());
	strcpy(registered_server.port, GetMasterServerPort());

	if (MS_Connect(registered_server.ip, registered_server.port, 1))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}
	MSOpenUDPSocket();

	// keep the TCP connection open until AddToMasterServer() is completed;
}

static inline void SendPingToMasterServer(void)
{
/*	static tic_t next_time = 0;
	tic_t cur_time;
	char *inbuffer = (char*)netbuffer;

	cur_time = I_GetTime();
	if (!netgame)
		UnregisterServer();
	else if (cur_time > next_time) // ping every 2 second if possible
	{
		next_time = cur_time+2*TICRATE;

		if (con_state == MSCS_WAITING)
			AddToMasterServer();

		if (con_state != MSCS_REGISTERED)
			return;

		// cur_time is just a dummy data to send
		WRITEUINT32(inbuffer, cur_time);
		doomcom->datalength = sizeof (cur_time);
		doomcom->remotenode = (INT16)msnode;
		I_NetSend();
	}
*/

// Here, have a simpler MS Ping... - Cue
	if(time(NULL) > (MSLastPing+(60*2)) && con_state != MSCS_NONE)
	{
		//CONS_Printf("%ld (current time) is greater than %d (Last Ping Time)\n", time(NULL), MSLastPing);
		if(MSLastPing < 1)
			AddToMasterServer(true);
		else
			AddToMasterServer(false);
	}
}

void SendAskInfoViaMS(INT32 node, tic_t asktime)
{
	const char *address;
	UINT16 port;
	char *inip;
	ms_holepunch_packet_t mshpp;

	MSOpenUDPSocket();

	// This must be called after calling MSOpenUDPSocket, due to the
	// static buffer.
	address = I_GetNodeAddress(node);

	// Copy the IP address into the buffer.
	inip = mshpp.ip;
	while(*address && *address != ':') *inip++ = *address++;
	*inip = '\0';

	// Get the port.
	port = (UINT16)(*address++ ? atoi(address) : 0);
	mshpp.port = SHORT(port);

	// Set the time for ping calculation.
	mshpp.time = LONG(asktime);

	// Send to the MS.
	M_Memcpy(netbuffer, &mshpp, sizeof(mshpp));
	doomcom->datalength = sizeof(ms_holepunch_packet_t);
	doomcom->remotenode = (INT16)msnode;
	I_NetSend();
}

void UnregisterServer(void)
{
	if (con_state != MSCS_REGISTERED)
	{
		con_state = MSCS_NONE;
		CloseConnection();
		return;
	}

	con_state = MSCS_NONE;

	CONS_Printf("Unregistering this server to the master server...\n");

	if (MS_Connect(registered_server.ip, registered_server.port, 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}

	if (RemoveFromMasterSever() < 0)
		CONS_Printf("cannot remove this server from the master server\n");

	CloseConnection();
	MSCloseUDPSocket();
	MSLastPing = 0;
}

void MasterClient_Ticker(void)
{
	if (server && cv_internetserver.value)
		SendPingToMasterServer();
}

static void ServerName_OnChange(void)
{
	UnregisterServer();
	RegisterServer();
}

static void InternetServer_OnChange(void)
{
/*	if (cv_internetserver.value)
		RegisterServer();
	else
		UnregisterServer(); */
	if(cv_internetserver.value && Playing())
	{
		CV_StealthSetValue(&cv_internetserver, 0);
		CONS_Printf("You cannot register on the Master Server mid-game, please end your current session and re-host if you wish to advertise your server on the Master Server.\n");
		return;
	}
#ifndef NONET
	if (!Playing() && !dedicated)
		M_AlterRoomOptions();
#endif
}

static void MasterServer_OnChange(void)
{
	UnregisterServer();
	RegisterServer();
}

#ifndef NONET
// Like recv, but waits until we've got enough data to fill the buffer.
static size_t recvfull(SOCKET_TYPE s, char *buf, size_t len, int flags)
{
	/* Total received. */
	size_t totallen = 0;

	while(totallen < len)
	{
		ssize_t ret = (ssize_t)recv(s, buf + totallen, (int)(len - totallen), flags);

		/* Error. */
		if(ret == -1)
			return (size_t)-1;

		totallen += ret;
	}

	return totallen;
}
#endif
