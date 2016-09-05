#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"

//#include "string.h"

#define MSG_CHAT 2001
struct PACKET_MSG_CHAT : PACKET_HEADER
{
	char msg[50];
	int msg_size;

	PACKET_MSG_CHAT(int _from, int _to, char* _msg):PACKET_HEADER(_from, _to)
	{
		size = sizeof(*this);

		msg1 = MSG_CHAT;
		msg_size = strlen(msg);

		strcpy(msg,_msg);		
	}
};

#define MSG_STING_ID 2002
#define MAX_STRING_ID 100
struct PACKET_MSG_STING_ID : PACKET_MSG_ID
{
	char strID[MAX_STRING_ID];

	PACKET_MSG_STING_ID(int _from, int _to, 
						int id, char* strID)
		:PACKET_MSG_ID(_from, _to, id)
	{
		size = sizeof(*this);

		msg1 = MSG_STING_ID;

		strcpy(this->strID, strID);
	}
};

void OnClientConnected(SOCKET client_sock, char* ip, int port, int index)
{
	printf("Client(%s:%d) is entered.\n",  ip, port);
}

BOOL OnClientActivated(SOCKET client_sock)
{
	PACKET_HEADER* pMsg=ReceiveMessage(client_sock);
	if(pMsg==NULL)
		return false;

	printf("[OnClientActivated] %d\n", pMsg->msg1);

	switch(pMsg->msg1)
	{
	case MSG_CHAT:
	{
		PACKET_MSG_CHAT* pChat = (PACKET_MSG_CHAT*)pMsg;
 		pChat->msg[pChat->msg_size] = 0;	
		printf("%s\n", pChat->msg); 
	
		SendBackMessageToTarget(client_sock, pChat);
	}
	break;
	default:
		if(pMsg->to!=TARGET_SERVER)
 			SendMessageToOne(pMsg->to,pMsg);
		break;
	}

	return true;
}

void OnClientDisconnected(SOCKET client_sock, char* ip, int port, int index)
{
	printf("Client(%s:%d) is exited.\n",  ip, port);
}

int main(int argc, char* argv[])
{
	printf("Server is started.\n");

	StartWithPacket(OnClientConnected, OnClientActivated, OnClientDisconnected);

	 return 0;
}