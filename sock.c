/***************************************************************************
 * Copyright 2000-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * Ntools BSD License
 * Version 1.0 March 2005
 *
 * Nobby Noboru Hirano <nobby@ntools.net>
 ***************************************************************************/
#ifndef __WIN32__
#   if defined(_WIN32) || defined(WIN32)
#       define __WIN32__
#   endif
#endif

#ifndef __WIN32__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <sys/wait.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>

#else

#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

typedef int socklen_t;

#endif /* WIN32 */

#define	NSocK_INPUT_READ	0x0001
#define	NSocK_INPUT_WRITE	0x0002

static int nsock_win32_init(void)
{
#ifdef __WIN32__
	WSADATA wsaData;
	WORD wVersionRequested;
	static int WSA_init_flag = 1;

	if(WSA_init_flag) {
		wVersionRequested = MAKEWORD( 2, 0 );
		if(WSAStartup( wVersionRequested, &wsaData ) != 0) {
			fprintf(stderr,"Winsock.DLL not suport !!\n");
			return 0;
		}
		WSA_init_flag = 0;
	}
#endif
	return 1;
}

/*****************************************************************
	Create a new server socket
	returns -1 on error
******************************************************************/
int Ssocket(char *host, int port)
{
	int sock;
	struct sockaddr_in adr;
	struct hostent *hp;

	nsock_win32_init();
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = PF_INET;

	if(host != NULL) {
		unsigned long inaddr;

		inaddr = inet_addr(host);
		if (inaddr != INADDR_NONE) memcpy(&adr.sin_addr, &inaddr, sizeof(inaddr));
		else {
			hp = gethostbyname(host);
			if (hp == NULL) return -1;
			memcpy(&adr.sin_addr, hp->h_addr, hp->h_length);
		}
	}
	else adr.sin_addr.s_addr = INADDR_ANY;

	adr.sin_port = htons(port);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Socket open error");
		return sock;
	}
	if (bind(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0) {
		perror("Bind");
		close (sock);
		return -1;
	}
	if (listen(sock, 5) < 0) {
		perror("listen");
		close (sock);
		return -1;
	}

	return sock;
}

/*****************************************************************
	Create a new client socket
	returns -1 on error
******************************************************************/
int Socket(char *host, int clientPort)
/*  *hostname
    clientPort  SMTP -> 25  POP2 -> 109  POP3 -> 110 */
{
	int sock;
	unsigned long inaddr;
	struct sockaddr_in adr;
	struct hostent *hp;

	nsock_win32_init();
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = PF_INET;

	inaddr = inet_addr(host);
	if (inaddr != INADDR_NONE) memcpy(&adr.sin_addr, &inaddr, sizeof(inaddr));
	else {
		hp = gethostbyname(host);
		if (hp == NULL) return -1;
		memcpy(&adr.sin_addr, hp->h_addr, hp->h_length);
	}
	adr.sin_port = htons(clientPort);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) return sock;
	if (connect(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0) return -1;
	return sock;
}

#ifndef __WIN32__ /* Now not support */

#define	MACA	ifr.ifr_hwaddr.sa_data
char *get_mac_adrs(char *ifname, char *hwaddr)
{
	struct ifreq ifr;
	int sock;

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) return NULL;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
		close(sock);
		return NULL;
	}

	if (hwaddr == NULL) {
		hwaddr = malloc(64);
	}

	if (hwaddr != NULL) /* check error  not "else" */
		sprintf(hwaddr, "%02X:%02X:%02X:%02X:%02X:%02X", (MACA[0] & 0377), (MACA[1] & 0377), (MACA[2] & 0377), (MACA[3] & 0377), (MACA[4] & 0377), (MACA[5] & 0377));

	close(sock);
	return (hwaddr);
}

