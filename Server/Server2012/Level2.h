int InitWSA()
{
	// 윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return -1;

	return 0;
}


void CleanupWSA()
{
	// 윈속 종료
	WSACleanup();
}

SOCKET listen_sock;

int InitServerSocket()
{
	// socket()
	int retval;
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");	
	
	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	return 0;
}

void CloseServerSocket()
{
	// closesocket()
	closesocket(listen_sock);
}

#define BUFSIZE 512

void(*Connected)(SOCKET client_sock, char* ip, int port);
void(*Disconnected)(SOCKET client_sock, char* ip, int port);
BOOL(*Activated)(SOCKET client_sock);

int Send(SOCKET client_sock, int size, char* buf)
{
	// 데이터 보내기
	int retval = send(client_sock, buf, size, 0);
	if(retval == SOCKET_ERROR){
		err_display("send()");
		return retval;
	}

	return retval;
}

int Receive(SOCKET client_sock, int size, char* buf)
{
	// 데이터 받기
	int retval = recv(client_sock, buf, size, 0);
	if(retval == SOCKET_ERROR){
		err_display("recv()");
		buf[0] = '\0';
		return 0;
	}
	else if(retval == 0){
		buf[0] = '\0';
		return 0;
	}
	
	//buf[retval] = '\0';
	return retval;
}

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen;
	
	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);
	Connected(client_sock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	while(Activated(client_sock));
	// closesocket()
	
	Disconnected(client_sock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(client_sock);

	return 0;
}

void Run(void(*pfConnected)(SOCKET client_sock, char* ip, int port), BOOL(*pfActivate)(SOCKET client_sock), 
		 void(*pfDisconnected)(SOCKET client_sock, char* ip, int port))
{
	Connected=pfConnected;
	Disconnected=pfDisconnected;
	Activated=pfActivate;

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;
	DWORD ThreadId;

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			continue;
		}

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient, 
			(LPVOID)client_sock, 0, &ThreadId);
		if(hThread == NULL)
			printf("[오류] 스레드 생성 실패!\n");
		else
			CloseHandle(hThread);
	}
}