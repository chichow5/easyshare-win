#include "threadproc.h"

#ifdef _WIN32
#pragma comment (lib, "ws2_32.lib") //╪сть ws2_32.dll
#endif //_WIN32



void log(int level, char *processName, const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  if (level < ERROR){
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
  fprintf(stderr, "[info]: %s process exit\n",processName);
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
    if ((nread = read(fd, ptr, std::min(nleft, MAXLINE))) < 0){
      if (errno == EINTR) nread = 0;
      else{
        log(ERROR, "readn", "failed to read from remote");
        break;
      }
    }else if (nread == 0){
      log(ERROR, "readn", "read meet EOF");
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

void udp_feedback(int port){
  SOCKET sockfd;
  socklen_t clilen;
  struct sockaddr_in servaddr, cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
    log(FATAL, "udp_feedback", "can't init socket for sockfd");
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0){
    log(FATAL, "udp_feedback", "can't bind to port %d",port);
    p_exit("udp_feedback",EXIT_FAILURE);
  }else{
    log(FINE, "udp_feedback", "start listening on port %d",port);
  }

  char *buf;
  if ((buf = (char*)malloc((MAXLINE+10) * sizeof(char))) == NULL){
    log(FATAL, "udp_feedback", "can't malloc space for"
      " buffer on size %d",MAXLINE+10);
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  Cryption *cp = new Cryption();
  char str_need[] = "Is this address stands a server?";
  int len_sn = strlen(str_need);
  char str_response[] = "Yes, I am!";
  int len_rp = strlen(str_response);
  //hexDump(str_response, len_rp);
  cp->encryption((void*)str_response, len_rp);
  while(1) {
    clilen = sizeof(cliaddr);
    //package received
    int n = recvfrom(sockfd, buf, MAXLINE, 0, (SA *)&cliaddr, &clilen);
    buf[n] = 0;
    cp->decryption((void*)buf, n);
    if (strncmp(buf, str_need, len_sn) != 0){
      //bad message, giving up
      log(INFO, "upd_feedback", "client not match, giving up");
      fprintf(stdout,"\t\t%s\n",buf);
      continue;
    }
    //send response
    sendto(sockfd, str_response, len_rp, 0, (SA *)&cliaddr, clilen);
    log(INFO, "upd_feedback","client match, send response");
  }
}

void tcp_main(int port){
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    log(FATAL, "tcp_main", "can't init socket for listenfd");
    p_exit("tcp_main", EXIT_FAILURE);
  }
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0){
    log(FATAL, "tcp_main", "can't bind listenfd");
    p_exit("tcp_main", EXIT_FAILURE);
  }else{
    log(INFO, "tcp_main", "start listening on port %d", port);
  }
  listen(listenfd, 20);

  char *buf;
  if ((buf = (char*)malloc((MAXLINE+10) * sizeof(char))) == NULL){
    log(FATAL, "udp_feedback", "can't malloc space for"
      " buffer on size %d",MAXLINE+10);
    p_exit("udp_feedback", EXIT_FAILURE);
  }

  Cryption *cp = new Cryption();
  char str_need[] = "I want to send a package";
  int len_sn = strlen(str_need);
  while(1) {
    clilen = sizeof(cliaddr);
    //socket established with client
    connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
    log(FINE, "tcp_main", "handling client");
    
    readn(connfd, (void*)buf, len_sn);
    buf[len_sn] = 0;
    cp->decryption((void*)buf, len_sn);
    log(INFO,"tcp_main","client request:%s",buf);
    if (strncmp(buf, str_need, len_sn) != 0){
      //bad request, giving up
      log(INFO,"tcp_main", "bad request, giving up");
      close(connfd);
      continue;
    }
    //prepare to receive file or text
    //asume header length less than MAXLINE
    //to the truth, much less than
    int nread = readn(connfd, buf, MAXLINE);

    /**
     * header goes like
     * text;description;length; + binary text in UTF-8()
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
        log(FINE,"header spliter", "head[%d]:%s",nsplit-1,head[nsplit-1]);
        if (nsplit == 3) break;
      }else{
        head[nsplit][j++] = buf[lenh];
      }
    }
    //can't handle UTF-8 currently, throw to file
    char path[200];
    bool text_flag = false;
    if (strcmp(head[0], types[0]) == 0){
      strcpy(path, "text.txt");
      path[8] = 0;
      text_flag = true;
    }else if (strcmp(head[0], types[1]) == 0){
      strcpy(path, head[1]);
      path[strlen(head[1])] = 0;
      //FILE *tmp = fopen("filename","w");
      //fprintf(tmp, "%s",path);
      //fclose(tmp);
    }else{
      log(INFO, "write to file", "bad type");
      close(connfd);
      continue; //back to loop
    }

    FILE *fd;
    if ((fd = fopen(path, "wb")) == NULL){
      log(ERROR, "receive", "can't open %s", path);
      close(connfd);
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
      log(INFO,"receive", "length giving and received not match");
    }
    close(connfd);
    fclose(fd);
    log(INFO,"tcp_main","text or file received");
    if (text_flag){
      system("cat text.txt|xsel -b");
    }
  }
}

/*
int main(){
  pid_t pid;
  if ((pid = fork()) == 0){
    udp_feedback(udp_port);
  }
  sleep(1);
  tcp_main(tcp_port);
}
*/