char *get_ip_adrs(char *ifname, char *ipaddr)
{
	struct ifreq ifr;
	int sock;
	struct sockaddr_in *sap;

	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) return NULL;
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(sock, SIOCGIFADDR, &ifr) != 0) {
		close(sock);
		return NULL;
	}
	if (ipaddr == NULL) {
		ipaddr = malloc(16);
	}
	sap = (struct sockaddr_in *)&ifr.ifr_addr;
	strncpy(ipaddr, inet_ntoa(sap->sin_addr), 15);
	ipaddr[15] = '\0';
	close(sock);
	return (ipaddr);
}
#endif /* __WIN32__ */

/*****************************************************************
	Create a new udp socket
	returns -1 on error
******************************************************************/
static int udp_sockopen(int port, int bflg)
/* if server mode port = Port Num else port = 0 */
{
	int sock;
	struct sockaddr_in adr;

	nsock_win32_init();
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock < 0) return sock;

	memset(&adr, 0, sizeof(adr));
	adr.sin_family = PF_INET;
	adr.sin_port = htons(port);
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bflg)
		if (bind(sock, (struct sockaddr *)&adr, sizeof(adr)) < 0) return -1;
	return sock;
}


int UdpSock(int port)
/* if server mode port = Port Num else port = 0 */
{
	return (udp_sockopen(port, 1));
}

int UdpSockNobind(int port)
/* if server mode port = Port Num else port = 0 */
{
	return (udp_sockopen(port, 0));
}
		
/*****************************************************************
	Set a new udp socket to port & server
******************************************************************/
int UdpAdrSet(int pt, char *ip, struct sockaddr_in *adr)
{
	struct in_addr inaddr;
	struct hostent *hp;

	memset(adr, 0, sizeof(struct sockaddr_in));
	if (inet_aton(ip, &inaddr) != 0)
		memcpy(&adr->sin_addr, &inaddr, sizeof(inaddr));
	else {
		hp = gethostbyname(ip);
		if (hp == NULL) return -1;
		memcpy(&adr->sin_addr, hp->h_addr, hp->h_length);
	}
	adr->sin_family = PF_INET;
	adr->sin_port = htons(pt);
	return 0;
}

/*****************************************************************
Read a chunk of bytes from the socket.
Returns 0 for success.
******************************************************************/
size_t SockRead(int sock, char *buf,size_t len)
{
	size_t n, l = 0;

	while (len) {
		n = read(sock, &buf[l], len);
		if (n <= 0) return -1;
		len -= n;
		l += n;
	}
	return (l);
}

/*****************************************************************
Get a string terminated by an '\n', delete any '\r' and the '\n'.
Pass it a valid socket, a buffer for the string, and
the length of the buffer (including the trailing \0)
returns 0 for success.
*****************************************************************/

size_t SockGets(int sock, char *buf, size_t len)
{
	size_t r;
	size_t l = 0;

	while (--len) {
		r = read(sock, buf, 1);
		if (r < 0)
			return(r);

		if (r == 0) continue;
		if (*buf == '\n') {
			++buf;
			++l;
			break;
		}
		++buf;
		++l;
	}
	*buf = '\0';

	return (l);
}

/*****************************************************************
Write a chunk of bytes to the socket.
Returns 0 for success.
******************************************************************/
size_t SockWrite(int sock, char *buf, size_t len)
{
	size_t n, l = 0;

	while (len) {
		n = write(sock, buf, len);
		if (n <= 0) return -1;
		len -= n;
		buf += n;
		l += n;
	}
	return (l);
}

/*****************************************************************
Send formatted output to the socket, followed
by a CR-LF.
Returns 0 for success.
******************************************************************/
size_t SockPrintf(int sock, char *format, ...)
{
	va_list ap;
	char buf[8192];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	return SockWrite(sock, buf, strlen(buf));
}


/*****************************************************************
 Check socket for readability.  return 0 for not readable,
 !0 for readable.
******************************************************************/
int SockStatus(int sock, unsigned ms, fd_set *fds)
{
	struct timeval timeout;
	int r;

	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = (ms % 1000) * 1000;

	FD_ZERO(fds);
	FD_SET(sock, fds);
    r = select(sock+1, fds, NULL, NULL, &timeout);
	return r;
}

