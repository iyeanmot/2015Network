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

		//size = sizeof(*this);

		from = _from;
		to = _to;
		
		msg1 = -1;
		msg2 = -1;
		msg3 = -1;
	}
};

#define MSG_ID 1001
struct PACKET_MSG_ID : PACKET_HEADER
{
	int id;

	PACKET_MSG_ID(int _from, int _to, int id):PACKET_HEADER(_from, _to)
	{
		size = sizeof(*this);

		msg1 = MSG_ID;

		this->id = id;
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

void SendMessage(PACKET_HEADER* msg)
{
	Send(msg->size, (char*)msg);
}

char buf[BUFSIZE+1];
int size;

PACKET_HEADER* ReceiveMessage()
{
	Receive(sizeof(PACKET_HEADER),buf);

	char* pSendMeg = buf+sizeof(PACKET_HEADER);
	PACKET_HEADER* header = (PACKET_HEADER*)buf;

	Receive(header->size-sizeof(PACKET_HEADER),pSendMeg);

	return (PACKET_HEADER*)buf;
}