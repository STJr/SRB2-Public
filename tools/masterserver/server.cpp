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

#ifdef __GNUC__
#include <unistd.h>
#endif
#include <typeinfo>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif
#include "ipcs.h"
#include "common.h"
#include "srvlist.h"
#include "stats.h"

//=============================================================================

#ifdef __GNUC__
#define ATTRPACK __attribute__ ((packed))
#else
#define ATTRPACK
#endif

#define PT_ASKINFOVIAMS 15

static CServerList servers_list;
static CServerSocket server_socket;
static CServerStats server_stats;

FILE *logfile = NULL;

#if defined(_MSC_VER)
#pragma pack(1)
#endif

typedef struct
{
	char ip[16];			// Big enough to hold a full address.
	unsigned short port;
	unsigned char padding1[2];
	unsigned long time;
} ATTRPACK ms_holepunch_packet_t;

typedef struct
{
	char clientaddr[22];
	unsigned char padding1[2];
	unsigned long time;
} ATTRPACK msaskinfo_pak;

//
// SRB2 network packet data.
//
typedef struct
{
	unsigned checksum;
	unsigned char ack; // if not null the node asks for acknowledgement, the receiver must resend the ack
	unsigned char ackreturn; // the return of the ack number

	unsigned char packettype;
	unsigned char reserved; // padding

	msaskinfo_pak msaskinfo;
} ATTRPACK doomdata_t;

#if defined(_MSC_VER)
#pragma pack()
#endif

static unsigned int dummykeys[] = { 0xdeadcafe, 0xbaadf00d };

static struct { char *version; unsigned int *keys; int numkeys; } secureversions[] =
{
	{"dummy",	dummykeys,	NUM_ELEMENTS(dummykeys)}
};

//=============================================================================

/*
** checkPassword()
*/
static int checkPassword(char *pw)
{
	const char *cpw;

	pw[15] = '\0'; // for security reason
	cpw = pCrypt(pw, "04");
	memset(pw, 0, 16); // erase that ASAP!
	if (strcmp(cpw, "04f5jSy71x6s."))
	{
		logPrintf(logfile, "Bad password\n");
		return 0;
	}
	return 1;
}

/*
** sendServersInformations()
*/
static void sendServersInformations(int id)
{
	msg_t msg;
	int writecode;
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	logPrintf(logfile, "Sending servers informations\n");
	msg.id = id;
	msg.type = SEND_SERVER_MSG;
	while (p)
	{
		const char *str = p->getString();

		msg.length = strlen(str)+1; // send also the '\0'
		strcpy(msg.buffer, str);
		dbgPrintf(CYAN, "Writing: (%d)\n%s\n", msg.length, msg.buffer);
		writecode = server_socket.write(&msg);
		if (writecode < 0)
		{
			dbgPrintf(LIGHTRED, "Write error... %d client %d deleted\n", writecode, id);
			return;
		}
		p = (CServerItem *)servers_list.getNext();
	}
	msg.length = 0;
	//dbgPrintf(CYAN, "Writing: (%d) %s\n", msg.length, "");
	writecode = server_socket.write(&msg);
	if (writecode < 0)
	{
		dbgPrintf(LIGHTRED, "Write error... %d client %d deleted\n",
			writecode, id);
	}
	server_stats.num_retrieval++;
}

/*
** sendShortServersInformations()
*/
static void sendShortServersInformations(int id)
{
	msg_t msg;
	int writecode;
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	logPrintf(logfile, "Sending short servers informations\n");
	msg.id = id;
	msg.type = SEND_SHORT_SERVER_MSG;
	while (p)
	{
		msg_server_t *info = (msg_server_t *) msg.buffer;

		info->header.buffer[0] = '\0'; // nothing interresting in it (for now)
		strcpy(info->ip,      p->getIP());
		strcpy(info->port,    p->getPort());
		strcpy(info->name,    p->getName());
		strcpy(info->version, p->getVersion());

		msg.length = sizeof (msg_server_t);
		writecode = server_socket.write(&msg);
		if (writecode < 0)
		{
			dbgPrintf(LIGHTRED, "Write error... %d client %d "
				"deleted\n", writecode, id);
			return;
		}
		p = (CServerItem *)servers_list.getNext();
	}
	msg.length = 0;
	writecode = server_socket.write(&msg);
	if (writecode < 0)
	{
		dbgPrintf(LIGHTRED, "Write error... %d client %d deleted\n",
			writecode, id);
	}
	server_stats.num_retrieval++;
}

