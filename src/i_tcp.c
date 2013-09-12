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
/// \brief TCP driver, socket code.
///
///	This is not really OS-dependent because all OSes have the same socket API.
///	Just use ifdef for OS-dependent parts.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#ifdef __OS2__
#include <sys/types.h>
#include <sys/time.h>
#endif // __OS2__

#include "doomdef.h"

#if defined (NOMD5) && !defined (NONET)
//#define NONET
#endif

#if !defined (NONET) && !defined (NOIPX)
#define USEIPX //Alam: Remline to turn off IPX support
#ifdef __linux__
//#define HAVE_IP6
#endif
#endif

#ifndef NONET
#if (defined (_WIN32) || defined (_WIN32_WCE)) && !defined (_XBOX)
#include <winsock.h>
#ifdef USEIPX
#include <wsipx.h>
#endif
#else
#if !defined (SCOUW2) && !defined (SCOUW7) && !defined (__OS2__)
#ifdef HAVE_LWIP
#include <lwip/inet.h>
#else
#include <arpa/inet.h>
#endif
#endif

#ifdef HAVE_LWIP
#include <lwip/sockets.h>
#define ioctl lwip_ioctl
#else
#ifdef __APPLE_CC__
#define _BSD_SOCKLEN_T_
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#if defined(_arch_dreamcast) && !defined(HAVE_LWIP)
#include <kos/net.h>
#elif defined(HAVE_LWIP)
#include <lwip/lwip.h>
#else
#include <netdb.h>
#include <sys/ioctl.h>
#endif
#include <errno.h>
#include <time.h>

#ifdef _arch_dreamcast
#include "sdl/SRB2DC/dchelp.h"
#endif

#define STD_STRING_LEN 256 // Just some standard length for a char string

#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)
	#include <sys/time.h>
	#ifdef __GLIBC__
		#include <netipx/ipx.h>
	#elif defined (USEIPX)
		#ifdef FREEBSD
			#include <netipx/ipx.h>
		#elif defined (__CYGWIN__)
			#include <wsipx.h>
		#else
			#include <linux/ipx.h>
		#endif
	#endif // USEIPX
	#ifndef __CYGWIN__
	typedef struct sockaddr_ipx SOCKADDR_IPX, *PSOCKADDR_IPX;
	#endif
#endif // UNIXCOMMON
#endif // win32

#if defined (_WIN32_WCE) || defined (_WIN32)
	// some undefined under win32
	#undef errno
	//#define errno WSAGetLastError() //Alam_GBC: this is the correct way, right?
	#define errno h_errno // some very strange things happen when not using h_error?!?
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#define EMSGSIZE WSAEMSGSIZE
	#define ECONNREFUSED WSAECONNREFUSED
	#define ETIMEDOUT WSAETIMEDOUT
	#ifndef IOC_VENDOR
	#define IOC_VENDOR 0x18000000
	#endif
	#ifndef _WSAIOW
	#define _WSAIOW(x,y) (IOC_IN|(x)|(y))
	#endif
	#ifndef SIO_UDP_CONNRESET
	#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
	#endif
#endif

#ifdef __DJGPP__
#ifdef WATTCP // Alam_GBC: Wattcp may need this
#include <tcp.h>
#define strerror strerror_s
#else // wattcp
#include <lsck/lsck.h>
#endif // libsocket
#endif // djgpp

#ifdef USEIPX

