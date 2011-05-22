// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2011 by Sonic Team Jr.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//-----------------------------------------------------------------------------
/// \file
/// \brief getaddrinfo stub

#ifndef __I_ADDRINFO__
#define __I_ADDRINFO__

#ifdef __GNUG__
#pragma interface
#endif

#ifndef AI_PASSIVE
#define AI_PASSIVE     0x01
#endif
#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 0x04
#endif
#ifndef AI_V4MAPPED
#define AI_V4MAPPED    0x08
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG  0x20
#endif

#ifdef _WIN32
#ifndef EAI_NONAME
#define EAI_NONAME WSAHOST_NOT_FOUND
#endif
#endif

#ifndef EAI_NONAME
#define EAI_NONAME -2
#endif

#ifdef _PS3 // PSL1GHT v2
struct my_addrinfo {
	int                 ai_flags;
	int                 ai_family;
	int                 ai_socktype;
	int                 ai_protocol;
	size_t              ai_addrlen;
	struct sockaddr    *ai_addr;
	struct my_addrinfo *ai_next;
};
#elif defined (_WIN32) // already use the stub for Win32
// w32api, ws2tcpip.h, r1.12
struct my_addrinfo {
        int     ai_flags;
        int     ai_family;
        int     ai_socktype;
        int     ai_protocol;
        size_t  ai_addrlen;
        char   *ai_canonname;
        struct sockaddr  *ai_addr;
        struct my_addrinfo  *ai_next;
};
#else
#define my_addrinfo addrinfo
#endif

void WS_addrinfocleanup(void);

#ifndef my_addrinfo
void I_freeaddrinfo(struct my_addrinfo *res);
int I_getaddrinfo(const char *node, const char *service,
                         const struct my_addrinfo *hints,
                         struct my_addrinfo **res);
#elif !defined (test_stub)
#define I_getaddrinfo getaddrinfo
#define I_freeaddrinfo freeaddrinfo
#endif


#endif
