#define MAX_CLIENT 500
#define NONE_SOCKET 0

SOCKET gClients[MAX_CLIENT];

void(*gpfUserConnected)(SOCKET client_sock, char* ip, int port, int index);
void(*gpfUserDisconnected)(SOCKET client_sock, char* ip, int port, int index);

void InitLevel3()
{
	for(int i = 0; i <MAX_CLIENT; ++i)
	{
		gClients[i] = NONE_SOCKET;
	}
}

void CloseLevel3()
{
	for(int i = 0; i <MAX_CLIENT; ++i)
	{
		if(gClients[i] == NONE_SOCKET)
		{
			closesocket(gClients[i]);
			gClients[i] = NONE_SOCKET;
		}
	}
}

void OnLevel3ClientConnected(SOCKET client_sock, char* ip, int port)
{
	for(int i = 0; i< MAX_CLIENT; ++i)
	{
		if(gClients[i] == NONE_SOCKET){
			gClients[i] = client_sock;
			gpfUserConnected(client_sock,ip,port,i);
			break;
		}
	}
}

void OnLevel3ClientDisconnected(SOCKET client_sock, char* ip, int port)
{
	for(int i = 0; i< MAX_CLIENT; ++i)
	{
		if(gClients[i] == client_sock)
		{
			gpfUserDisconnected(client_sock,ip,port,i);
			gClients[i] = NONE_SOCKET;
			break;
		}
	}
}

void SendMessage(SOCKET client_socket, int size, char* buf)
{
	Send(client_socket, sizeof(int), (char*)&size);

	Send(client_socket, size, buf);
}

void SendBroadcastMessage(int size, char* buf)
{
	for(int i = 0; i < MAX_CLIENT; ++i)
	{
		if(gClients[i] != NONE_SOCKET){
			SendMessage(gClients[i], size, buf);
		}
	}
}

BOOL ReceiveMessage(SOCKET client_socket, int* size, char* buf)
{	
	int nRec = Receive(client_socket,  sizeof(int), (char*)size);

	if(nRec!=0) 
	{
		Receive(client_socket, *size, buf);
		return true;
	}

	return false;
}

void Start(void(*pfConnected)(SOCKET client_sock, char* ip, int port, int index), BOOL(*pfActivate)(SOCKET client_sock), 
		 void(*pfDisconnected)(SOCKET client_sock, char* ip, int port, int index))
{

	InitWSA();
	InitServerSocket();

	InitLevel3();

	gpfUserConnected = pfConnected;
	gpfUserDisconnected = pfDisconnected;

	Run(OnLevel3ClientConnected, pfActivate, OnLevel3ClientDisconnected);

	CloseLevel3();


	CloseServerSocket();
	CleanupWSA();
}