#if defined (__DJGPP__) || defined (__OS2__)
// ipx not yet supported in libsocket (cut and pasted from wsipx.h (winsock)
typedef struct sockaddr_ipx
{
	INT16 sa_family;
	char sa_netnum[4];
	char sa_nodenum[6];
	UINT16 sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX;
#define NSPROTO_IPX 1000
#endif

#ifndef AF_IPX
#define AF_IPX 23 // Novell Internet Protocol
#endif

#ifndef PF_IPX
#define PF_IPX AF_IPX
#endif

#ifndef NSPROTO_IPX
#define NSPROTO_IPX PF_IPX
#endif

#endif

#ifdef HAVE_IP6
#include <netinet/in.h>
#elif defined (_WIN32) || defined (__DJGPP__) || defined (__HAIKU__)
#if !defined (_SA_FAMILY_T)
typedef UINT16 sa_family_t;
#endif
#endif

typedef union
{
	sa_family_t sa_family;
	struct sockaddr_in ip;
#ifdef USEIPX
	struct sockaddr_ipx ipx;
#endif
#ifdef HAVE_IP6
	struct sockaddr_in6 ip6;
#endif
} mysockaddr_t;

#endif // !NONET

static int ipx = 0;

#define MAXBANS 100

#include "i_system.h"
#include "i_net.h"
#include "d_net.h"
#include "i_tcp.h"
#ifndef NONET
static mysockaddr_t clientaddress[MAXNETNODES+1];
static boolean nodeconnected[MAXNETNODES+1];
static mysockaddr_t banned[MAXBANS];
static UINT8 bannedmask[MAXBANS];
static mysockaddr_t shunned[MAXBANS];
static UINT8 shunnedmask[MAXBANS];
#endif
#include "m_argv.h"

#include "doomstat.h"

// win32 or djgpp
#if defined (_WIN32) || defined (_WIN32_WCE) || defined (__DJGPP__)
	// winsock stuff (in winsock a socket is not a file)
	#define ioctl ioctlsocket
	#define close closesocket

	#ifdef _WIN32_WCE
	#include "sdl/SRB2CE/cehelp.h"
	#endif

#endif

//#if defined (_WIN32) || defined (_WIN32_WCE) || defined (__CYGWIN__)
#ifdef __DJGPP__

#ifdef WATTCP
#define SELECTTEST
#endif

#elif defined(HAVE_LWIP)
#define SELECTTEST
#elif !defined( _arch_dreamcast)
#define SELECTTEST
#endif

//#endif

#define DEFAULTPORT 5029

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

static SOCKET_TYPE mysocket = BADSOCKET;

static int numshun = 0;
static size_t numbans = 0;
static boolean SOCK_bannednode[MAXNETNODES+1]; /// \note do we really need the +1?
static boolean init_tcp_driver = false;

UINT16 sock_port = DEFAULTPORT;

#ifndef NONET

#ifdef WATTCP
static void wattcp_outch(char s)
{
	static char old = '\0';
	char pr[2] = {s,0};
	if (s == old && old == ' ') return;
	else old = s;
	if (s == '\r') CONS_Printf("\n");
	else if (s != '\n') CONS_Printf(pr);
}
#endif

static const char *SOCK_AddrToStr(mysockaddr_t *sk)
{
	static char s[64]; // 255.255.255.255:65535 or IPv6:65535
	if (sk->sa_family == AF_INET)
	{
		strcpy(s, inet_ntoa(sk->ip.sin_addr));
		if (sk->ip.sin_port != 0) strcat(s, va(":%d", ntohs(sk->ip.sin_port)));
	}
#ifdef USEIPX
	else
#if (defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON)) && !defined (__CYGWIN__)
	if (sk->sa_family == AF_IPX)
	{
#ifdef FREEBSD
		sprintf(s, "%s", ipx_ntoa(sk->ipx.sipx_addr));
#else
		sprintf(s,"%08x.%02x%02x%02x%02x%02x%02x:%d", sk->ipx.sipx_network,
			(UINT8)sk->ipx.sipx_node[0],
			(UINT8)sk->ipx.sipx_node[1],
			(UINT8)sk->ipx.sipx_node[2],
			(UINT8)sk->ipx.sipx_node[3],
			(UINT8)sk->ipx.sipx_node[4],
			(UINT8)sk->ipx.sipx_node[5],
			sk->ipx.sipx_port);
#endif
	}
#else
	if (sk->sa_family == AF_IPX)
	{
		sprintf(s, "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%d",
			(UINT8)sk->ipx.sa_netnum[0],
			(UINT8)sk->ipx.sa_netnum[1],
			(UINT8)sk->ipx.sa_netnum[2],
			(UINT8)sk->ipx.sa_netnum[3],
			(UINT8)sk->ipx.sa_nodenum[0],
			(UINT8)sk->ipx.sa_nodenum[1],
			(UINT8)sk->ipx.sa_nodenum[2],
			(UINT8)sk->ipx.sa_nodenum[3],
			(UINT8)sk->ipx.sa_nodenum[4],
			(UINT8)sk->ipx.sa_nodenum[5],
			sk->ipx.sa_socket);
	}
#endif // UNIXCOMMON
#endif // USEIPX
#ifdef HAVE_IP6
	if (sk->sa_family == AF_INET6)
	{
		inet_ntop(AF_INET6, &sk->ip6.sin6_addr, s, sizeof (sk->ip6));
	}
#endif
	else
		sprintf(s, "Unknown type");
	return s;
}
#endif

static const char *SOCK_GetNodeAddress(INT32 node)
{
	if (node == 0)
		return "self";
#ifdef NONET
	return NULL;
#else
	if (!nodeconnected[node])
		return NULL;
	return SOCK_AddrToStr(&clientaddress[node]);
#endif
}

static const char *SOCK_GetBanAddress(size_t ban)
{
	if (ban >= numbans)
		return NULL;
#ifdef NONET
	return NULL;
#else
	return SOCK_AddrToStr(&banned[ban]);
#endif
}

static const char *SOCK_GetBanMask(size_t ban)
{
#ifdef NONET
	(void)ban;
#else
	static char s[16]; //255.255.255.255 netmask? no, just CDIR for only
	if (ban >= numbans)
		return NULL;
	if (sprintf(s,"%d",bannedmask[ban]) > 0)
		return s;
#endif
	return NULL;
}

#ifdef USEIPX
static boolean IPX_cmpaddr(mysockaddr_t *a, mysockaddr_t *b, UINT8 mask)
{
	(void)mask;
#if (defined (__unix__) || defined (UNIXCOMMON)) && !defined (__CYGWIN__)
#ifdef FREEBSD
	return ipx_neteq(a->ipx.sipx_addr, b->ipx.sipx_addr)
		&& ipx_hosteq(a->ipx.sipx_addr, b->ipx.sipx_addr);
#else
	return ((!memcmp(&(a->ipx.sipx_network), &(b->ipx.sipx_network), 4))
		&& (!memcmp(&(a->ipx.sipx_node), &(b->ipx.sipx_node), 6)));
#endif
#else
	return ((!memcmp(&(a->ipx.sa_netnum), &(b->ipx.sa_netnum), 4))
		&& (!memcmp(&(a->ipx.sa_nodenum), &(b->ipx.sa_nodenum), 6)));
#endif // UNIXCOMMON
}
#endif // USEIPX

#ifndef NONET
static boolean UDP_cmpaddr(mysockaddr_t *a, mysockaddr_t *b, UINT8 mask)
{
	UINT32 bitmask = INADDR_NONE;

	if (mask && mask < 32)
		bitmask = htonl(-1 << (32 - mask));

	if (b->sa_family == AF_INET)
		return (a->ip.sin_addr.s_addr & bitmask) == (b->ip.sin_addr.s_addr & bitmask)
			&& (b->ip.sin_port == 0 || (a->ip.sin_port == b->ip.sin_port));
#ifdef HAVE_IP6
	else if (b->sa_family == AF_INET6)
		return memcmp(&a->ip6.sin6_addr, &b->ip6.sin6_addr, sizeof(b->ip6.sin6_addr))
			&& (b->ip6.sin6_port == 0 || (a->ip6.sin6_port == b->ip6.sin6_port));
#endif
	else
		return false;
}

static boolean (*SOCK_cmpaddr)(mysockaddr_t *a, mysockaddr_t *b, UINT8 mask);

static SINT8 getfreenode(void)
{
	SINT8 j;

	for (j = 0; j < MAXNETNODES; j++)
		if (!nodeconnected[j])
		{
			nodeconnected[j] = true;
			return j;
		}
	return -1;
}

// This is a hack. For some reason, nodes aren't being freed properly.
// This goes through and cleans up what nodes were supposed to be freed.
static void cleanupnodes(void)
{
	SINT8 j;

	if (!Playing())
		return;

	// Why can't I start at zero?
	for (j = 1; j < MAXNETNODES; j++)
		if (!nodeingame[j])
			nodeconnected[j] = false;
}
#endif

#ifndef NONET
static void SOCK_Get(void)
{
	int j;
	ssize_t c;
	socklen_t fromlen;
	mysockaddr_t fromaddress;

	do
	{
		fromlen = (socklen_t)sizeof(fromaddress);
		c = recvfrom(mysocket, (char *)&doomcom->data, MAXPACKETLENGTH, 0,
			(void *)&fromaddress, &fromlen);
		if (c == ERRSOCKET)
		{
			if ((errno == EWOULDBLOCK) || (errno == EMSGSIZE) || (errno == ECONNREFUSED) || (errno == ETIMEDOUT))
			{
				doomcom->remotenode = -1; // no packet
				return;
			}
#if defined (_WIN32)
			else if (errno == WSAECONNRESET) // 2k has some extra errors
			{
				DEBFILE("Connection reset (likely that the server isn't running)\n"); //Alam_GBC: how about DEBFILE instead of annoying the user?
				//D_QuitNetGame(); // Graue 07-04-2004: win32 only and quit
				doomcom->remotenode = -1;      // no packet too
				return;
				/// \todo see if the D_QuitNetGame actually fixes it, or whether it crashes or something
				/// Alam_GBC: this WSAECONNRESET happends alot when talking to a masterlist server, i am guess when talking too much at a time
				/// Later, hmmm, SIO_UDP_CONNRESET turned off should fix this
			}
#endif
			I_Error("SOCK_Get error #%u: %s\n\n(Disabling any firewalls and/or rebooting your computer may fix this problem)", errno, strerror(errno));
		}

		// check if it's a DoS attacker and don't respond.
		for (j = 0; j < numshun; j++)
			if (SOCK_cmpaddr(&fromaddress, &shunned[j], shunnedmask[j]))
				break;
	} while (j < numshun);

	// find remote node number
	for (j = 0; j < MAXNETNODES; j++)
	{
		if (SOCK_cmpaddr(&fromaddress, &clientaddress[j], 0))
		{
			doomcom->remotenode = (INT16)j; // good packet from a game player
			doomcom->datalength = (INT16)c;
			return;
		}
	}

	// not found

	// find a free slot
	cleanupnodes();
	j = getfreenode();
	if (j > 0)
	{
		size_t i;
		M_Memcpy(&clientaddress[j], &fromaddress, fromlen);
		DEBFILE(va("New node detected: node:%d address:%s\n", j,
				SOCK_GetNodeAddress(j)));
		doomcom->remotenode = (INT16)j; // good packet from a game player
		doomcom->datalength = (INT16)c;

		// check if it's a banned dude so we can send a refusal later
		for (i = 0; i < numbans; i++)
		{
			if (SOCK_cmpaddr(&fromaddress, &banned[i], bannedmask[i]))
			{
				SOCK_bannednode[j] = true;
				DEBFILE("This dude has been banned\n");
				break;
			}
		}
		if (i == numbans)
			SOCK_bannednode[j] = false;
		return;
	}

	DEBFILE("New node detected: No more free slots\n");
	doomcom->remotenode = -1; // no packet
}
#endif

// check if we can send (do not go over the buffer)
#ifndef NONET

static fd_set set;

#ifdef SELECTTEST
static boolean SOCK_CanSend(void)
{
	struct timeval timeval_for_select = {0, 0};
	fd_set          tset;
	int wselect;

	M_Memcpy(&tset, &set, sizeof (tset));
	wselect = select(255, NULL, &tset, NULL, &timeval_for_select);
	if (wselect >= 1)
	{
		if (FD_ISSET(mysocket, &tset))
			return true;
		else
			return false;
	}
	else if (wselect == 0)
	{
		return false;
	}
	else if (wselect == ERRSOCKET)
	{
		return false;
	}
	return false;
}

static boolean SOCK_CanGet(void)
{
	struct timeval timeval_for_select = {0, 0};
	fd_set          tset;
	int rselect;

	M_Memcpy(&tset, &set, sizeof (tset));
	rselect = select(255, &tset, NULL, NULL, &timeval_for_select);
	if (rselect >= 1)
	{
		if (FD_ISSET(mysocket, &tset))
			return true;
		else
			return false;
	}
	else if (rselect == 0)
	{
		return false;
	}
	else if (rselect == ERRSOCKET)
	{
		return false;
	}
	return false;
}
#endif
#endif

#ifndef NONET
static void SOCK_Send(void)
{
	ssize_t c;
	socklen_t d = (socklen_t)sizeof(struct sockaddr);

	if (!nodeconnected[doomcom->remotenode])
		return;

	c = sendto(mysocket, (char *)&doomcom->data, doomcom->datalength, 0,
		(struct sockaddr *)&clientaddress[doomcom->remotenode], d);

	if (c == ERRSOCKET && errno != ECONNREFUSED && errno != EWOULDBLOCK)
		I_Error("SOCK_Send, error sending to node %d (%s) #%u: %s", doomcom->remotenode,
			SOCK_GetNodeAddress(doomcom->remotenode), errno, strerror(errno));
}
#endif

#ifndef NONET
static void SOCK_FreeNodenum(INT32 numnode)
{
	// can't disconnect from self :)
	if (!numnode)
		return;

	DEBFILE(va("Free node %d (%s)\n", numnode, SOCK_GetNodeAddress(numnode)));

	nodeconnected[numnode] = false;

	// put invalid address
	memset(&clientaddress[numnode], 0, sizeof (clientaddress[numnode]));
}
#endif

//
// UDPsocket
//
#ifndef NONET
static SOCKET_TYPE UDP_Socket(void)
{
	SOCKET_TYPE s = BADSOCKET;
	struct sockaddr_in address;
#ifdef HAVE_IP6
	struct sockaddr_in6 address6;
#endif
	UINT16 sock_port_local;

#ifdef WATTCP
	char trueval = true;
#else
	unsigned long trueval = true;
#endif
	int i;
	socklen_t j;

	// allocate a socket
#ifdef HAVE_IP6
	s = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (s == (SOCKET_TYPE)ERRSOCKET || s == BADSOCKET)
#endif
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == (SOCKET_TYPE)ERRSOCKET || s == BADSOCKET)
		I_Error("UDP_Socket error #%u: Can't create socket: %s", errno, strerror(errno));

#ifdef _WIN32
	{ // Alam_GBC: disable the new UDP connection reset behavior for Win2k and up
#if 0
		DWORD dwBytesReturned = 0;
		BOOL bfalse = FALSE;
		WSAIoctl(s, SIO_UDP_CONNRESET, &bfalse, sizeof(bfalse),
		         NULL, 0, &dwBytesReturned, NULL, NULL);
#else
		unsigned long falseval = false;
		ioctl(s, SIO_UDP_CONNRESET, &falseval);
#endif
	}
#endif

	memset(&address, 0, sizeof (address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_NONE);
	if (M_CheckParm("-bindaddr"))
	{
		if (!M_IsNextParm())
			I_Error("syntax: -bindaddr <ip-address>");
		address.sin_addr.s_addr = inet_addr(M_GetNextParm());
	}
	if (address.sin_addr.s_addr == htonl(INADDR_NONE))
	{
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		j = (socklen_t)sizeof(trueval);
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&trueval, j);
	}

	//Hurdler: I'd like to put a server and a client on the same computer
	//BP: in fact for client we can use any free port we want i have read
	//    in some doc that connect in udp can do it for us...
	address.sin_port = htons(0); //????
	if (M_CheckParm("-clientport"))
	{
		if (!M_IsNextParm())
			I_Error("syntax: -clientport <portnum>");
		sock_port_local = (UINT16)atoi(M_GetNextParm());
	}
	else
		sock_port_local = sock_port;

	address.sin_port = htons(sock_port_local);

	//setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&trueval, sizeof (trueval));

	j = (socklen_t)sizeof(address);
	if (bind(s, (void *)&address, j) == ERRSOCKET)
	{
#ifdef _WIN32
		if (errno == WSAEADDRINUSE)
			I_Error("UDP_Socket error: The address and port SRB2 had attempted to bind to is already in use.\n"
				"\nThis isn't a normal error, and probably indicates that something network-related\n"
				"on your computer is configured improperly.");
#endif
		I_Error("UDP_Socket error #%u: %s", errno, strerror(errno));
	}

#ifdef HAVE_IP6
	memset(&address6, 0, sizeof (address6));
	address6.sin6_family = AF_INET6;
	address6.sin6_addr = in6addr_any;

	//Hurdler: I'd like to put a server and a client on the same computer
	//BP: in fact for client we can use any free port we want i have read
	//    in some doc that connect in udp can do it for us...
	address6.sin6_port = htons(sock_port_local);

	if (bind(s, (void *)&address6, sizeof (address6)) == ERRSOCKET)
	{
		CONS_Printf("failed to bind with IPv6 stack");
	}
#endif

	// make it non blocking
	if (ioctl(s, FIONBIO, &trueval) != 0)
	{
		I_Error("UDP_Socket error. Could not set socket to non blocking mode");
	}

	// make it broadcastable
	j = (socklen_t)sizeof(trueval);
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, j))
	{
		I_Error("UDP_Socket error. Could not set socket to allow broadcast");
	}

	j = (socklen_t)sizeof(i);
	getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, &j); // FIXME: so an INT32 value is written to a (char *); portability!!!!!!!
	CONS_Printf("Network system buffer: %dKb\n", i>>10);

	if (i < 64<<10) // 64k
	{
		i = 64<<10;
		j = (socklen_t)sizeof(i);
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, j);
		getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, &j); // FIXME: so an INT32 value is written to a (char *); portability!!!!!!!
		if (i < 64<<10)
			CONS_Printf("Can't set buffer length to 64k, file transfer will be bad\n");
		else
			CONS_Printf("Network system buffer set to: %dKb\n",i>>10);
	}

	// ip + udp
	packetheaderlength = 20 + 8; // for stats

	clientaddress[0].sa_family = AF_INET;
	clientaddress[0].ip.sin_port = htons(sock_port_local);
	clientaddress[0].ip.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //GetLocalAddress(); // my own ip
	// setup broadcast adress to BROADCASTADDR entry
	clientaddress[BROADCASTADDR].sa_family = AF_INET;
	clientaddress[BROADCASTADDR].ip.sin_port = htons(sock_port_local);
	clientaddress[BROADCASTADDR].ip.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	doomcom->extratics = 1; // internet is very high ping

	SOCK_cmpaddr = UDP_cmpaddr;
	return s;
}
#endif

