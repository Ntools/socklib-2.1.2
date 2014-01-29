/***************************************************************************
 * Copyright 1996-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * Warning !!!
 * Distribute unfreely,
 * This program sources can be used authorized user only !!
 * exclude executable files, Under working following OS
 *
 *                          hp-ux   hp9000 Series 700
 *                          Linux   ELF 1.2.13 or later
 *                          Microsoft Windows95 Windows NT
 *                Will be
 *                          Solaris 2  SUN OS 4.x
 *
 *
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Nobby Noboru Hirano <noble@leo.bekkoame.or.jp>
 ***************************************************************************/
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

#include "enge.h"

/*****************************************************************
	Create a new client socket
	returns -1 on error
******************************************************************/
	SOCKET Socket(char *host, u_short clientPort)
{
	SOCKET sock;
	unsigned long inaddr;
	struct sockaddr_in adr;
	struct hostent *hp;
	int ern;
	static int init_flag = 1;
	WSADATA wsaData;
	WORD wVersionRequested;

	if(init_flag) {
		wVersionRequested = MAKEWORD( 2, 0 );
		if(WSAStartup( wVersionRequested, &wsaData ) != 0) {
			fprintf(stderr,"Winsock.DLL not suport !!\n");
			return 0;
		}
		init_flag = 0;
	}
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;

	inaddr = inet_addr(host);
	if (inaddr != INADDR_NONE) memcpy(&adr.sin_addr, &inaddr, sizeof(inaddr));
    else {
		hp = gethostbyname(host);
		if (hp == NULL) {
			ern = WSAGetLastError();
			fprintf(stderr, "winsock error: [%d] ", ern);
			WSACleanup();
			return 0;
		}
		memcpy(&adr.sin_addr, hp->h_addr, hp->h_length);
	}
	adr.sin_port = htons(clientPort);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		WSACleanup();
		return sock;
	}

	connect(sock, (struct sockaddr *)&adr, sizeof(struct sockaddr));
#if 0
	if (connect(sock, (struct sockaddr *)&adr, sizeof(struct sockaddr)) == SOCKET_ERROR) {
		perror("Socket");
		closesocket(sock);
		WSACleanup();
		return 0;
	}
#endif
	return sock;
}


/*****************************************************************
Write a chunk of bytes to the socket.
Returns 0 for success.
******************************************************************/
size_t SockWrite(SOCKET socket,char *buf, size_t len)
{
	size_t n, cnt = 0;

	while (len) {
		n = send(socket, buf, len, 0);
		if (n == SOCKET_ERROR) return -1;
		len -= n;
		buf += n;
		cnt += n;
	}
	return cnt;
}

/*****************************************************************
Read a chunk of bytes from the socket.
Returns 0 for success.
******************************************************************/
size_t SockRead(SOCKET socket, char *buf, size_t len)
{
	size_t n, cnt = 0;

	while (len > 0) {
		n = recv(socket, buf, len, 0);
		if (n <= 0) return -1;
		len -= n;
		buf += n;
		cnt += n;
	}
	return cnt;
}

/*****************************************************************
Get a string terminated by an '\n', delete any '\r' and the '\n'.
Pass it a valid socket, a buffer for the string, and
the length of the buffer (including the trailing \0)
returns 0 for success.
*****************************************************************/
char *SockGets(SOCKET socket, char *bf, size_t len)
{
	char *buf;

	buf = bf;
	while (--len) {
		if (recv(socket, buf, 1, 0) != 1) return NULL;
		if (*buf == '\n') break;
		if (*buf != '\r') buf++;	/* remove all CRs */
	}
	*buf = '\0';
	return buf;
}


/*****************************************************************
Send a nul terminated string to the socket, followed by a CR-LF.
Returns 0 for success.
*****************************************************************/
size_t SockPuts(SOCKET socket, char *buf)
{
    size_t rc, l;

	if((l = strlen(buf)) == 0) return 0;

	if ((rc = SockWrite(socket, buf, l)) <= 0) return rc;
	return SockWrite(socket, "\r\n", 2);
}


/*****************************************************************
Send formatted output to the socket, followed
by a CR-LF.
Returns 0 for success.
******************************************************************/
size_t SockPrintf(SOCKET socket,char *format, ...)
{
	va_list ap;
	char buf[8192];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	return SockWrite(socket, buf, strlen(buf));
}

/*****************************************************************
 Check socket for readability.  return 0 for not readable,
 !0 for readable.
******************************************************************/
int SockStatus(SOCKET socket, int ms, fd_set *fds)
{
	struct timeval timeout;

	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = (ms % 1000) * 1000;

	FD_ZERO(fds);
	FD_SET(socket, fds);
	return (select((int)socket+1, fds, NULL, NULL, &timeout));
}

int WSockStatus(SOCKET sock,int ms, fd_set *fds)
{
	struct timeval timeout;

	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = (ms % 1000) * 1000;

	FD_ZERO(fds);
	FD_SET(sock, fds);
	return select(sock+1, NULL, fds, NULL, &timeout);
}

/*****************************************************************
  make client socket.
  returns -1 on error
******************************************************************/
SOCKET act_rec(SOCKET sock, char **ip)
{
	size_t l;
	struct sockaddr_in adr;
	SOCKET netsock;
	static char client_IP[512];

	memset(&adr, 0, sizeof(adr));
	l = sizeof(adr);
	if (ip != NULL) *ip = NULL;

	if((netsock = accept(sock, (struct sockaddr *)&adr, &l)) >= 0) {
		strcpy(client_IP, inet_ntoa(adr.sin_addr));
		*ip = client_IP;
	}
	else {
		perror("Net accept error");
	}
	return (netsock);
}
