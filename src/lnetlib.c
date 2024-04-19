/*
** See Copyright Notice In lang.h
** lnetlib.c
** Very Simple Sockets Lib
*/


/* this is just to get some basic games
going on */

# pragma  comment(lib,"Ws2_32")
#include  "Winsock2.h"
#include  "ws2tcpip.h"
#include    "ws2def.h"


typedef struct LMSG {
	unsigned int length;
} LMSG;



lapi int netlib_init(lRuntime *R) {
	WSADATA data;
	WSAStartup(MAKEWORD(2,2),&data);
	return 0;
}


lapi int netlib_close(lRuntime *R) {
	WSACleanup();
	return 0;
}


lapi int netlib_listen(lRuntime *R) {
	SOCKET handle = (SOCKET) lang_getsysobj(R,0);
	int error = listen(handle,SOMAXCONN);
	lang_pushlong(R,error!=SOCKET_ERROR);
	return 1;
}


lapi int netlib_accept(lRuntime *R) {
	SOCKET handle = (SOCKET) lang_getsysobj(R,0);
	SOCKET client = accept(handle,NULL,NULL);
	lang_pushsysobj(R,(lsysobj)client);
	return 1;
}


lapi int netlib_pollclient(lRuntime *R) {
	SOCKET handle = (SOCKET) lang_getsysobj(R,0);
	fd_set ready;
	FD_ZERO(&ready);
	FD_SET(handle,&ready);
	TIMEVAL timeout = {0};
	int result = select(0,&ready,NULL,NULL,&timeout);
   if (FD_ISSET(handle,&ready)) {
      SOCKET client = accept(handle,NULL,NULL);
      LASSERT(client != INVALID_SOCKET);
		lang_pushsysobj(R,(lsysobj)client);
   } else lang_pushnil(R);
	return 1;
}


lapi int netlib_tcpserver(lRuntime *R) {
	lString *addrnameS = lang_getstr(R,0);
	lString *addrportS = lang_getstr(R,1);
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
		lang_pushsysobj(R,(lsysobj)thesocket);
	} else lang_pushnil(R);

	return 1;
}


lapi int netlib_tcpclient(lRuntime *R) {
	lString *addrnameS = lang_getstr(R,0);
	lString *addrportS = lang_getstr(R,1);
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
		lang_pushsysobj(R,(lsysobj)thesocket);
	} else lang_pushnil(R);
	return 1;
}


lapi int netlib_send(lRuntime *R) {
	/* todo: make this a class? */
	SOCKET socket = (SOCKET) lang_getsysobj(R,0);
	lString *payload = lang_getstr(R,1);
	LMSG message = { payload->length };
	llongint sent = 0;
	sent += send(socket,(char*)&message,sizeof(message),0);
	sent += send(socket,payload->c,payload->length,0);
	lang_pushlong(R,sent);
	return 1;
}


lapi int netlib_ioctl(lRuntime *R) {
	SOCKET socket = (SOCKET) lang_getsysobj(R,0);
	long mode = 1;
	int error = ioctlsocket(socket,FIONBIO,&mode);
	lang_pushlong(R,error == 0);
	return 1;
}


lapi int netlib_recv(lRuntime *R) {
	SOCKET socket = (SOCKET) lang_getsysobj(R,0);
	LMSG message = {0};
	if (recv(socket,(char*)&message,sizeof(message),0) != -1) {
		if (message.length != 0) {
			llongint length = message.length;
			lString *obj = langS_new2(R,length);
			lang_pushString(R,obj);
			char *cursor = obj->c;
			do {
				llongint result = recv(socket,cursor,length,0);
				if (result == SOCKET_ERROR) {
					int error = WSAGetLastError();
					if (error == WSAEWOULDBLOCK) {
						/* todo: retry until a certain timeout? */
					} else {
						char erbuf[0x100];
						sys_geterrormsg(error,erbuf,sizeof(erbuf));
						lang_logerror("netlib sys error '%i': %s",error,erbuf);
						break;
					}
				} else if (result == 0) {
					/* connection closed gracefully, simply break */
					break;
				} else {
					LASSERT(result > 0);
					length -= result;
					cursor += result;
				}
			} while (length != 0);
			*cursor = 0;
		} else lang_pushnil(R);
	} else lang_pushnil(R);
	return 1;
}


lapi void netlib_load(lRuntime *R) {
	lModule *md = R->md;
	lang_addglobal(md,lang_pushnewS(R,"listen"),lang_C(netlib_listen));
	lang_addglobal(md,lang_pushnewS(R,"accept"),lang_C(netlib_accept));
	lang_addglobal(md,lang_pushnewS(R,"pollclient"),lang_C(netlib_pollclient));
	lang_addglobal(md,lang_pushnewS(R,"tcpserver"),lang_C(netlib_tcpserver));
	lang_addglobal(md,lang_pushnewS(R,"tcpclient"),lang_C(netlib_tcpclient));
	lang_addglobal(md,lang_pushnewS(R,"netlib_init"),lang_C(netlib_init));
	lang_addglobal(md,lang_pushnewS(R,"netlib_close"),lang_C(netlib_close));
	lang_addglobal(md,lang_pushnewS(R,"send"),lang_C(netlib_send));
	lang_addglobal(md,lang_pushnewS(R,"recv"),lang_C(netlib_recv));
	lang_addglobal(md,lang_pushnewS(R,"ioctl"),lang_C(netlib_ioctl));
}