#ifdef USEIPX
static SOCKET_TYPE IPX_Socket(void)
{
	SOCKET_TYPE s = BADSOCKET;
	SOCKADDR_IPX address;
#ifdef WATTCP
	char trueval = true;
#else
	unsigned long trueval = true;
#endif
	int i;
	socklen_t j;

	// allocate a socket
	s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
	if (s == (SOCKET_TYPE)ERRSOCKET || s == BADSOCKET)
		I_Error("IPX_socket error #%u: Can't create socket: %s", errno, strerror(errno));

	memset(&address, 0, sizeof (address));
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON) && !defined (__CYGWIN__)
	address.sipx_family = AF_IPX;
	address.sipx_port = htons(sock_port);
#else
	address.sa_family = AF_IPX;
	address.sa_socket = htons(sock_port);
#endif // UNIXCOMMON
	if (bind(s, (void *)&address, sizeof (address)) == ERRSOCKET)
		I_Error("IPX_Bind error #%u: %s", errno, strerror(errno));

	// make it non blocking
	ioctl(s, FIONBIO, &trueval);

	// make it broadcastable
	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, sizeof (trueval));

	// set receive buffer to 64Kb
	j = sizeof (i);
	getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, &j);
	CONS_Printf("Network system receive buffer: %dKb\n",i>>10);
	if (i < 128<<10)
	{
		i = 64<<10;
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof (i)))
		{
			CONS_Printf("Network system receive buffer set to: %dKb\n",i>>10);
			CONS_Printf("Can't set receive buffer length to 64k, file transfer will be bad\n");
		}
		else
		{
			CONS_Printf("Network system receive buffer set to: %dKb\n",i>>10);
		}
	}

	// ipx header
	packetheaderlength = 30; // for stats

	// setup broadcast adress to BROADCASTADDR entry
