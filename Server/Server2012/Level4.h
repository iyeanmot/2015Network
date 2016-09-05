#define TARGET_SERVER -1
#define TARGET_ALL -2
#define TARGET_SENDING_CLIENT -3

struct PACKET_HEADER
{
	char head[4];
	int size;
	int from;
	int to;

	int msg1;
	int msg2;
	int msg3;
	
	PACKET_HEADER(int _from, int _to)
	{
		head[0]='H';
		head[1]='E';
		head[2]='A';
		head[3]='D';

		//size = sizeof(this);

		from = _from;
		to = _to;
	}
};

#define MSG_ID 1001
struct PACKET_MSG_ID : PACKET_HEADER
{
	int id;

	PACKET_MSG_ID(int _from, int _to, int _id):PACKET_HEADER(_from, _to)
	{
		size = sizeof(*this);

		msg1 = MSG_ID;

		this->id = _id;
	}
};

#define MSG_NEW_CLIENT 1002
struct PACKET_MSG_NEW_CLIENT : PACKET_HEADER
{
	int id;

	PACKET_MSG_NEW_CLIENT(int _from, int _to, int _id):PACKET_HEADER(_from, _to)
	{
		size = sizeof(*this);

		msg1 = MSG_NEW_CLIENT;

		this->id = _id;
	}
};

void(*gpfLevel4UserConnected)(SOCKET client_sock, char* ip, int port, int index);
void(*gpfLevel4UserDisconnected)(SOCKET client_sock, char* ip, int port, int index);

void SendMessage(SOCKET client_socket, PACKET_HEADER* msg)
{
	Send(client_socket, msg->size, (char*)msg);
}

void SendMessageToOne(int nIndex, PACKET_HEADER* msg)
{
	if(gClients[nIndex] != NONE_SOCKET){
		SendMessage(gClients[nIndex], msg);
	}
}

void SendBroadcastMessage(PACKET_HEADER* msg)
{
	for(int i = 0; i < MAX_CLIENT; ++i)
	{
		if(gClients[i] != NONE_SOCKET){
			SendMessage(gClients[i], msg);
		}
	}
}

void SendBroadcastMessageExceptOne(SOCKET client_socket, PACKET_HEADER* msg)
{
	for(int i = 0; i < MAX_CLIENT; ++i)
	{
		if(gClients[i] != NONE_SOCKET && gClients[i]!=client_socket){
			SendMessage(gClients[i], msg);
		}
	}
}

void SendMessage(PACKET_HEADER* msg)
{
	for(int i = 0; i < MAX_CLIENT; ++i)
	{
		if(msg->to==i){
			SendMessage(gClients[i], msg);
			return;
		}
	}
}

void SendBackMessageToTarget(SOCKET client_sock, PACKET_HEADER* msg)
{
	switch(msg->to)
	{
		case TARGET_ALL:
			SendBroadcastMessage(msg);
			break;
		case TARGET_SERVER://Do not send anymore.
			break;
		case TARGET_SENDING_CLIENT:
			SendMessage(client_sock, msg);
			break;
		default:
			break;
	}
}
char buf[BUFSIZE+1];
int size;
		
PACKET_HEADER* ReceiveMessage(SOCKET client_socket)
{
	Receive(client_socket,sizeof(PACKET_HEADER),buf);

	char* pSendMeg = buf+sizeof(PACKET_HEADER);
	PACKET_HEADER* header = (PACKET_HEADER*)buf;

	Receive(client_socket, header->size-sizeof(PACKET_HEADER),pSendMeg);



	return (PACKET_HEADER*)buf;
}

void OnLevel4ClientConnected(SOCKET client_sock, char* ip, int port, int index)
{
	PACKET_MSG_ID msg(TARGET_SERVER, index, index);
	SendMessage(client_sock,&msg);
	
	PACKET_MSG_NEW_CLIENT msg2(TARGET_SERVER, TARGET_ALL, index);
	SendBroadcastMessageExceptOne(client_sock, &msg2);

	gpfLevel4UserConnected(client_sock,ip,port,index);
}

void OnLevel4ClientDisconnected(SOCKET client_sock, char* ip, int port, int index)
{
	gpfLevel4UserConnected(client_sock,ip,port,index);
}

void StartWithPacket(void(*pfConnected)(SOCKET client_sock, char* ip, int port, int index), BOOL(*pfActivate)(SOCKET client_sock), 
		 void(*pfDisconnected)(SOCKET client_sock, char* ip, int port, int index))
{
	gpfLevel4UserConnected = pfConnected;
	gpfLevel4UserDisconnected = pfDisconnected;

	Start(OnLevel4ClientConnected,pfActivate,OnLevel4ClientDisconnected);
}