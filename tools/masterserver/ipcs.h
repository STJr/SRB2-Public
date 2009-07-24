// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000 by DooM Legacy Team.
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
//
//-----------------------------------------------------------------------------

#ifndef _IPCS_H_
#define _IPCS_H_

#if defined (_WIN32) || defined ( __OS2__)
#include <io.h>
#include <sys/types.h>
typedef int socklen_t;
#if defined (__OS2__)
#include <netinet/in.h>
#endif
#endif
#ifdef _WIN32
#include <winsock.h>
#define close closesocket
#define SHUT_RD SD_RECEIVE

#ifndef SD_RECEIVE
#define SD_RECEIVE 0
#endif

#else
#include <arpa/inet.h>   // inet_addr(),...
#endif

#ifndef SOCKET
#define SOCKET u_int
#endif

// ================================ DEFINITIONS ===============================

#define PACKET_SIZE 1024
#define MAX_CLIENT    64

#ifndef _WIN32
#define NO_ERROR                      0
#define SOCKET_ERROR               -201
#endif
#define BIND_ERROR                 -202
#define CONNECT_ERROR              -203
#define LISTEN_ERROR               -204
#define ACCEPT_ERROR               -205
#define WRITE_ERROR                -210
#define READ_ERROR                 -211
#define CLOSE_ERROR                -212
#define GETHOSTBYNAME_ERROR        -220
#define SELECT_ERROR               -230
#define TIMEOUT_ERROR              -231
#define MALLOC_ERROR               -301

#define INVALID_MSG                   -1
#define ACCEPT_MSG                   100
#define ADD_SERVER_MSG               101
#define ADD_CLIENT_MSG               102
#define REMOVE_SERVER_MSG            103
#define ADD_SERVERv2_MSG             104
#define GET_SERVER_MSG               200
#define SEND_SERVER_MSG              201
#define GET_LOGFILE_MSG              202
#define SEND_FILE_MSG                203
#define ERASE_LOGFILE_MSG            204
#define GET_SHORT_SERVER_MSG         205
#define SEND_SHORT_SERVER_MSG        206
#define ASK_SERVER_MSG               206
#define ANSWER_ASK_SERVER_MSG        207
#define GET_MOTD_MSG                 208
#define SEND_MOTD_MSG                209
#define UDP_RECV_MSG                 300
#define TIMEOUT_MSG                  301
#define HTTP_REQUEST_MSG       875770417    // "4321"
#define SEND_HTTP_REQUEST_MSG  875770418    // "4322"
#define TEXT_REQUEST_MSG       825373494    // "1236"
#define SEND_TEXT_REQUEST_MSG  825373495    // "1237"
#define RSS92_REQUEST_MSG      825373496    // "1238"
#define SEND_RSS92_REQUEST_MSG 825373497    // "1239"
#define RSS10_REQUEST_MSG      825373744    // "1240"
#define SEND_RSS10_REQUEST_MSG 825373745    // "1241"
#define ADD_PSERVER_MSG        0xabacab81    // this number just need to be different than the others
#define REMOVE_PSERVER_MSG     0xabacab82

#define HEADER_SIZE ((long)sizeof (long)*3)

#define HEADER_MSG_POS      0
#define IP_MSG_POS         16
#define PORT_MSG_POS       32
#define HOSTNAME_MSG_POS   40

// Keep this structure 8 bytes aligned (current size is 80)
typedef struct
{
	union
	{
		char buffer[16]; // information such as password
		unsigned int signature;
	} header;

	char ip[16];
	char port[8];
	char name[32];       
	char version[8]; // format is: x.yy.z (like 1.30.2 or 1.31)
} msg_server_t;

typedef struct
{
	long id;
	long type;
	long length;
	char buffer[PACKET_SIZE];
} msg_t;

class CSocket
{
protected:
	sockaddr_in addr;
	msg_t msg;
	fd_set rset;
public:
	int getIP(char *);
	CSocket();
	~CSocket();
};

class CServerSocket : public CSocket
{
private:
	sockaddr_in udp_addr;
	sockaddr_in udp_in_addr;
	SOCKET udp_fd;
	SOCKET accept_fd;
	size_t num_clients;
	SOCKET client_fd[MAX_CLIENT];
	sockaddr_in client_addr[MAX_CLIENT];

public:
	int deleteClient(size_t id);
	int listen(char *str_port);
	int accept();
	int read(msg_t *msg);
	const char *getUdpIP();
	const char *getUdpPort(bool);
	int write(msg_t *msg);
	int writeUDP(char *data, int length, char *ip, unsigned short port);
	const char *getClientIP(size_t id);
	const char *getClientPort(size_t id);
	CServerSocket();
	~CServerSocket();
};

class CClientSocket : public CSocket
{
private:
	SOCKET socket_fd;
public:
	int connect(char *ip_addr, char *str_port);
	int read(msg_t *msg);
	int write(msg_t *msg);
	CClientSocket();
	~CClientSocket();
};

// ================================== PROTOS ==================================

// ================================== EXTERNS =================================

#endif