#if defined (__unix__) || defined(__APPLE__) || defined (UNIXCOMMON) && !defined (__CYGWIN__)
	clientaddress[BROADCASTADDR].sa_family = AF_IPX;
	clientaddress[BROADCASTADDR].ipx.sipx_port = htons(sock_port);
#ifndef FREEBSD
	clientaddress[BROADCASTADDR].ipx.sipx_network = 0;
	for (i = 0; i < 6; i++)
		clientaddress[BROADCASTADDR].ipx.sipx_node[i] = (UINT8)0xFF;
#else
	clientaddress[BROADCASTADDR].ipx.sipx_addr.x_net.s_net[0] = 0;
	clientaddress[BROADCASTADDR].ipx.sipx_addr.x_net.s_net[1] = 0;
	for (i = 0; i < 6; i++)
		clientaddress[BROADCASTADDR].ipx.sipx_addr.x_host.c_host[i] = (UINT8)0xFF;
#endif
#else
	clientaddress[BROADCASTADDR].sa_family = AF_IPX;
	clientaddress[BROADCASTADDR].ipx.sa_socket = htons(sock_port);
	for (i = 0; i < 4; i++)
		clientaddress[BROADCASTADDR].ipx.sa_netnum[i] = 0;
	for (i = 0; i < 6; i++)
		clientaddress[BROADCASTADDR].ipx.sa_nodenum[i] = (UINT8)0xFF;
