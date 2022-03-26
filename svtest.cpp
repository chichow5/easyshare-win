#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

//#define SERVICE_NAME TEXT("srv_demo")
char SERVICE_NAME[] = "serv_demo";
SERVICE_STATUS sstatus;
SERVICE_STATUS_HANDLE sstatusHandle;

HANDLE hMainLoopThead;
DWORD dwMainLoopTID;
HANDLE CreateMainLoop();

void WINAPI ServiceMain(int argc, char** argv);
void WINAPI ServiceHandler(DWORD fdwControl);

int main(int argc, const char** argv) {
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
	hMainLoopThread = CreateMainLoop(&dwMainLoopTID);

	//Initialization complete - report running status
	sstatus.dwCurrentState = SERVICE_RUNNING;
	sstatus.dwCheckPoint = 0;
	sstatus.dwWaitHint = 9000;
	if (!SetServiceStatus(sstatusHandle, &sstatus)) {
		DWORD nError = GetLastError();
	}
}

void WINAPI ServiceHandler(DWORD fdwControl) {
	switch (fdwControl) 
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			sstatus.dwWin32ExitCode = 0;
			sstatus.dwCurrentState = SERVICE_STOPPED;
			sstatus.dwCheckPoint = 0;
			sstatus.dwWaitHint = 0;
			//add your quit code here
			break;

		default:
			return;
	}
	if (!SetServiceStatus(sstatusHandle, &sstatus))
	{
		DWORD nError = GetLastError();
	}
}
DWORD WINAPI ThreadProc(LPVOID lpParameter){
	
}

HANDLE CreateMainLoop(DWORD *TID){
	HANDLE hThread = CreateThread(NULL,0,ThreadProc,NULL,0,TID);
	if (hThread == NULL){
		fprintf(stderr,"[error](init thread)can't create main loop thread");
	}
	return hThread;
}

