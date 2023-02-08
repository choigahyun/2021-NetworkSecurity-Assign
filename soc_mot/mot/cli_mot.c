#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define  LINE 1024

DWORD WINAPI ProcessInputSend(LPVOID arg);

char userID[10];		// 유저ID
char line[LINE], chatdata[LINE + 1];
struct sockaddr_in server_addr;
SOCKET serversocket;	// 서버 연결 소켓
char* escape = "/quit";	// 종료 명령
BOOL	bIsQuit;		// 종료 flag

char message[300];
int strLen;

void main()
{
	WSADATA wsa; // 소켓
	HANDLE hThread; // 스레드
	DWORD ThreadId;

	bIsQuit = FALSE;

	printf("Input ID : ");	// 유저ID 입력
	gets(userID);

	// 소켓 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit(1);
	}
	// 소켓 생성
	if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("fail make socket\n");
		exit(0);
	}
	// 메모리 초기화
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // 소켓 인자들 설정
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//	server_addr.sin_addr.s_addr = inet_addr("192.168.100.211");
	server_addr.sin_port = htons(8787);

	// 서버와 연결
	if (connect(serversocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("fail connect 2 server\n");
		exit(0);
	}
	else {
		printf("connected server\n");
	}
	// 스레드 생성
	hThread = CreateThread(NULL, 0, ProcessInputSend, 0, 0, &ThreadId);
	if (hThread == NULL) {
		printf("fail make thread\n");
	}
	else {
		CloseHandle(hThread);
	}

	while (!bIsQuit) {
		while (1) {
			send(serversocket, "POLLING", 8, 0);
			strLen = recv(serversocket, message, sizeof(message) - 1, 0);
			printf("%s\n", message);
			Sleep(10000);
		}
	}

	closesocket(serversocket);
	WSACleanup();
}

/* 사용자 입력 */
DWORD WINAPI ProcessInputSend(LPVOID arg)
{
	while (TRUE) {
		if (fgets(chatdata, LINE, stdin)) {		// 스트림 read
			chatdata[strlen(chatdata) - 1] = '\0';
			sprintf(line, "%s", chatdata);		// 버퍼의 첫 내용을 출력
			if (send(serversocket, line, strlen(line), 0) < 0) {
				printf("Write fail\n");
			}
		}
	}
	return 0;
}
