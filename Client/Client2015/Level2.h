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

SOCKET sock;

int InitClientSocket(char* ip, int port)
{
	int retval;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) err_quit("socket()");	
	
	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	serveraddr.sin_addr.s_addr = inet_addr(ip);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("connect()");

	return 0;
}

void CloseClientSocket()
{
	// closesocket()
	closesocket(sock);
}

void Send(int size, char* buf)
{
	if(size == 0)
		return;

	// 데이터 보내기
	int retval = send(sock, buf, size, 0);
	if(retval == SOCKET_ERROR){
		err_display("send()");
	}
}

int Receive(int retval, char* buf)
{
	retval = recvn(sock, buf, retval, 0);
	if(retval == SOCKET_ERROR){
		err_display("recv()");	
		buf[0] = '\0';
		return 0;
	}
	else if(retval == 0) {	
		buf[0] = '\0';
		return 0;
	}
		
	// 받은 데이터 출력
	//buf[retval] = '\0';

	return retval;
}

void(*gpfReceiveActivate)();

DWORD WINAPI ProcessClient(LPVOID arg)
{
	while(1)
	{
		gpfReceiveActivate();
	}
}

void Run(void(*pfConnected)(), void(*pfSendActivate)(), void(*pfReceiveActivate)(), void(*pfDisconnected)())
{
	pfConnected();

	gpfReceiveActivate = pfReceiveActivate;

	if(pfReceiveActivate!=NULL)
	{
		DWORD ThreadId;
		HANDLE hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)pfReceiveActivate, 0, &ThreadId);
		if(hThread == NULL)
			printf("[오류] 스레드 생성 실패!\n");
		else
			CloseHandle(hThread);
	}

	while(1)
	{
		pfSendActivate();
	}

	pfDisconnected();
}