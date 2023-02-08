#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE		1024
#define MAXCLIENT	5
#define	SERV_PORT	8787

DWORD WINAPI ProcessInputSend(LPVOID arg);

char userID[10];		// ����ID
char line[LINE], chatdata[LINE + 1];
struct sockaddr_in server_addr;
SOCKET serversocket;	// ���� ���� ����
BOOL	bIsQuit;		// ���� flag

int CurTem = 25;
int UpperBound = 30;
int LowerBound = -5; 
char teminfo[300];

char message[300];
int strLen;

int maxfdp1;
int chatuser;	// ä�� ������ ��
unsigned int client_s[MAXCLIENT];
char* escape = "/quit";	// ���� ���

void ErrorHandling(char* message);

/* i��° ���� �������� */
void disconCli(int i)
{
	closesocket(client_s[i]);	// i��° ���� ����
	if (i == chatuser - 1) {
		client_s[i] = client_s[chatuser - 1];
		chatuser--;		// �������� ����
		printf("Now On the net %d users\n", chatuser);
	}
}
/* ���� ä�� �����ڼ��� �˻� */
int maxuser(int user)
{
	unsigned int max = user;
	int i;

	for (i = 0; i < chatuser; i++) {
		if (max < client_s[i])
			max = client_s[i];
	}

	return max;
}

int main()
{
	char readline[LINE];	// read buffer
	char* start = "Now you connected Server\n";		// ���Ӿ˸�
	int	i, n, addrlen;
	SOCKET serversocket, clientsocket, clilen;		// ���� ����
	unsigned short	port;
	fd_set read_fds;
	struct sockaddr_in clientd_addr, server_addr;	// ���� �ּ�
	WSADATA wsa;		// �������� ���ϼ���
	int	iError;

	// ���� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {		
		exit(1);
	}
	// ���� ����
	if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		
		printf("fail make socket\n");
		exit(0);
	}

	port = SERV_PORT;
	ZeroMemory(&server_addr, sizeof(server_addr));		// �ʱ�ȭ
	server_addr.sin_family = AF_INET;			// ���� �ɼ� ����
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(serversocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("fail bind\n");
		exit(0);
	}

	listen(serversocket, SOMAXCONN);	// ���� ���, SOMAXCONN : �ִ� �����
	maxfdp1 = serversocket + 1;			// �ִ���� + 1

	while (1)
	{
		FD_ZERO(&read_fds);		// ���� ��ũ���� �ʱ�ȭ
		FD_SET(serversocket, &read_fds);
		for (i = 0; i < chatuser; i++) {
			FD_SET(client_s[i], &read_fds);
		}
		maxfdp1 = maxuser(serversocket) + 1;

		if (select(maxfdp1, &read_fds, (fd_set*)0, (fd_set*)0, NULL) < 0) {
			iError = WSAGetLastError();
			printf("fail select function\n");
			exit(0);
		}

		if (FD_ISSET(serversocket, &read_fds)) {
			// ���ϵ�ũ���Ϳ� ���� �����Ͱ� ������
			addrlen = sizeof(clientd_addr); // �ּұ��� ����
			// ���� ����
			clilen = sizeof(clientd_addr);
			clientsocket = accept(serversocket, (struct sockaddr*)&clientd_addr, &addrlen);

			if (clientsocket == -1) {
				printf("fail accept\n");
				exit(0);
			}

			client_s[chatuser] = clientsocket;
			chatuser++;

			send(clientsocket, start, strlen(start), 0);
			printf("%d user connected\n", chatuser);		// �������Ӿ˸�
		}
		

		// ��ε� ĳ����
		for (i = 0; i < chatuser; i++) {		// ������ �ִ� ��������ŭ

			memset(readline, '\0', LINE);
			if (FD_ISSET(client_s[i], &read_fds)) {		// ���� ��ũ���� ���� Ȯ��
				if ((n = recv(client_s[i], readline, LINE, 0)) <= 0) {
					disconCli(i);
					continue;
				}

				if (strstr(readline, escape)) {		// FIXME : ����� ������ ���� Ű���� ó�� ����
					disconCli(i); // ���� ���� ����
					continue;
				}
				
				if (strcmp(readline, "QUERY") == 0) {
					sprintf(teminfo, "CurTem : %d\nUpperBound : %d\nLowerBound : %d\nUPPER_BOUND or LOWER_BOUND", CurTem, UpperBound, LowerBound);
					send(client_s[i], teminfo, sizeof(teminfo), 0);
				}
				else if (strcmp(readline, "CONFIGURE UPPER_BOUND") == 0) {
					recv(client_s[i], readline, LINE, 0);
					UpperBound = atoi(readline);
					printf("change UpperBound : %d\n", UpperBound);
				}
				else if (strcmp(readline, "CONFIGURE LOWER_BOUND") == 0) {
					recv(client_s[i], readline, LINE, 0);
					LowerBound = atoi(readline);
					printf("change LowerBound : %d\n", LowerBound);
				}
				else if (strcmp(readline, "POLLING") == 0) {
					CurTem = refresh_temp(CurTem);
					if (CurTem< LowerBound || CurTem > UpperBound) {
						send(client_s[i], "warning", 8, 0);
					}
					else {
						send(client_s[i], "safe", 5, 0);
					}
				}

			}
		}
	}

	closesocket(serversocket);
	WSACleanup();
	return 0;
}

int refresh_temp(int curr_temp) {
	return curr_temp + rand() % 3 - 1;
}


