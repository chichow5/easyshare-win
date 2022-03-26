#ifndef _THREADPROC_H
#define _THREADPROC_H

/*--cross platform--*/
#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#define SOCKET int
#include <WinSock2.h>
#include <Windows.h>
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

#define FINE  0
#define INFO  1
#define ERROR 2
#define FATAL 3

using namespace std;

char types[][30] = {
  "text",
  "file"
};
char lev[][11]={
    "FINE", 
    "INFO", 
    "ERROR",
    "FATAL",
};

void udp_feedback(int);
void tcp_main(int);
void log(int level, char *processName, const char *fmt, ...);
void p_exit(char *processName, int status);

#endif //_THREADPROC_H