/*
** addServer()
*/
static void addServer(int id, char *buffer)
{
	msg_server_t *info;
	char refuse = 0;

	//TODO: Be sure there is no flood from a given IP:
	//      If a host need more than 2 servers, then it should be registrated
	//      manually

	info = (msg_server_t *)buffer;

	// I want to be sure the informations are correct, of course!
	info->port[sizeof (info->port)-1] = '\0';
	info->name[sizeof (info->name)-1] = '\0';
	info->version[sizeof (info->version)-1] = '\0';
	// retrieve the true ip of the server
	strcpy(info->ip, server_socket.getClientIP(id));
	//strcpy(info->port, server_socket.getClientPort(id));

	if (info->version[0] == '1' &&
		info->version[1] == '.' &&
		info->version[2] == '0' &&
		info->version[3] == '9' &&
		info->version[4] == '.')
	{
		if ((info->version[5] == '2') || (info->version[5] == '3'))
		{
			refuse = 1;
		}
	}

	if (info->version[0] == '1' &&
		info->version[1] == '.' &&
		info->version[2] == '6' &&
		info->version[3] == '9' &&
		info->version[4] == '.' &&
		info->version[5] == '6')
	{
		refuse = 1;
	}

	// Validate certain versions.
	for(unsigned int i = 0; i < NUM_ELEMENTS(secureversions); i++)
	{
		if(strcmp(info->version, secureversions[i].version) == 0)
		{
			refuse = 1;

			for(int j = 0; j < secureversions[i].numkeys; j++)
			{
				unsigned int signature = 0;
				for(const char *insname = info->name; *insname; signature += *insname++);
				signature *= signature * (unsigned int)secureversions[i].keys[j];
				signature &= 0xAAAAAAAA;

				if(info->header.signature == signature)
				{
					refuse = 0;
					break;
				}
			}

			break;
		}
	}

	if (!refuse)
	{
		logPrintf(logfile, "Adding the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
		servers_list.remove(info->ip, info->port, info->name, info->version, ST_TEMPORARY);
		servers_list.insert(info->ip, info->port, info->name, info->version, ST_TEMPORARY);
		server_stats.num_servers++;
		server_stats.num_add++;
		server_stats.putLastServer(info);
	}
	else
	{
		logPrintf(logfile, "Not adding the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
	}
}

/*
** addServerv2()
*/
static void addServerv2(int id, char *buffer)
{
	msg_server_t *info;
	char Handshakepassed = 0;

	//TODO: Be sure there is no flood from a given IP:
	//      If a host need more than 2 servers, then it should be registrated
	//      manually

	info = (msg_server_t *)buffer;

	// I want to be sure the informations are correct, of course!
	info->port[sizeof (info->port)-1] = '\0';
	info->name[sizeof (info->name)-1] = '\0';
	info->version[sizeof (info->version)-1] = '\0';
	// retrieve the true ip of the server
	strcpy(info->ip, server_socket.getClientIP(id));
	//strcpy(info->port, server_socket.getClientPort(id));

	//blah blah blah http://orospakr.is-a-geek.org/projects/srb2/ticket/164 etc etc

	if (Handshakepassed)
	{
		logPrintf(logfile, "Adding the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
		servers_list.insert(info->ip, info->port, info->name, info->version, ST_TEMPORARY);
		server_stats.num_servers++;
		server_stats.num_add++;
		server_stats.putLastServer(info);
	}
}

/*
** addPermanentServer()
*/
static void addPermanentServer(char *buffer)
{
	msg_server_t *info;

	info = (msg_server_t *)buffer;

	// I want to be sure the informations are correct, of course!
	info->ip[sizeof (info->ip)-1] = '\0';
	info->port[sizeof (info->port)-1] = '\0';
	info->name[sizeof (info->name)-1] = '\0';
	info->version[sizeof (info->version)-1] = '\0';

	if (!checkPassword(info->header.buffer))
		return;

	logPrintf(logfile, "Adding the permanent server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
	servers_list.insert(info->ip, info->port, info->name, info->version, ST_PERMANENT);
	//servers_list.insert(new CServerItem(info->ip, info->port, info->name, info->version, ST_PERMANENT));
	server_stats.num_servers++;
	server_stats.num_add++;
	server_stats.putLastServer(info);
}

/*
** removeServer()
*/
static void removeServer(int id, char *buffer)
{
	msg_server_t *info;

	info = (msg_server_t *)buffer;

	// I want to be sure the informations are correct, of course!
	info->port[sizeof (info->port)-1] = '\0';
	info->name[sizeof (info->name)-1] = '\0';
	info->version[sizeof (info->version)-1] = '\0';

	// retrieve the true ip of the server
	strcpy(info->ip, server_socket.getClientIP(id));

	logPrintf(logfile, "Removing the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
	if (servers_list.remove(info->ip, info->port, info->name, info->version, ST_TEMPORARY))
		server_stats.num_servers--;
	else
		server_stats.num_badconnection++;
	server_stats.num_removal++;
}

/*
** removePermanentServer()
*/
static void removePermanentServer(char *buffer)
{
	msg_server_t *info;

	info = (msg_server_t *)buffer;

	// I want to be sure the informations are correct, of course!
	info->ip[sizeof (info->ip)-1] = '\0';
	info->port[sizeof (info->port)-1] = '\0';
	info->name[sizeof (info->name)-1] = '\0';
	info->version[sizeof (info->version)-1] = '\0';

	if (!checkPassword(info->header.buffer))
		return;

	logPrintf(logfile, "Removing the pemanent server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
	if (servers_list.remove(info->ip, info->port, info->name, info->version, ST_PERMANENT))
		server_stats.num_servers--;
	server_stats.num_removal++;
}

/*
** sendFile()
*/
static void sendFile(int id, char *buffer, FILE *f)
{
	msg_t msg;
	int count;
	int writecode;
	msg_server_t *info;

	info = (msg_server_t *)buffer;
	if (!checkPassword(info->header.buffer))
		return;

	logPrintf(logfile, "Sending file\n");
	msg.id = id;
	msg.type = SEND_FILE_MSG;
	fseek(f, 0, SEEK_SET);
	while ((count = fread(msg.buffer, 1, PACKET_SIZE, f)) > 0)
	{
		msg.length = count;
		if (count != PACKET_SIZE) // send a null terminated string
		{
			msg.length++;
			msg.buffer[count] = '\0';
		}
		writecode = server_socket.write(&msg);
		if (writecode < 0)
		{
			dbgPrintf(LIGHTRED, "Write error... %d client %d "
				"deleted\n", writecode, id);
			return;
		}
	}
	msg.length = 0;
	//dbgPrintf(CYAN, "Writing: (%d) %s\n", msg.length, "");
	writecode = server_socket.write(&msg);
	if (writecode < 0)
	{
		dbgPrintf(LIGHTRED, "Write error... %d client %d deleted\n",
			writecode, id);
	}
}

/*
** sendHttpLine()
*/
static void sendHttpLine(int id, const char *lpFmt, ...)
{
	msg_t msg;
	int len;
	int writecode;
	va_list arglist;

	va_start(arglist, lpFmt);
	len = vsnprintf(msg.buffer, sizeof msg.buffer, lpFmt, arglist);
	va_end(arglist);
	if (len <= 0)
		return;
	strcat(msg.buffer, "\n");

	msg.id = id;
	msg.type = SEND_HTTP_REQUEST_MSG;
	msg.length = len+1;

	writecode = server_socket.write(&msg);
	if (writecode < 0)
	{
		dbgPrintf(LIGHTRED, "Write error... %d client %d deleted\n",
			writecode, id);
		return;
	}
}

/*
** sendHttpServersList()
*/
static void sendHttpServersList(int id)
{
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	sendHttpLine(id, "<FONT COLOR=\"#00FF00\"><CODE>");
	if (!p)
	{
		sendHttpLine(id, "<A NAME=\"%23s\">No server connected.</A>",
			"00000000000000000000000");
	}
	else while (p)
	{
		sendHttpLine(id,
"<A NAME=\"%23s\">IP: %15s    Port: %5s    Name: %-31s    Version: %7s </A>",
p->getGuid(),p->getIP(), p->getPort(), p->getName(), p->getVersion());
		p = (CServerItem *)servers_list.getNext();
	}
	sendHttpLine(id, "</CODE></FONT>");
}

static void sendtextServersList(int id)
{
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	if (!p)
	{
		sendHttpLine(id, "No SRB2 Games are running, "
			"so why don't you start one?");
	}
	else while (p)
	{
		sendHttpLine(id,
			"IP: %15s | Port: %5s | Name: %s | Version: %7s",
			p->getIP(), p->getPort(), p->getName(),
			p->getVersion());
		p = (CServerItem *)servers_list.getNext();
	}
}

static void sendRSS10ServersList(int id)
{
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	sendHttpLine(id, "<items> <rdf:Seq>");
	if (!p)
	{
		sendHttpLine(id, "<rdf:li rdf:resource=\"http://ms.srb2.org/#00000000000000000000000\" />");
	}
	else while (p)
	{
		sendHttpLine(id, "<rdf:li rdf:resource=\"http://ms.srb2.org/#%23s\" />",p->getGuid());
		p = (CServerItem *)servers_list.getNext();
	}

	sendHttpLine(id, "</rdf:Seq> </items> </channel>");

	p = (CServerItem *)servers_list.getFirst();

	if (!p)
	{
		sendHttpLine(id, "<item rdf:about=\"http://ms.srb2.org/#00000000000000000000000\"><title>No servers</title></item>");
	}
	else while (p)
	{
		sendHttpLine(id, "<item rdf:about=\"http://ms.srb2.org/#%23s\"><title>%s</title><link>http://ms.srb2.org/#%23s</link><srb2ms:address>%s</srb2ms:address><srb2ms:port>%5s</srb2ms:port><srb2ms:version>%s</srb2ms:version><dc:date>%24s</dc:date></item>",
			p->getGuid(), p->getName(), p->getGuid(), p->getIP(), p->getPort(), p->getVersion(), p->getRegtime(), p->getGuid());
		p = (CServerItem *)servers_list.getNext();
	}
}

static void sendRSS92ServersList(int id)
{
	CServerItem *p = (CServerItem *)servers_list.getFirst();

	if (!p)
	{
		sendHttpLine(id, "<item><title>No servers</title><description>I'm sorry, but no servers are running now</description></item>");
	}
	else while (p)
	{
		sendHttpLine(id, "<item><title>%s</title><description>IP: %s | Port: %5s | Version: %s</description></item>", p->getName(), p->getIP(), p->getPort(), p->getVersion());
		p = (CServerItem *)servers_list.getNext();
	}
}

static void sendMOTDText(int id)
{
	sendHttpLine(id, SERVERMOTDTEXT);
}

/*
** sendHttpRequest()
*/
static void sendHttpRequest(int id)
{
	server_stats.num_http_con++;
	logPrintf(logfile, "Sending http request\n");
	// Status
	sendHttpLine(id, "<P>Server up and running since: %s (%d days %d hours)<BR>",
		server_stats.getUptime(), server_stats.getDays(), server_stats.getHours());
	sendHttpLine(id, "Total number of connections: %d (%d HTTP requests)<BR>",
		server_stats.num_connections, server_stats.num_http_con);
	sendHttpLine(id, "Number of bad connections: %d<BR>",
		server_stats.num_badconnection);
	sendHttpLine(id, "Current number of servers: %d<BR></P>",
		server_stats.num_servers);
	// Motd
	sendHttpLine(id, "<H3>Message of the day</H3><P>%s</P>",
		server_stats.getMotd());
	// Usage
	sendHttpLine(id, "<H3>Usage</H3>");
	sendHttpLine(id, "<P>Number of server adds: %d<BR>",
		server_stats.num_add);
	sendHttpLine(id, "Number of server removals: %d<BR>",
		server_stats.num_removal);
	sendHttpLine(id, "Number of auto removals: %d<BR>",
		server_stats.num_autoremoval);
	sendHttpLine(id, "Number of list retrievals: %d<BR></P>",
		server_stats.num_retrieval);
	// Servers' list
	sendHttpLine(id, "<H3>Servers' list</H3><PRE>");
	sendHttpServersList(id);
	// Last server registered
	sendHttpLine(id, "<H3>Last server registered</H3>");
	sendHttpLine(id, "<P>%s</PRE>",
		server_stats.getLastServers());
	// Version
	sendHttpLine(id, "<H3>Version</H3>");
	sendHttpLine(id, "<P>Build date/time: %s</P>",
		server_stats.getVersion());

	server_socket.deleteClient(id); // close connection with the script
}

static void sendtextRequest(int id)
{
	server_stats.num_text_con++;
	logPrintf(logfile, "Sending text request\n");
	sendtextServersList(id);
	server_socket.deleteClient(id); // close connection with the script
}

static void sendRSS92Request(int id)
{
	server_stats.num_RSS92_con++;
	logPrintf(logfile, "Sending RSS .92 feed items request\n");
	sendRSS92ServersList(id);
	server_socket.deleteClient(id); // close connection with the script
}

static void sendRSS10Request(int id)
{
	server_stats.num_RSS10_con++;
	logPrintf(logfile, "Sending RSS 1.0 feed items request\n");
	sendRSS10ServersList(id);
	server_socket.deleteClient(id); // close connection with the script
}

static void sendMOTDRequest(int id)
{
	logPrintf(logfile, "Sending MOTD request\n");
	sendMOTDText(id);
	server_socket.deleteClient(id); // close connection with the script
}

/*
** eraseLogFile()
*/
static void eraseLogFile(char *buffer)
{
	msg_server_t *info;

	info = (msg_server_t *)buffer;
	if (!checkPassword(info->header.buffer))
		return;

#ifdef HAVE_SYSLOG
	logPrintf(NULL, "Reopening syslog socket\n"); // well, quite useless!
	closelog();
	openlog("SRB2MS", LOG_PID, LOG_USER);
#else
	logPrintf(logfile, "Erasing file\n"); // well, quite useless!
	if (logfile)
		fclose(logfile);
	logfile = fopen("server.log", "w+t");
#endif
}


/*
** CheckHeartBeats(): check whether a heartbeat was sent by the servers within
**                    within the timeout interval. If not, remove them.
*/
static void CheckHeartBeats()
{

	static time_t lastHeartBeatcheck = 0;
	const time_t HB_timeout = 
#ifdef UDP_HEARTBEAT
		20;
#else
		5 * 60;
#endif
	time_t cur_time = time(NULL);


	if (cur_time - lastHeartBeatcheck > HB_timeout/2) // it's high time to check the servers
	{
		CServerItem *p = (CServerItem *)servers_list.getFirst();
		while (p)
		{
			if (lastHeartBeatcheck - (p->HeartBeat) > HB_timeout // do not allow a ping higher than HB_timeout
				&& p->type != ST_PERMANENT)
			{
				CServerItem *q = p;
				p = (CServerItem *)servers_list.getNext();
				logPrintf(logfile, "Autoremoving %s %s %s %s\n", q->getIP(), q->getPort(), q->getName(), q->getVersion());
				if (servers_list.remove(q))
				{
					server_stats.num_servers--;
					server_stats.num_autoremoval++;
				}
			}
			else
				p = (CServerItem *)servers_list.getNext();
		}
		lastHeartBeatcheck = cur_time;
	}
}

#ifdef UDP_HEARTBEAT
/*
** updateServerHB()
*/
static void updateServerHB()
{
	CServerItem *p = (CServerItem *)servers_list.getFirst();
	const char *fromIP = server_socket.getUdpIP();
	const char *fromPort = server_socket.getUdpPort(false);
	bool match = false;

	while (p && !match)
	{
		if (!strcmp(fromIP, p->getIP())
			&& !strcmp(fromPort, p->getPort()))
		{
			p->HeartBeat = time(NULL);
			match = true;
		}
		p = (CServerItem *)servers_list.getNext();
	}

	// ok, if the HeartBeat doesn't match both IP and port, check on
	// just IP, and update the port of that ServerItem, yes Mystic,
	// I'm talking about you and your bad NAT router
	p = (CServerItem *)servers_list.getFirst(); // go back to start of list
	while (p && !match)
	{
		if (strcmp(fromIP, p->getIP()) == 0)
		{
			if (p->setPort(fromPort))
			{
				p->HeartBeat = time(NULL);
				match = true;
			}
		}
		p = (CServerItem *)servers_list.getNext();
	}
}
#endif

/*
** analyseUDP()
*/
static int analyseUDP(long size, char *buffer)
{
	// this would be the part of reading the PT_SERVERINFO packet,
	// but i'm not about to backport that sloppy code
	(void)size;
	(void)buffer;
	return INVALID_MSG;
}


//
// SRB2 packet checksum
//
static unsigned SRB2Checksum(doomdata_t *srb2packet, int length)
{
	unsigned c = 0x1234567;
	const int l = length - 4;
	const unsigned char *buf = (unsigned char *)srb2packet + 4;
	int i;

	for (i = 0; i < l; i++, buf++)
		c += (*buf) * (i+1);

	return c;
}

/*
** forwardAskInfoRequest()
*/
static int forwardAskInfoRequest(ms_holepunch_packet_t *mshpp)
{
	doomdata_t srb2packet;
	const char *fromIP = server_socket.getUdpIP();
	const char *fromPort = server_socket.getUdpPort(false);

	// Secretarial stuff.
	srb2packet.ack = srb2packet.ackreturn = 0;
	srb2packet.packettype = PT_ASKINFOVIAMS;

	// Time for ping calculation.
	srb2packet.msaskinfo.time = mshpp->time;

	// Client's address.
	strcpy(srb2packet.msaskinfo.clientaddr, fromIP);
	strcat(srb2packet.msaskinfo.clientaddr, ":");
	strcat(srb2packet.msaskinfo.clientaddr, fromPort);

	// Calculate checksum for SRB2.
	srb2packet.checksum = SRB2Checksum(&srb2packet, sizeof(srb2packet));

	// Send the packet.
	server_socket.writeUDP((char*)&srb2packet, sizeof(srb2packet), mshpp->ip, mshpp->port);

	return 0;
}

/*
** processAskInfoRequest()
*/
static int processAskInfoRequest(ms_holepunch_packet_t *mshpp)
{
	// Before telling the server about the prospective new client, we need
	// to make sure that the server is actually on our list. This isn't
	// strictly necessary, but it's probably a good idea.

	CServerItem *p = (CServerItem *)servers_list.getFirst();

	while (p)
	{
		if (!strcmp(mshpp->ip, p->getIP())
			&& mshpp->port == atoi(p->getPort()))
		{
			// We have a match, so send a request to the server.
			forwardAskInfoRequest(mshpp);
			return 0;
		}
		p = (CServerItem *)servers_list.getNext();
	}

	dbgPrintf(LIGHTGREEN, "%s:%d isn't on the serverlist.\n",
		mshpp->ip, mshpp->port);
	return 0;	// Not necessarily an error: server might have shut down.
}

/*
** UDPMessage()
*/
static int UDPMessage(long size, char *buffer)
{
	if (size == 4)
	{
#ifdef UDP_HEARTBEAT
		updateServerHB();
#endif
		return 0;
	}
	else if (size == sizeof(ms_holepunch_packet_t))
	{
		return processAskInfoRequest((ms_holepunch_packet_t*)buffer);
	}
	else if (size > 58)
	{
		dbgPrintf(LIGHTGREEN, "Got a UDP %d byte long message\n", size);
		return analyseUDP(size, buffer);
	}
	else
		return INVALID_MSG;
}

/*
** analyseMessage()
*/
static int analyseMessage(msg_t *msg)
{
	switch (msg->type)
	{
	case UDP_RECV_MSG:
		return UDPMessage(msg->length, msg->buffer);
	case ACCEPT_MSG:
		server_stats.num_connections++;
		break;
	case ADD_SERVER_MSG:
		addServer(msg->id, msg->buffer);
		break;
	case ADD_SERVERv2_MSG:
		addServerv2(msg->id, msg->buffer);
		break;
	case REMOVE_SERVER_MSG:
		removeServer(msg->id, msg->buffer);
		break;
	case ADD_PSERVER_MSG:
		addPermanentServer(msg->buffer);
		break;
	case REMOVE_PSERVER_MSG:
		removePermanentServer(msg->buffer);
		break;
	case GET_LOGFILE_MSG:
		sendFile(msg->id, msg->buffer, logfile);
		break;
	case ERASE_LOGFILE_MSG:
		eraseLogFile(msg->buffer);
		break;
	case ADD_CLIENT_MSG: // New client (unsupported)
		// TODO: add him in the player list
		break;
	case GET_SERVER_MSG:
		sendServersInformations(msg->id);
		break;
	case GET_SHORT_SERVER_MSG:
		sendShortServersInformations(msg->id);
		break;
	case HTTP_REQUEST_MSG:
		sendHttpRequest(msg->id);
		break;
	case TEXT_REQUEST_MSG:
		sendtextRequest(msg->id);
		break;
	case RSS92_REQUEST_MSG:
		sendRSS92Request(msg->id);
		break;
	case RSS10_REQUEST_MSG:
		sendRSS10Request(msg->id);
		break;
	case GET_MOTD_MSG:
		sendMOTDRequest(msg->id);
		break;
	default:
		return INVALID_MSG;
	}
	return 0;
}

/*
** main()
*/
int main(int argc, char *argv[])
{
	msg_t msg;

	if (argc <= 1)
	{
		fprintf(stderr, "usage: %s port\n", argv[0]);
		exit(1);
	}

	if (server_socket.listen(argv[1]) < 0)
	{
		fprintf(stderr, "Error while initializing the server\n");
		exit(2);
	}

#ifdef HAVE_SYSLOG
	openlog("SRB2MS", LOG_PID, LOG_USER);
#else
	logfile = openFile("server.log");
#endif
#if !defined (__DEBUG__) && !defined (_WIN32)
	switch (fork())
	{
	case 0: break;  // child
	case -1: printf("Error while launching the server in background\n"); return -1;
	default: return 0; // parent: keep child in background
	}
#endif
	srand((unsigned)time(NULL)); // Alam: GUIDs
	for (;;)
	{
		memset(&msg, 0, sizeof (msg)); // remove previous message
		if (!server_socket.read(&msg))
		{
			// valid message: header message seems ok
			analyseMessage(&msg);
			//servers_list.show(); // for debug purpose
		}

		CheckHeartBeats();
	}

	/* NOTREACHED */
	return 0;
}