/*****************************************************************
 Check socket for writability.  return 0 for not writable,
 !0 for writable.
******************************************************************/
int WSockStatus(int sock,unsigned ms, fd_set *fds)
{
	struct timeval timeout;
	int r;

	timeout.tv_sec = ms / 1000;
	timeout.tv_usec = (ms % 1000) * 1000;

	FD_ZERO(fds);  
	FD_SET(sock, fds);
	r = select(sock+1, NULL, fds, NULL, &timeout);
	return r;
}

/*****************************************************************
  make client socket.
  returns -1 on error
******************************************************************/
int act_rec(int sock, char **ip)
{
	socklen_t l;
	struct sockaddr_in adr;
	int netsock;
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

#ifndef __WIN32__ /* Not support Windows */
/*
 static function use local only
*/

static int submsec(struct timeb *tps, struct timeb *tpd)
{
	unsigned short ms;
	int t, tt = 0;

	if (tps->millitm >= tpd->millitm) ms = tps->millitm - tpd->millitm;
	else {
		ms = (tps->millitm + 1000) - tpd->millitm;
		tt = -1;
	}
	t = (((int)tps->time - (int)tpd->time) + tt) * 1000 + (int)(ms);
	return t;
}

/*****************************************************************
Read a chunk of bytes from the socket.
Returns Readbytes for success.
******************************************************************/
size_t UdpReadTout(int sock, void *buf, size_t len, struct sockaddr *adr, unsigned ms)
{
	size_t n;
	fd_set fds;
	socklen_t slt;

	slt = sizeof(struct sockaddr_in);
	n = SockStatus(sock, ms, &fds);
	if (FD_ISSET(sock, &fds)) {
		n = recvfrom(sock, buf, len, 0, adr, &slt);
		if (n <= 0) return -1;
	}
	return (n);
}

/*****************************************************************
Write a chunk of bytes to the socket.
Returns 0 for success.
******************************************************************/
size_t UdpWriteTout(int sock, void *buf,size_t len, struct sockaddr *adr, unsigned ms)
{
	size_t n;
	fd_set fds;

	n = WSockStatus(sock, ms, &fds);
	if (FD_ISSET(sock, &fds)) {
		n = sendto(sock, buf, len, 0, adr, sizeof(struct sockaddr_in));
		if (n <= 0) return -1;
	}
	return (n);
}

/*****************************************************************
Read a chunk of bytes from the socket.
Returns Readbytes for success.
******************************************************************/
size_t SockReadTout(int sock, char *buf,size_t len, time_t tout)
{
	size_t st, n, l = -2;
	struct timeb tb, tn;
	fd_set fds;

	ftime(&tb);
	while (len) {
		st = SockStatus(sock, 100, &fds);
		if (FD_ISSET(sock, &fds)) {
			n = recv(sock, &buf[l], 1, 0);
			if (n <= 0) return -1;
			len -= n;
			l += n;
		}
		else {
			ftime(&tn);
			if (submsec(&tn, &tb) >= tout) break;
		}
	}
	return ((st < 0)? -1: l);
}

/*****************************************************************
Write a chunk of bytes to the socket.
Returns 0 for success.
******************************************************************/
size_t SockWriteTout(int sock, char *buf,size_t len, time_t tout)
{
	size_t st, n, l = -2;
	struct timeb tb, tn;
	fd_set fds;

	ftime(&tb);
	while (len) {
		st = WSockStatus(sock, 100, &fds);
		if (FD_ISSET(sock, &fds)) {
			n = write(sock, buf, 1);
			if (n <= 0) return -1;
			len -= n;
			buf += n;
			l += n;
		}
		else {
			ftime(&tn);
			if (submsec(&tn, &tb) >= tout) break;
		}
	}
	return ((st < 0)? -1: l);
}

/*****************************************************************
Get a string terminated by an '\n', delete any '\r' and the '\n'.
Pass it a valid socket, a buffer for the string, and
the length of the buffer (including the trailing \0)
returns 0 for success.
*****************************************************************/
size_t SockGetsTout(int sock, char *buf, size_t len, time_t tout)
{
	ssize_t r;
	size_t l = 0;
	struct timeb tb, tn;
	fd_set fds;

	ftime(&tb);

	while (--len) {
		SockStatus(sock, 10, &fds);
		if (FD_ISSET(sock, &fds)) {
			r = recv(sock, buf, 1, 0);
			if (r < 0)
				return(r);

			if (r == 0) goto chk_time;
			if (*buf == '\n') {
				++buf;
				++l;
				break;
			}
			++buf;
			++l;
		}
		else {
chk_time:
			ftime(&tn);
			if (submsec(&tn, &tb) >= tout) {
				*buf ='\0';
				return -2;
			}
		}
	}
	*buf = '\0';

	return (l);
}

/*****************************************************************
Send formatted output to the socket, followed
by a CR-LF.
Returns 0 for success.
******************************************************************/
int SockPrintfTout(int sock, time_t tout, char *format, ...)
{
	va_list ap;
	char buf[8192];

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	return (int)SockWriteTout(sock, buf, strlen(buf), tout);
}
#endif /* __WIN32__ */

/*****************************************************************
Get a string terminated by an '\n', delete any '\r' and the '\n'.
Pass it a valid socket, a buffer for the string, and
the length of the buffer (including the trailing \0)
returns 0 for success.
*****************************************************************/
void crcut(char *s)
{
	char *p;

	if ((p = strchr(s, 0x0a)) != NULL) *p = '\0';
	if ((p = strchr(s, 0x0d)) != NULL) *p = '\0';
}

size_t UdpSockGetsFrom(int sock, struct sockaddr_in *adr, char *buf, size_t len)
{
	size_t l;
	socklen_t adrlen = sizeof(struct sockaddr_in);

	l = recvfrom(sock, buf, len, 0, (struct sockaddr *)adr,&adrlen);
	if (l < 0) return -1;
	crcut(buf);
	return (strlen(buf));
}

/*****************************************************************
Send formatted output to the socket, followed
Returns transmit length for success.
******************************************************************/
size_t UdpSockPrintfTo(int sock, struct sockaddr_in *addr, char *format, ...)
{
	va_list ap;
	char buf[8192];
	size_t l;

	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);

	l = sendto(sock, buf, strlen(buf), 0, 
	           (struct sockaddr *)addr, sizeof(struct sockaddr_in));
	return((l < 0)? -1: l);
}