#endif // UNIXCOMMON
	SOCK_cmpaddr = IPX_cmpaddr;
	return s;
}
#endif // USEIPX

boolean I_InitTcpDriver(void)
{
	boolean tcp_was_up = init_tcp_driver;
#ifndef NONET
	if (!init_tcp_driver)
	{
#ifdef _WIN32
		WSADATA WSAData;
		const int WSAresult = WSAStartup(MAKEWORD(1,1),&WSAData);
		if (WSAresult != 0)
		{
			LPCSTR WSError = NULL;
			switch (WSAresult)
			{
				case WSASYSNOTREADY:
					WSError = "The underlying network subsystem is not ready for network communication";
					break;
				case WSAEINPROGRESS:
					WSError = "A blocking Windows Sockets 1.1 operation is in progress";
					break;
				case WSAEPROCLIM:
					WSError = "Limit on the number of tasks supported by the Windows Sockets implementation has been reached";
					break;
				case WSAEFAULT:
					WSError = "WTF? The WSAData is not a valid pointer? What kind of setup do you have?";
					break;
				default:
					WSError = va("Error code %u",WSAresult);
					break;
			}
			if (WSAresult != WSAVERNOTSUPPORTED) CONS_Printf("WinSock(TCP/IP) error: %s\n",WSError);
		}
		if (LOBYTE(WSAData.wVersion) != 1 ||
			HIBYTE(WSAData.wVersion) != 1)
		{
			WSACleanup();
			CONS_Printf("No WinSock(TCP/IP) 1.1 driver detected");
		}
		CONS_Printf("WinSock description: %s\n",WSAData.szDescription);
		CONS_Printf("WinSock System Status: %s\n",WSAData.szSystemStatus);
#endif
#ifdef HAVE_LWIP
		lwip_kos_init();
#elif defined(_arch_dreamcast)
		//return;
		net_init();
#endif
#ifdef __DJGPP__
#ifdef WATTCP // Alam_GBC: survive bootp, dhcp, rarp and wattcp/pktdrv from failing to load
		survive_eth   = 1; // would be needed to not exit if pkt_eth_init() fails
		survive_bootp = 1; // ditto for BOOTP
		survive_dhcp  = 1; // ditto for DHCP/RARP
		survive_rarp  = 1;
		//_watt_do_exit = false;
		//_watt_handle_cbreak = false;
		//_watt_no_config = true;
		_outch = wattcp_outch;
		init_misc();
//#ifdef DEBUGFILE
		dbug_init();
//#endif
		switch (sock_init())
		{
			case 0:
				init_tcp_driver = true;
				break;
			case 3:
				CONS_Printf("No packet driver detected");
				break;
			case 4:
				CONS_Printf("Error while talking to packet driver");
				break;
			case 5:
				CONS_Printf("BOOTP failed");
				break;
			case 6:
				CONS_Printf("DHCP failed");
				break;
			case 7:
				CONS_Printf("RARP failed");
				break;
			case 8:
				CONS_Printf("TCP/IP failed");
				break;
			case 9:
				CONS_Printf("PPPoE login/discovery failed");
				break;
			default:
				CONS_Printf("Unknown error with TCP/IP stack");
				break;
		}
		hires_timer(0);
#else // wattcp
		if (__lsck_init())
			init_tcp_driver = true;
		else
			CONS_Printf("No Tcp/Ip driver detected");
#endif // libsocket
#endif // __DJGPP__
#ifndef __DJGPP__
		init_tcp_driver = true;
#endif
	}
#endif
	if (!tcp_was_up && init_tcp_driver)
		I_AddExitFunc(I_ShutdownTcpDriver);
	return init_tcp_driver;
}

