#include "threadproc.h"
#include <tchar.h>
//#include <Mstcpip.h>

#ifdef _WIN32
#pragma comment (lib, "ws2_32.lib") //¨n§ã§ä§î ws2_32.dll
#endif //_WIN32

char mytypes[][30] = {
  "text",
  "file"
};
char lev[][11]={
    "FINE",
    "INFO",
    "ERROR",
    "FATAL",
};
void log(int level, char *processName, const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	char buffer[256];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	
	string out;
	out += '[';
	out += lev[level];
	out += ']';
	out += '(';
	out += processName;
	out += ')';
	out += ':';
	out += buffer;
	
  if (level < _ERROR){
    //fprintf(stdout, "[%s](%s): ",lev[level], processName);
    //vfprintf(stdout, fmt, ap);
    //fprintf(stdout, "\n");
  }else{
    //fprintf(stderr, "[%s](%s): ",lev[level], processName);
    //vfprintf(stderr, fmt, ap);
    //fprintf(stderr, "(system): %s",strerror(errno));
    //fprintf(stderr, "\n");
    out += "(system):";
	out += strerror(errno);
	//stream<<"(system)"<<strerror(errno);
  }
  //stream<<endl;
  out+='\n';
  
	//OutputDebugString((char*)stream.str().c_str());
	OutputDebugString(out.c_str());
}
void log2(int level, char *processName, const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  if (level < _ERROR){
    fprintf(stdout, "[%s](%s): ",lev[level], processName);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
  }else{
    fprintf(stderr, "[%s](%s): ",lev[level], processName);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "(system): %s",strerror(errno));
    fprintf(stderr, "\n");
  }
  va_end(ap);
}

void p_exit(char *processName, int status){
  fprintf(stderr, "[_INFO]: %s process exit\n",processName);
  exit(status);
}

class Cryption {
private :
  unsigned char key;
  unsigned char getKey(){
    return (unsigned char)85;
  }
public:
  Cryption (){
    this->key = this->getKey();
  }
  void encryption(void *vptr, int len){
    //do nothing 
    return;
    unsigned char *ptr = (unsigned char*)vptr;
    for (int i=0; i<len; i++) ptr[i] ^= key;
  }
  void decryption(void *vptr, int len){
    //do nothing
    return;
    unsigned char *ptr = (unsigned char*)vptr;
    for (int i=0; i<len; i++) ptr[i] ^= key;
  }
};

ssize_t readn(int fd, void *vptr, size_t nbytes){
  int nleft = nbytes;
  char *ptr = (char*)vptr;
  int nread = 0;
  while(nleft > 0){
    if ((nread = recv(fd, ptr, std::min(nleft, MAXLINE),MSG_WAITALL)) < 0){
      if (errno == EINTR) nread = 0;
      else{
        log(_ERROR, "readn", "failed to read from remote");
        break;
      }
    }else if (nread == 0){
      log(_ERROR, "readn", "read meet EOF");
      break;
    }
    nleft -= nread;
    ptr   += nread;
  }
  return nbytes - nleft;
}

int parseInt(char *str){ //assume length no longer than 32bit int
  int re = 0;
  for (int i=0; ;i++){
    if (!std::isdigit(str[i])){
      break;
    }
    re = re*10 + str[i]-'0';
  }
  return re;
}

void hexDump(char *vptr, int len){
  for (int i=0; i<len; i++){
    if (i%8==0){
      if (i!=0) puts("");
      printf("0x%d ",i);
    }else if(i%4==0){
      if (i!=0) printf(" ");
    }
    for (int j=7; j>=0; j--){
      printf("%d",(vptr[i]>>j&1));
    }
    printf(" ");
  }
  puts("");
}

