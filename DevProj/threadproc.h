#ifndef _THREADPROC_H
#define _THREADPROC_H

/*--cross platform--*/
#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET int
#define closesocket close
#else
#include <WinSock2.h>
#include <Windows.h>
#define socklen_t int
#endif //_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <cstdarg>
#include <cstring>
#include <iostream>
#define SA struct sockaddr

#define tcp_port 54123
#define udp_port 54124

#define MAXLINE 4096

#define _FINE  0
#define _INFO  1
#define _ERROR 2
#define _FATAL 3

using namespace std;

void UDPFeedback(int port);
void StopUDP(int port);

void TCPMain(int port);
void StopTCP(int port);

void log(int level, char *processName, const char *fmt, ...);
void p_exit(char *processName, int status);

#endif //_THREADPROC_H
