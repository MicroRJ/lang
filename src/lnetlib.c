/*
** See Copyright Notice In elf.h
** lnetlib.c
** Very Simple Sockets Lib
*/


/* this is just to get some basic games going on */

#if !defined(PLATFORM_WEB)

# pragma comment(lib,"Ws2_32")

#include "Winsock2.h"
#include "ws2tcpip.h"
#include   "ws2def.h"

typedef struct LMSG {
	unsigned int length;
} LMSG;



elf_api int netlib_init(elf_ThreadState *R) {
	WSADATA data;
	WSAStartup(MAKEWORD(2,2),&data);
	return 0;
}


elf_api int netlib_close(elf_ThreadState *R) {
	WSACleanup();
	return 0;
}


elf_api int netlib_listen(elf_ThreadState *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	int error = listen(handle,SOMAXCONN);
	elf_locint(R,error!=SOCKET_ERROR);
	return 1;
}


elf_api int netlib_accept(elf_ThreadState *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	SOCKET client = accept(handle,NULL,NULL);
	elf_locsys(R,(elf_Handle)client);
	return 1;
}


elf_api int netlib_pollclient(elf_ThreadState *R) {
	SOCKET handle = (SOCKET) elf_getsys(R,0);
	fd_set ready;
	FD_ZERO(&ready);
	FD_SET(handle,&ready);
	TIMEVAL timeout = {0};
	int result = select(0,&ready,NULL,NULL,&timeout);
   if (FD_ISSET(handle,&ready)) {
      SOCKET client = accept(handle,NULL,NULL);
      elf_ensure(client != INVALID_SOCKET);
		elf_locsys(R,(elf_Handle)client);
   } else elf_locnil(R);
	return 1;
}


elf_api int netlib_tcpserver(elf_ThreadState *R) {
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
		elf_locsys(R,(elf_Handle)thesocket);
	} else elf_locnil(R);

	return 1;
}


elf_api int netlib_tcpclient(elf_ThreadState *R) {
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
		elf_locsys(R,(elf_Handle)thesocket);
	} else elf_locnil(R);
	return 1;
}


elf_api int netlib_send(elf_ThreadState *R) {
	/* todo: make this a class? */
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	elf_String *payload = elf_getstr(R,1);
	LMSG message = { payload->length };
	elf_int sent = 0;
	sent += send(socket,(char*)&message,sizeof(message),0);
	sent += send(socket,payload->c,payload->length,0);
	elf_locint(R,sent);
	return 1;
}


elf_api int netlib_ioctl(elf_ThreadState *R) {
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	long mode = 1;
	int error = ioctlsocket(socket,FIONBIO,&mode);
	elf_locint(R,error == 0);
	return 1;
}


elf_api int netlib_recv(elf_ThreadState *R) {
	SOCKET socket = (SOCKET) elf_getsys(R,0);
	LMSG message = {0};
	if (recv(socket,(char*)&message,sizeof(message),0) != -1) {
		if (message.length != 0) {
			elf_int length = message.length;
			elf_String *obj = elf_newstrlen(R,length);
			elf_locstr(R,obj);
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
					elf_ensure(result > 0);
					length -= result;
					cursor += result;
				}
			} while (length != 0);
			*cursor = 0;
		} else elf_locnil(R);
	} else elf_locnil(R);
	return 1;
}
#else
elf_api int netlib_init(elf_ThreadState *R) { return 0; };
elf_api int netlib_close(elf_ThreadState *R) { return 0; };
elf_api int netlib_listen(elf_ThreadState *R) { return 0; };
elf_api int netlib_accept(elf_ThreadState *R) { return 0; };
elf_api int netlib_pollclient(elf_ThreadState *R) { return 0; };
elf_api int netlib_tcpserver(elf_ThreadState *R) { return 0; };
elf_api int netlib_tcpclient(elf_ThreadState *R) { return 0; };
elf_api int netlib_send(elf_ThreadState *R) { return 0; };
elf_api int netlib_ioctl(elf_ThreadState *R) { return 0; };
elf_api int netlib_recv(elf_ThreadState *R) { return 0; };
#endif


elf_api void netlib_load(elf_ThreadState *R) {
	elf_Module *md = R->md;
	lang_addglobal(md,elf_newlocstr(R,"listen"),elf_valbid(netlib_listen));
	lang_addglobal(md,elf_newlocstr(R,"accept"),elf_valbid(netlib_accept));
	lang_addglobal(md,elf_newlocstr(R,"pollclient"),elf_valbid(netlib_pollclient));
	lang_addglobal(md,elf_newlocstr(R,"tcpserver"),elf_valbid(netlib_tcpserver));
	lang_addglobal(md,elf_newlocstr(R,"tcpclient"),elf_valbid(netlib_tcpclient));
	lang_addglobal(md,elf_newlocstr(R,"netlib_init"),elf_valbid(netlib_init));
	lang_addglobal(md,elf_newlocstr(R,"netlib_close"),elf_valbid(netlib_close));
	lang_addglobal(md,elf_newlocstr(R,"send"),elf_valbid(netlib_send));
	lang_addglobal(md,elf_newlocstr(R,"recv"),elf_valbid(netlib_recv));
	lang_addglobal(md,elf_newlocstr(R,"ioctl"),elf_valbid(netlib_ioctl));
}