void StopUDP(int port){
#ifdef _WIN32
	WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
#endif
	bool bSucceed = false;;
	SOCKET sockfd;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    	log(_FATAL, "StopUPD", "can't init sockfd");
    	return;
  	}
  	char str_cmd[] = "cmd:stop";
  	const int len_cmd = strlen(str_cmd);
	char str_res[] = "UDPFeedback stopped";
	const int len_res = strlen(str_res)+100;
	char *buf;
	if ((buf = (char*)malloc(len_res * sizeof(char))) == NULL){
		log(_FATAL, "StopUPD","can't init recv buffer");
		return;
	}
	
	struct sockaddr_in servaddr;
	socklen_t len = sizeof(servaddr);
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	/* send to localhost:port */
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	
	/*fucking windows, giving up. */
	sendto(sockfd, str_cmd, len_cmd, 0, (SA *)&servaddr, len);
	/*
	/ set receivce timeout /
	fd_set fds;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	//BOOL bNewBehavior = FALSE;
	//DWORD dwBytesReturned = 0;
	//WSAIoctl(sockfd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
	
	/ try 5 times /
	int mtries = 5, tries = 1, n, nrecv;
	struct sockaddr recvaddr;
	int lenaddr;
	log(_FINE,"UDPStop","start sending stop cmd");
	while(tries <= mtries){
		sendto(sockfd, str_cmd, len_cmd, 0, (SA *)&servaddr, len);
		lenaddr = sizeof(recvaddr);
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		n = select(sockfd, &fds, NULL, NULL, &tv);
		if (WSAGetLastError() == 10054){
			n = select(sockfd, &fds, NULL, NULL, &tv);
		}
		if (n == 0){
			log(_INFO,"UDPStop","waiting feedback timeout");
			continue;
		}else if (n == -1){
			log(_ERROR,"UDPStop Select","%d",WSAGetLastError());
			continue;
		}
		
		nrecv = recvfrom(sockfd, buf, len_res, 0, (SA *)&recvaddr, &lenaddr);
		if (nrecv == SOCKET_ERROR){
			log(_ERROR,"UDPStop","SOCKET_ERROR %d",WSAGetLastError());
		}else if (nrecv==len_res&&strncmp(str_res,buf,len_res)==0){
			log(_INFO,"UDPStop","stop finished");
			bSucceed = true;
			break;
		}
		tries++;
	}
	if (!bSucceed){
		log(_INFO,"UDPStop", "stop failed after %d tries",mtries);
	}
	*/
	
#ifdef _WIN32    
    WSACleanup();
#endif
}
void UDPFeedback(int port){
#ifdef _WIN32
	WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
#endif

  SOCKET sockfd;
  socklen_t clilen;
  struct sockaddr_in servaddr, cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    log(_FATAL, "udp_feedback", "can't init socket for sockfd");
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0){
    log(_FATAL, "udp_feedback", "can't bind to port %d",port);
    p_exit("udp_feedback",EXIT_FAILURE);
  }else{
    log(_FINE, "udp_feedback", "start listening on port %d",port);
  }

  char *buf;
  if ((buf = (char*)malloc((MAXLINE+10) * sizeof(char))) == NULL){
    log(_FATAL, "udp_feedback", "can't malloc space for"
      " buffer on size %d",MAXLINE+10);
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  Cryption *cp = new Cryption();
  char str_need[] = "Is this address stands a server?";
  int len_sn = strlen(str_need);
  char str_response[] = "Yes, I am!";
  int len_rp = strlen(str_response);
  cp->encryption((void*)str_response, len_rp);
  
  /* client from localhost send stop command; */
  char str_cmd[] = "cmd:stop";
  char str_res[] = "UDPFeedback stopped";
  while(1) {
    clilen = sizeof(cliaddr);
    /* package received */
    int n = recvfrom(sockfd, buf, MAXLINE, 0, (SA *)&cliaddr, &clilen);
    buf[n] = 0;
    cp->decryption((void*)buf, n);
    
    bool bMatch = false;
	if (buf[0] == 'c'){
		if (strncmp(str_cmd, buf, strlen(str_cmd)) == 0){
			//stop command received
			//send feedback
			sendto(sockfd, str_res, strlen(str_res), 0, (SA *)&cliaddr, clilen);
			//break the loop;
			break;
		}
	}else if (strncmp(buf, str_need, len_sn) == 0){
    	bMatch = true;
    }
    if (!bMatch){
		/* bad message, giving up */
    	log(_INFO, "upd_feedback", "client not match, giving up");
    	fprintf(stdout,"\t\t%s\n",buf);
    }else{
    	/* send response */
    	sendto(sockfd, str_response, len_rp, 0, (SA *)&cliaddr, clilen);
    	log(_INFO, "upd_feedback","client match, send response");
	}
  }
  closesocket(sockfd);
  free(buf);
  log(_INFO, "UDPFeedback","cmd:stop;Process exiting");
  //system("pause");
#ifdef _WIN32
  WSACleanup();
  ExitThread(EXIT_SUCCESS);
#else
  Exit(EXIT_SUCCESS);
#endif
  
}

