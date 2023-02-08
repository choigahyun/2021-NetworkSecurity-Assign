#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE		1024
#define MAXCLIENT	5
#define	SERV_PORT	8787

DWORD WINAPI ProcessInputSend(LPVOID arg);

char userID[10];		// 유저ID
char line[LINE], chatdata[LINE + 1];
struct sockaddr_in server_addr;
SOCKET serversocket;	// 서버 연결 소켓
BOOL	bIsQuit;		// 종료 flag

int CurTem = 25;
int UpperBound = 30;
int LowerBound = -5; 
char teminfo[300];

char message[300];
int strLen;

int maxfdp1;
int chatuser;	// 채팅 참가자 수
unsigned int client_s[MAXCLIENT];
char* escape = "/quit";	// 종료 명령

void ErrorHandling(char* message);

/* i번째 유저 계정삭제 */
void disconCli(int i)
{
	closesocket(client_s[i]);	// i번째 소켓 닫음
	if (i == chatuser - 1) {
		client_s[i] = client_s[chatuser - 1];
		chatuser--;		// 총유저수 줄임
		printf("Now On the net %d users\n", chatuser);
	}
}
/* 현재 채팅 참가자수를 검색 */
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
	char* start = "Now you connected Server\n";		// 접속알림
	int	i, n, addrlen;
	SOCKET serversocket, clientsocket, clilen;		// 소켓 변수
	unsigned short	port;
	fd_set read_fds;
	struct sockaddr_in clientd_addr, server_addr;	// 소켓 주소
	WSADATA wsa;		// 윈도우즈 소켓선언
	int	iError;

	// 소켓 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {		
		exit(1);
	}
	// 소켓 생성
	if ((serversocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		
		printf("fail make socket\n");
		exit(0);
	}

	port = SERV_PORT;
	ZeroMemory(&server_addr, sizeof(server_addr));		// 초기화
	server_addr.sin_family = AF_INET;			// 소켓 옵션 지정
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(serversocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("fail bind\n");
		exit(0);
	}

	listen(serversocket, SOMAXCONN);	// 연결 대기, SOMAXCONN : 최대 연결수
	maxfdp1 = serversocket + 1;			// 최대소켓 + 1

	while (1)
	{
		FD_ZERO(&read_fds);		// 파일 디스크립터 초기화
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
			// 파일디스크립터에 읽을 데이터가 있으면
			addrlen = sizeof(clientd_addr); // 주소길이 지정
			// 소켓 연결
			clilen = sizeof(clientd_addr);
			clientsocket = accept(serversocket, (struct sockaddr*)&clientd_addr, &addrlen);

			if (clientsocket == -1) {
				printf("fail accept\n");
				exit(0);
			}

			client_s[chatuser] = clientsocket;
			chatuser++;

			send(clientsocket, start, strlen(start), 0);
			printf("%d user connected\n", chatuser);		// 유저접속알림
		}
		

		// 브로드 캐스팅
		for (i = 0; i < chatuser; i++) {		// 접속해 있는 유저수만큼

			memset(readline, '\0', LINE);
			if (FD_ISSET(client_s[i], &read_fds)) {		// 파일 디스크립터 버퍼 확인
				if ((n = recv(client_s[i], readline, LINE, 0)) <= 0) {
					disconCli(i);
					continue;
				}

				if (strstr(readline, escape)) {		// FIXME : 사용자 삭제를 위한 키워드 처리 수정
					disconCli(i); // 유저 계정 삭제
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


