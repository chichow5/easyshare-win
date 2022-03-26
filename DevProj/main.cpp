#include "threadproc.h"

using namespace std;
int main(){
	OutputDebugString("Hello World");
	int cmd;
	printf("\n\t===================");
	printf("\n\t|  1.TCP Main     |");
	printf("\n\t|  2.STOP TCP     |");
	printf("\n\t|  3.UDP Feedback |");
	printf("\n\t|  4.STOP UDP     |");
	printf("\n\t==================");
	printf("\n\n\tPlease input:");
	scanf("%d",&cmd);
	switch(cmd){
		case 1:TCPMain(tcp_port);    break;
		case 2:StopTCP(tcp_port);    break;
		case 3:UDPFeedback(udp_port);break;
		case 4:StopUDP(udp_port);    break;
		default:break;	
	}
	return 0;
}