/*****************************************************************
 Compare IP address(hostname) (h1) & IP address(hostname) (h2)
 If sane IP address return 0
******************************************************************/
static int inet2_long(char *host, struct in_addr *adr)
{
	struct hostent *hp;

	if (inet_aton(host, adr) == 0) {
		hp = gethostbyname(host);
		if (hp == NULL) return -1;
		memcpy(adr, hp->h_addr, hp->h_length);
	}
	return 1;
}

int hostnamecmp(char *h1, char *h2)
{
	struct in_addr a1, a2;

	if (inet2_long(h1, &a1) < 0) return -1;
	if (inet2_long(h2, &a2) < 0) return -2;
	return (a1.s_addr - a2.s_addr);
}

/*****************************************************************
 convert tcp socket(tcp/udp) to IP address
******************************************************************/
char *tcp_IPadress (int sock, char *ip_buf)
{
	struct sockaddr_in adr;
	socklen_t adrlen = sizeof (struct sockaddr_in);
	char *ip = NULL;

	if (getpeername (sock, (struct sockaddr *) &adr, &adrlen) > 0) {
		if (adr.sin_family == PF_INET) {
			strncpy(ip_buf, inet_ntoa(adr.sin_addr), 15);
			ip = ip_buf;
			ip[15] = '\0';
		}
	}
	return (ip);
}