#ifndef NONET
static void SOCK_CloseSocket(void)
{
	if (mysocket != (SOCKET_TYPE)ERRSOCKET && mysocket != BADSOCKET)
	{
// quick fix bug in libsocket 0.7.4 beta 4 under winsock 1.1 (win95)
#if !defined (__DJGPP__) || defined (WATTCP)
		FD_CLR(mysocket, &set);
		close(mysocket);
#endif
	}
	mysocket = BADSOCKET;
}
#endif

void I_ShutdownTcpDriver(void)
{
#ifndef NONET
	SOCK_CloseSocket();

	CONS_Printf("I_ShutdownTcpDriver: ");
#ifdef _WIN32
	WSACleanup();
#endif
#ifdef HAVE_LWIP
	lwip_kos_shutdown();
#elif defined(_arch_dreamcast)
	net_shutdown();
#endif
#ifdef __DJGPP__
#ifdef WATTCP // wattcp
	//_outch = NULL;
	sock_exit();
#else
	__lsck_uninit();
#endif // libsocket
#endif // __DJGPP__
	CONS_Printf("shut down\n");
	init_tcp_driver = false;
#endif
}

#ifndef NONET
static SINT8 SOCK_NetMakeNode(const char *hostname)
{
	SINT8 newnode;
	char *localhostname = strdup(hostname);
	char *portchar;
	UINT16 portnum = htons(sock_port);

	DEBFILE(va("Creating new node: %s\n", hostname));

	// retrieve portnum from address!
	strtok(localhostname, ":");
	portchar = strtok(NULL, ":");
	if (portchar)
		portnum = htons((UINT16)atoi(portchar));
	free(localhostname);

	// server address only in ip
#ifdef USEIPX
	if (ipx) // ipx only
		return BROADCASTADDR;
	else // tcp/ip
#endif
	{
		struct hostent *hostentry; // host information entry
		char *t;

		// remove the port in the hostname as we've it already
		t = localhostname = strdup(hostname);
		while ((*t != ':') && (*t != '\0'))
			t++;
		*t = '\0';

		cleanupnodes();
		newnode = getfreenode();
		if (newnode == -1)
		{
			free(localhostname);
			return -1;
		}
		// find ip of the server
		clientaddress[newnode].sa_family = AF_INET;
		clientaddress[newnode].ip.sin_port = portnum;
		clientaddress[newnode].ip.sin_addr.s_addr = inet_addr(localhostname);

		if (clientaddress[newnode].ip.sin_addr.s_addr == htonl(INADDR_NONE)) // not a ip ask to the dns
		{
			CONS_Printf("Resolving %s\n",localhostname);
			hostentry = gethostbyname(localhostname);
			if (!hostentry)
			{
				CONS_Printf("%s unknown\n", localhostname);
				I_NetFreeNodenum(newnode);
				free(localhostname);
				return -1;
			}
			clientaddress[newnode].ip.sin_addr.s_addr = *((UINT32 *)hostentry->h_addr_list[0]);

			CONS_Printf("Resolved %s\n", SOCK_GetNodeAddress(newnode));
		}

		free(localhostname);

		return newnode;
	}
}
#endif