/* notice string edge */
int utf82gbk(char *utf8s){
	int len = MultiByteToWideChar(CP_UTF8,0,utf8s,-1,NULL ,0);
	wchar_t *tmp = (wchar_t *)malloc((len+10)*sizeof(wchar_t));
	if (tmp == NULL){
		log(ERROR, "utf82gbk","malloc error for tmp string");
		return -1;
	}
	len = MultiByteToWideChar(CP_UTF8,0,utf8s,-1,tmp  ,len);
	len = WideCharToMultiByte(CP_ACP ,0,tmp  ,-1,NULL ,0   ,NULL,0);
	len = WideCharToMultiByte(CP_ACP ,0,tmp  ,-1,utf8s,len ,NULL,0);
	utf8s[len-1] = 0;
	free(tmp);
	return len-1;
}

void StopTCP(int port){
#ifdef _WIN32
	WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), &wsaData);
#endif

  	char str_cmd[] = "cmd:stop";
  	const int len_cmd = strlen(str_cmd);
  	/*
	char str_res[] = "TCPMain stopped";
	const int len_res = strlen(str_res)+100;
	char *buf;
	if ((buf = (char*)malloc(len_res * sizeof(char))) == NULL){
		log(_FATAL, "StopUPD","can't init recv buffer");
		return;
	}*/
	SOCKET sockfd;
	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		log(_ERROR, "StopTCP", "can't init sockfd");
    	return;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);
    connect(sockfd, (SA *)&servaddr, sizeof(SA));
    
	send(sockfd, str_cmd, len_cmd, NULL);
	
	log(_INFO,"StopTCP","finisehd");
	closesocket(sockfd);
	
#ifdef _WIN32    
    WSACleanup();
