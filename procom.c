/***************************************************************************
 * Copyright 2001-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * GNU GENERAL PUBLIC LICENSE
 * Show COPYING that is included sources packeage.
 *
 * Nobby Noboru Hirano <nobby@ntools.net>
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdarg.h>

#define	PRCOM

static char *unix_aname = NULL;


/*****************************************************************
Read a chunk of bytes from the socket.
Returns 0 for success.
******************************************************************/
size_t ProcRead(int sock, char *buf,size_t len)
{
	size_t n, l = 0;

	while (len) {
		n = recv(sock, &buf[l], len, MSG_WAITALL);
		if (n <= 0) return -1;
		len -= n;
		l += n;
	}
	return (l);
}

char *ProcGets(int sock, char *st, size_t len)
{
	ssize_t r;
	char *buf = st;

	while (--len) {
		r = recv(sock, buf, 1, MSG_WAITALL);
		if (r != 1) return(NULL);

		if (*buf == '\n') break;
		if (*buf != '\r') ++buf;   /* remove all CR then set LF */
	}
	*buf = '\0';

	return st;
}

size_t ProcPrintf(int sock, char *format, ...)
{
	va_list ap;
	char buf[8192];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	return send(sock, buf, strlen(buf), 0);
}


/* open UNIX server socket */
int ProcSsocket(char *sname)
{
	int sock;
	struct sockaddr_un adr;

	memset(&adr, 0, sizeof(adr));
	adr.sun_family = PF_UNIX;
	strcpy(adr.sun_path, sname);

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) return sock;

	if (bind(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0) {
		perror("UNIX Bind");
		close (sock);
		return -1;
	}
	if (listen(sock, 5) < 0) {
		perror("listen");
		close (sock);
		return -1;
	}
	unix_aname = sname;
	return sock;
}

/* open UNIX socket */
int ProcSocket(char *sname)
{
	int sock;
	struct sockaddr_un adr;

	memset(&adr, 0, sizeof(adr));
	adr.sun_family = PF_UNIX;
	strcpy(adr.sun_path, sname);

	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) return sock;

	if (connect(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0) {
		perror("UNIX connect");
		close (sock);
		return -1;
	}
	return sock;   
}

int ProcTerm(int sock)
{
	close(sock);
	unlink(unix_aname);
	unix_aname = NULL;
	return 0;
}