static boolean SOCK_OpenSocket(void)
{
#ifndef NONET
	size_t i;

	memset(clientaddress, 0, sizeof (clientaddress));

	for (i = 0; i < MAXNETNODES; i++)
		nodeconnected[i] = false;

	nodeconnected[0] = true; // always connected to self
	nodeconnected[BROADCASTADDR] = true;
	I_NetSend = SOCK_Send;
	I_NetGet = SOCK_Get;
	I_NetCloseSocket = SOCK_CloseSocket;
	I_NetFreeNodenum = SOCK_FreeNodenum;
	I_NetMakeNode = SOCK_NetMakeNode;

#ifdef SELECTTEST
	// seem like not work with libsocket : (
	I_NetCanSend = SOCK_CanSend;
	I_NetCanGet = SOCK_CanGet;
#endif

	// build the socket but close it first
	SOCK_CloseSocket();
#ifdef USEIPX
	if (ipx)
	{
		mysocket = IPX_Socket();
		net_bandwidth = 800000;
		hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
	}
	else
#endif // USEIPX
		mysocket = UDP_Socket();
	// for select
	FD_ZERO(&set);
	FD_SET(mysocket,&set);
#endif
	return (boolean)(mysocket != (SOCKET_TYPE)ERRSOCKET && mysocket != BADSOCKET);
}