#endif
}
void TCPMain(int port){
  WSADATA wsaData;
  WSAStartup( MAKEWORD(2, 2), &wsaData);
  
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    log(_FATAL, "tcp_main", "can't init socket for listenfd");
    p_exit("tcp_main", EXIT_FAILURE);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0){
    log(_FATAL, "tcp_main", "can't bind listenfd");
    p_exit("tcp_main", EXIT_FAILURE);
  }else{
    log(_INFO, "tcp_main", "start listening on port %d", port);
  }
  listen(listenfd, 20);

  char *buf;
  if ((buf = (char*)malloc((MAXLINE+10) * sizeof(char))) == NULL){
    log(_FATAL, "udp_feedback", "can't malloc space for"
      " buffer on size %d",MAXLINE+10);
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  Cryption *cp = new Cryption();
  char str_need[] = "I want to send a package";
  int len_sn = strlen(str_need);
  char str_cmd[] = "cmd:stop";
  const int len_cmd = strlen(str_cmd);
  char path[200] = "C:\\Users\\chichow\\phoneShare\\";
  char *of_path = path+28;
  while(1) {
    clilen = sizeof(cliaddr);
    //socket established with client
    connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
    log(_FINE, "tcp_main", "handling client");
    
    readn(connfd, (void*)buf, len_sn);
    buf[len_sn] = 0;
    cp->decryption((void*)buf, len_sn);
    log(_INFO,"tcp_main","client request:%s",buf);
	bool bReqMatch = false;
	if (buf[0] == 'c'){
    	if (strncmp(buf, str_cmd, len_cmd) == 0){
    		log(_INFO, "TCPMain","match stop cmd");
    		close(connfd);
    		break;
    	}
    }else if (strncmp(buf, str_need, len_sn) == 0){
    	bReqMatch = true;
    }
    if (!bReqMatch){
      //bad request, giving up
      log(_INFO,"tcp_main", "bad request, giving up");
      close(connfd);
      continue;
    }
    //prepare to receive file or text
    //asume header length less than MAXLINE
    //to the truth, much less than
    int nread = readn(connfd, buf, MAXLINE);

    /**
     * header goes like
     * text;description;length; + binary text in UTF-8
     * file;filename;length; + binary file content
     */
    int nsplit = 0,j=0,lenh=0;
    char head[3][200];
    int len;
    for (; lenh<nread; lenh++){
      if (buf[lenh] == ';'){
        head[nsplit++][j++] = 0;
        cp->decryption(head[nsplit-1],j);
		j = 0;
#ifdef _WIN32
		/* get filename from UTF-8 */
		/* my Windows using gbk charset */
        if (nsplit == 2) utf82gbk(head[1]);
#endif
		log(_FINE,"header split", "head[%d]:%s",nsplit-1,head[nsplit-1]);
        if (nsplit == 3) break;
      }else{
        head[nsplit][j++] = buf[lenh];
      }
    }
    /* can't handle UTF-8 currently, throw to file */
    
    bool text_flag = false;
    if (strcmp(head[0], mytypes[0]) == 0){ // type matches text
      strcpy(of_path, "text.txt");
      of_path[8] = 0;
      text_flag = true;
    }else if (strcmp(head[0], mytypes[1]) == 0){ //type matches file
      strcpy(of_path, head[1]);
      of_path[strlen(head[1])] = 0;
    }else{
      log(_INFO, "write to file", "bad type");
      close(connfd);
      continue; //back to loop
    }
    //C:\Users\chichow\phoneShare
    log(_INFO,"file path:%s",path);
    FILE *fd;
    if ((fd = fopen(path, "wb")) == NULL){
      log(_ERROR, "receive", "can't open %s", path);
      closesocket(connfd);
      continue;
    }
    // write to file
    lenh++; //buf[lenh] must be ';'
    fwrite(buf+lenh, nread-lenh, 1, fd);
    //nread tried to get MAXLINE chars but failed,
    //meaning no data reamaining
    int nleft = parseInt(head[2])-(nread-lenh);
    if (nread == MAXLINE){
      while(nleft > 0){
        if ((nread = readn(connfd, buf, std::min(MAXLINE, nleft)))
            < std::min(MAXLINE, nleft)){
          //didn't receive wanted length package
          break;
        }
        fwrite(buf, nread, 1, fd);
        nleft -= nread;
      }
    }
    if (nleft != 0){
      log(_INFO,"receive", "length giving and received not match");
    }
    close(connfd);
    fclose(fd);
    log(_INFO,"tcp_main","text or file received");
#ifndef _WIN32
	//copy to the clipboard by `xsel` in linux
	if (text_flag){
      system("cat text.txt|xsel -b");
    }
#endif
  }
  closesocket(listenfd);
  free(buf);
  log(_INFO,"TCPMain","cmd:stop;Process exiting");
  //system("pause");
  
#ifdef _WIN32
  WSACleanup();
  ExitThread(EXIT_SUCCESS);
#else
  Exit(EXIT_SUCCESS);
#endif
}


