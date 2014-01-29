/***************************************************************************
 * Copyright 2000-2005 Nobby Noboru Hirano
 * All rights reserved
 *
 * Ntools BSD License
 * Version 1.0 March 2005
 *
 * Nobby Noboru Hirano <nobby@ntools.net>
 ***************************************************************************/

int Ssocket(const char *host, const int port);
int Socket(const char *host, const int port);
int UdpSock(const int port);
int UdpSockNobind(const int port);
int UdpAdrSet(int pt, char *ip, struct sockaddr_in *adr);
size_t SockRead(const int sock, void *buf, size_t len);
char *SockGets(const int sock, char *buf, size_t len);
size_t SockWrite(const int sock, void *buf, size_t len);
size_t SockPrintf(const int sock, char *format, ...);
/*
 each functions check timeout tout msec.
 if timeout return -2;
 */
size_t SockReadTout(const int sock, void *buf,int len, size_t tout);
size_t SockWriteTout(const int sock, void *buf,int len, size_t tout);
size_t SockGetsTout(const int sock, char *buf, int len, size_t tout);
size_t SockPrintfTout(const int sock, int tout, char *format, ...);
size_t UdpReadTout(int sock, void *buf, size_t len, struct sockaddr *adr, unsigned ms);
size_t UdpWriteTout(int sock, void *buf,size_t len, struct sockaddr *adr, unsigned ms);

size_t UdpSockGetsFrom(int sock, struct sockaddr_in *adr, char *buf, size_t len);
size_t UdpSockPrintfTo(int sock, struct sockaddr_in *addr, char *format, ...);


int SockStatus(int sock, unsigned ms, fd_set *fds);
int WSockStatus(int sock,unsigned ms, fd_set *fds);

/*************************************
     tcp utility
*************************************/
/* 
 tcp_IPadress is convert from socket to ip adrress(char *)
 return value 
          error   NULL
          success char *ip
*/
char *tcp_IPadress (int sock, char ip[]);	/* ip[n] 'n' is more than 16 */

/* 
 hostnamecmp is compare like a strcmp
 However 'hostnamecmp' is available hostname & ip address.
 Able to compare ex. 'www.ntools.net' and '172.16.0.1'.
 */
int hostnamecmp(char *host1, char *host2);

/*
 get infomation from interface name ifname = eth0, wlan0 or any netdevice.
 set to hwaddr must be 64 byte or more, or NULL. if NULL rutern allocated
 char pointer.
 get_mac_adrs = H/W address , get_ip_adrs = ipv4 IP address
*/  
char *get_mac_adrs(char *ifname, char *hwaddr);
char *get_ip_adrs(char *ifname, char *ipaddr);


/* open UNIX server socket */
int ProcSsocket(const char *sname);
int ProcSocket(const char *sname);
size_t ProcRead(const int sock, void *buf, size_t len);
char *ProcGets(const int sock, char *buf, size_t len);
size_t ProcPrintf(const int sock, char *format, ...);

/* sleep until TCP connection on a socket */
int act_rec(int sock, char **ip);

/* TCP callback functions */
int init_Nsock_input(void);	/* must call befor using Nsock callback functions */
int close_Nsock_input(void); /* after using Nsock callback */

/* Return tag Number it is using for Nsock_input_remove */
int Nsock_input_add(int sock, unsigned condition,
                    void (*func)(void *, int , unsigned), void *data);

void Nsock_input_remove(int tag);