static boolean SOCK_Ban(INT32 node)
{
#ifdef NONET
	(void)node;
	return false;
#else
	if (numbans == MAXBANS)
		return false;

	bannedmask[numbans] = 32;
	M_Memcpy(&banned[numbans], &clientaddress[node], sizeof (mysockaddr_t));
	if (banned[numbans].sa_family == AF_INET)
		banned[numbans].ip.sin_port = 0;
	numbans++;
	return true;
#endif
}

static boolean SOCK_Shun(INT32 node)
{
	if (node > MAXNETNODES)
		return false;
#ifdef NONET
	return false;
#else
	if (numshun == MAXBANS)
		return false;

	shunnedmask[numshun] = 32;
	M_Memcpy(&shunned[numshun], &clientaddress[node], sizeof (mysockaddr_t));
	if (shunned[numshun].sa_family == AF_INET)
		shunned[numshun].ip.sin_port = 0;
	numshun++;
	return true;
#endif
}

static boolean SOCK_SetBanAddress(const char *address, const char *mask)
{
#ifdef NONET
	(void)address;
	(void)mask;
	return false;
#else
	if (numbans == MAXBANS || !address)
		return false;

	banned[numbans].ip.sin_addr.s_addr = inet_addr(address);

	if (banned[numbans].ip.sin_addr.s_addr == htonl(INADDR_NONE))
		return false;
	else
		banned[numbans].ip.sin_port = 0;

	banned[numbans].sa_family = AF_INET;

	if (mask)
		bannedmask[numbans] = (UINT8)atoi(mask);
	else
		bannedmask[numbans] = 32;

	if (bannedmask[numbans] > 32)
		bannedmask[numbans] = 32;

	numbans++;

	return true;
#endif
}

static void SOCK_ClearBans(void)
{
	numbans = 0;
}

boolean I_InitTcpNetwork(void)
{
	char serverhostname[255];
	boolean ret = false;
#ifdef USEIPX
	ipx = M_CheckParm("-ipx");
#endif
	// initilize the OS's TCP/IP stack
	if (!I_InitTcpDriver())
		return false;

	if (M_CheckParm("-udpport"))
	{
		if (M_IsNextParm())
			sock_port = (UINT16)atoi(M_GetNextParm());
		else
			sock_port = 0;
	}

	// parse network game options,
	if (M_CheckParm("-server") || dedicated)
	{
		server = true;

		// If a number of clients (i.e. nodes) is specified, the server will wait for the clients
		// to connect before starting.
		// If no number is specified here, the server starts with 1 client, and others can join
		// in-game.
		// Since Boris has implemented join in-game, there is no actual need for specifying a
		// particular number here.
		// FIXME: for dedicated server, numnodes needs to be set to 0 upon start
		if (dedicated)
			doomcom->numnodes = 0;
/*		else if (M_IsNextParm())
			doomcom->numnodes = (INT16)atoi(M_GetNextParm());*/
		else
			doomcom->numnodes = 1;

		if (doomcom->numnodes < 0)
			doomcom->numnodes = 0;
		if (doomcom->numnodes > MAXNETNODES)
			doomcom->numnodes = MAXNETNODES;

		// server
		servernode = 0;
		// FIXME:
		// ??? and now ?
		// server on a big modem ??? 4*isdn
		net_bandwidth = 16000;
		hardware_MAXPACKETLENGTH = INETPACKETLENGTH;

		ret = true;
	}
	else if (M_CheckParm("-connect"))
	{
		if (M_IsNextParm())
			strcpy(serverhostname, M_GetNextParm());
		else
			serverhostname[0] = 0; // assuming server in the LAN, use broadcast to detect it

		// server address only in ip
		if (serverhostname[0] && !ipx)
		{
			COM_BufAddText("connect \"");
			COM_BufAddText(serverhostname);
			COM_BufAddText("\"\n");

			// probably modem
			hardware_MAXPACKETLENGTH = INETPACKETLENGTH;
		}
		else
		{
			// so we're on a LAN
			COM_BufAddText("connect any\n");

			net_bandwidth = 800000;
			hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
		}
	}

	I_NetOpenSocket = SOCK_OpenSocket;
	I_Ban = SOCK_Ban;
	I_Shun = SOCK_Shun;
	I_ClearBans = SOCK_ClearBans;
	I_GetNodeAddress = SOCK_GetNodeAddress;
	I_GetBanAddress = SOCK_GetBanAddress;
	I_GetBanMask = SOCK_GetBanMask;
	I_SetBanAddress = SOCK_SetBanAddress;
	bannednode = SOCK_bannednode;

	return ret;
}
