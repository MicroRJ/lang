/*
** See Copyright Notice In elf.h
** lnetlib.c
** Very Simple Sockets Lib
*/


/* this is just to get some basic games going on */

#if defined(_WIN32)
# pragma comment(lib,"Ws2_32")
#include "Winsock2.h"
#include "ws2tcpip.h"
#include   "ws2def.h"

typedef struct LMSG {
	unsigned int length;
} LMSG;



lapi int netlib_init(elf_Runtime *R) {
	WSADATA data;
	WSAStartup(MAKEWORD(2,2),&data);
	return 0;
}


lapi int netlib_close(elf_Runtime *R) {
	WSACleanup();
	return 0;
}


lapi int netlib_listen(elf_Runtime *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	int error = listen(handle,SOMAXCONN);
	elf_putint(R,error!=SOCKET_ERROR);
	return 1;
}


lapi int netlib_accept(elf_Runtime *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	SOCKET client = accept(handle,NULL,NULL);
	elf_putsys(R,(elf_Handle)client);
	return 1;
}


lapi int netlib_pollclient(elf_Runtime *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	fd_set ready;
	FD_ZERO(&ready);
	FD_SET(handle,&ready);
	TIMEVAL timeout = {0};
	int result = select(0,&ready,NULL,NULL,&timeout);
   if (FD_ISSET(handle,&ready)) {
      SOCKET client = accept(handle,NULL,NULL);
      elf_assert(client != INVALID_SOCKET);
		elf_putsys(R,(elf_Handle)client);
   } else elf_putnil(R);
	return 1;
}


lapi int netlib_tcpserver(elf_Runtime *R) {
	elf_String *addrnameS = elf_getstr(R,0);
	elf_String *addrportS = elf_getstr(R,1);
	char *addrname = addrnameS ? addrnameS->c : 0;
	char *addrport = addrportS ? addrportS->c : 0;
	ADDRINFOA idealaddr = {0};
	idealaddr.ai_flags = AI_PASSIVE;
	idealaddr.ai_family = AF_INET;
	idealaddr.ai_socktype = SOCK_STREAM;
	idealaddr.ai_protocol = IPPROTO_TCP;
	ADDRINFOA *addrinfo = NULL;
	getaddrinfo(addrname,addrport,&idealaddr,&addrinfo);

	SOCKET thesocket = socket(addrinfo->ai_family,addrinfo->ai_socktype,addrinfo->ai_protocol);
	int error = bind(thesocket,addrinfo->ai_addr,addrinfo->ai_addrlen);
	if(error != SOCKET_ERROR) {
		elf_putsys(R,(elf_Handle)thesocket);
	} else elf_putnil(R);

	return 1;
}


lapi int netlib_tcpclient(elf_Runtime *R) {
	elf_String *addrnameS = elf_getstr(R,0);
	elf_String *addrportS = elf_getstr(R,1);
	char *addrname = addrnameS ? addrnameS->c : 0;
	char *addrport = addrportS ? addrportS->c : 0;
	ADDRINFOA idealaddr = {0};
	idealaddr.ai_flags = AI_PASSIVE;
	idealaddr.ai_family = AF_INET;
	idealaddr.ai_socktype = SOCK_STREAM;
	idealaddr.ai_protocol = IPPROTO_TCP;
	ADDRINFOA *addrinfo = NULL;
	getaddrinfo(addrname,addrport,&idealaddr,&addrinfo);

	SOCKET thesocket = socket(addrinfo->ai_family,addrinfo->ai_socktype,addrinfo->ai_protocol);

	int error = connect(thesocket,addrinfo->ai_addr,addrinfo->ai_addrlen);
	if(error != SOCKET_ERROR) {
		elf_putsys(R,(elf_Handle)thesocket);
	} else elf_putnil(R);
	return 1;
}


lapi int netlib_send(elf_Runtime *R) {
	/* todo: make this a class? */
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	elf_String *payload = elf_getstr(R,1);
	LMSG message = { payload->length };
	elf_int sent = 0;
	sent += send(socket,(char*)&message,sizeof(message),0);
	sent += send(socket,payload->c,payload->length,0);
	elf_putint(R,sent);
	return 1;
}


lapi int netlib_ioctl(elf_Runtime *R) {
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	long mode = 1;
	int error = ioctlsocket(socket,FIONBIO,&mode);
	elf_putint(R,error == 0);
	return 1;
}


lapi int netlib_recv(elf_Runtime *R) {
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	LMSG message = {0};
	if (recv(socket,(char*)&message,sizeof(message),0) != -1) {
		if (message.length != 0) {
			elf_int length = message.length;
			elf_String *obj = elf_newstrlen(R,length);
			elf_putstr(R,obj);
			char *cursor = obj->c;
			do {
				elf_int result = recv(socket,cursor,length,0);
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error == WSAEWOULDBLOCK) {
						/* todo: retry until a certain timeout? */
					} else {
						char erbuf[0x100];
						sys_geterrormsg(error,erbuf,sizeof(erbuf));
						elf_logerror("netlib sys error '%i': %s",error,erbuf);
						break;
					}
				} else if (result == 0) {
					/* connection closed gracefully, simply break */
					break;
				} else {
					elf_assert(result > 0);
					length -= result;
					cursor += result;
				}
			} while (length != 0);
			*cursor = 0;
		} else elf_putnil(R);
	} else elf_putnil(R);
	return 1;
}
#else
lapi int netlib_init(elf_Runtime *R) { return 0; };
lapi int netlib_close(elf_Runtime *R) { return 0; };
lapi int netlib_listen(elf_Runtime *R) { return 0; };
lapi int netlib_accept(elf_Runtime *R) { return 0; };
lapi int netlib_pollclient(elf_Runtime *R) { return 0; };
lapi int netlib_tcpserver(elf_Runtime *R) { return 0; };
lapi int netlib_tcpclient(elf_Runtime *R) { return 0; };
lapi int netlib_send(elf_Runtime *R) { return 0; };
lapi int netlib_ioctl(elf_Runtime *R) { return 0; };
lapi int netlib_recv(elf_Runtime *R) { return 0; };
#endif


lapi void netlib_load(elf_Runtime *R) {
	elf_Module *md = R->md;
	lang_addglobal(md,elf_pushnewstr(R,"listen"),lang_C(netlib_listen));
	lang_addglobal(md,elf_pushnewstr(R,"accept"),lang_C(netlib_accept));
	lang_addglobal(md,elf_pushnewstr(R,"pollclient"),lang_C(netlib_pollclient));
	lang_addglobal(md,elf_pushnewstr(R,"tcpserver"),lang_C(netlib_tcpserver));
	lang_addglobal(md,elf_pushnewstr(R,"tcpclient"),lang_C(netlib_tcpclient));
	lang_addglobal(md,elf_pushnewstr(R,"netlib_init"),lang_C(netlib_init));
	lang_addglobal(md,elf_pushnewstr(R,"netlib_close"),lang_C(netlib_close));
	lang_addglobal(md,elf_pushnewstr(R,"send"),lang_C(netlib_send));
	lang_addglobal(md,elf_pushnewstr(R,"recv"),lang_C(netlib_recv));
	lang_addglobal(md,elf_pushnewstr(R,"ioctl"),lang_C(netlib_ioctl));
}


