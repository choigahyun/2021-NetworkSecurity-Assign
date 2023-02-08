#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define  LINE 1024

DWORD WINAPI ProcessInputSend(LPVOID arg);

char userID[10];		// ����ID
char line[LINE], chatdata[LINE + 1];
struct sockaddr_in server_addr;
SOCKET serversocket;	// ���� ���� ����
char* escape = "/quit";	// ���� ���
BOOL	bIsQuit;		// ���� flag

char message[300];
int strLen;

void main()
{
	WSADATA wsa; // ����
	HANDLE hThread; // ������
	DWORD ThreadId;

	bIsQuit = FALSE;

	printf("Input ID : ");	// ����ID �Է�
	gets(userID);

	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit(1);
	}
	// ���� ����
	if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("fail make socket\n");
		exit(0);
	}
	// �޸� �ʱ�ȭ
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // ���� ���ڵ� ����
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//	server_addr.sin_addr.s_addr = inet_addr("192.168.100.211");
	server_addr.sin_port = htons(8787);

	// ������ ����
	if (connect(serversocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("fail connect 2 server\n");
		exit(0);
	}
	else {
		printf("connected server\n");
	}
	// ������ ����
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

/* ����� �Է� */
DWORD WINAPI ProcessInputSend(LPVOID arg)
{
	while (TRUE) {
		if (fgets(chatdata, LINE, stdin)) {		// ��Ʈ�� read
			chatdata[strlen(chatdata) - 1] = '\0';
			sprintf(line, "%s", chatdata);		// ������ ù ������ ���
			if (send(serversocket, line, strlen(line), 0) < 0) {
				printf("Write fail\n");
			}
		}
	}
	return 0;
}
