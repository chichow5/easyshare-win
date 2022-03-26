#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "threadproc.h"

//#define SERVICE_NAME TEXT("srv_demo")
char SERVICE_NAME[] = "serv_demo";
SERVICE_STATUS sstatus;
SERVICE_STATUS_HANDLE sstatusHandle;

HANDLE hMainLoopThread[2];
DWORD dwMainLoopTID[2];

void onCreate();
void onResume();
void onRestart();
void onStop();

FILE *fp;

void WINAPI ServiceMain(int argc, char** argv);
void WINAPI ServiceHandler(DWORD fdwControl);
 
int main(int argc, const char** argv) {
	//fp = fopen("log.txt","w");
	//fprintf(fp,"(main)\n");
	SERVICE_TABLE_ENTRY ServiceTable[2] = 
	{ {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain}, { NULL,NULL } };
	//启动服务控制分派机线程
	StartServiceCtrlDispatcher(ServiceTable);
	return 0;
}

void WINAPI ServiceMain(int argc, char** argv){
	sstatus.dwServiceType = SERVICE_WIN32;
	sstatus.dwCurrentState = SERVICE_START_PENDING;
	sstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	sstatus.dwWin32ExitCode = 0;
	sstatus.dwServiceSpecificExitCode = 0;
	sstatus.dwCheckPoint = 0;
	sstatus.dwWaitHint = 0;
	sstatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceHandler);
	if (sstatusHandle == 0) {
		DWORD nError = GetLastError();
	}
	// add your init code here

	// add your service thread here
	//fprintf(fp,"service onCreate\n");
	onCreate();

	//Initialization complete - report running status
	sstatus.dwCurrentState = SERVICE_RUNNING;
	sstatus.dwCheckPoint = 0;
	sstatus.dwWaitHint = 9000;
	if (!SetServiceStatus(sstatusHandle, &sstatus)) {
		DWORD nError = GetLastError();
	}
}

void WINAPI ServiceHandler(DWORD fdwControl) {
	sstatus.dwWin32ExitCode = 0;
	sstatus.dwCheckPoint = 0;
	sstatus.dwWaitHint = 0;
	switch (fdwControl) 
	{
		case SERVICE_CONTROL_CONTINUE:
			onResume();
			sstatus.dwCurrentState = SERVICE_RUNNING;
			break;
		case SERVICE_CONTROL_PAUSE:
			onStop();
			sstatus.dwCurrentState = SERVICE_PAUSED;
			break;
		case SERVICE_CONTROL_STOP:	
		case SERVICE_CONTROL_SHUTDOWN:
			onStop();
			sstatus.dwCurrentState = SERVICE_STOPPED;
			break;
		
		default:
			return;
	}
	if (!SetServiceStatus(sstatusHandle, &sstatus))
	{
		DWORD nError = GetLastError();
	}
}
//HANDLE hMainLoopThread[2];
//DWORD dwMainLoopTID[2];
void onCreate(){
	hMainLoopThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)TCPMain,
		(LPVOID)tcp_port,
		0,
		//NULL);
		&dwMainLoopTID[0]);
	hMainLoopThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)UDPFeedback,
		(LPVOID)udp_port,
		0,
		//NULL);
		&dwMainLoopTID[1]);
	if (hMainLoopThread[0]==NULL || hMainLoopThread[1]==NULL){
		log(_FATAL,"onCreate","can't init thread! WinNT:%d",GetLastError());
		ExitProcess(EXIT_FAILURE);
	}
}
void onResume(){
	onCreate();
}

void onStop(){
	if (sstatus.dwCurrentState == SERVICE_RUNNING){
		StopUDP(udp_port);
		
		HANDLE hT = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)StopTCP,
			(LPVOID)tcp_port,
			0,
			NULL);
			
		Sleep(1000);
		if (hMainLoopThread[0]!=NULL) TerminateThread(hMainLoopThread[0],0);
		if (hMainLoopThread[1]!=NULL) TerminateThread(hMainLoopThread[1],0);
		if (hT != NULL)				  TerminateThread(hT,0);
		if (hMainLoopThread[0]!=NULL) CloseHandle(hMainLoopThread[0]);
		if (hMainLoopThread[1]!=NULL) CloseHandle(hMainLoopThread[1]);
		if (hT != NULL)				  CloseHandle(hT);
	}
}
