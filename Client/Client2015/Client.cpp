#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 512

#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"

#define MSG_CHAT 2001
struct PACKET_MSG_CHAT : PACKET_HEADER
{
	char msg[50];
	int msg_size;

	PACKET_MSG_CHAT(int _from, int _to, char* _msg):PACKET_HEADER(_from, _to)
	{
		size = sizeof(*this);

		msg1 = MSG_CHAT;

		RemoveNewLineCharacter(_msg);
		msg_size = strlen(msg);
		strcpy(msg,_msg);		
	}
};

#define MSG_STRING_ID 2002
#define MAX_CLIENT 500
#define MAX_STRING_ID 100
char strID[MAX_CLIENT][MAX_STRING_ID];
char tempStrID[MAX_STRING_ID];
struct PACKET_MSG_STRING_ID : PACKET_MSG_ID
{
	char strID[MAX_STRING_ID];
	int nIsFromNewClient;

	PACKET_MSG_STRING_ID(int _from, int _to, 
						int id, char* strID, int _nIsFromNewClient)
		:PACKET_MSG_ID(_from, _to, id)
	{
		size = sizeof(*this);

		msg1 = MSG_STRING_ID;

		strcpy(this->strID, strID);

		nIsFromNewClient=_nIsFromNewClient;
	}
};

void OnConnected()
{

}

char sendingBuf[BUFSIZE+1];

void OnSendingActivated()
{
	if(fgets(sendingBuf, BUFSIZE+1, stdin) == NULL)
		return;

	RemoveNewLineCharacter(sendingBuf);

	switch(sendingBuf[0])
	{
	case '#':{//To server
		PACKET_MSG_CHAT chat(-1,TARGET_SERVER,sendingBuf+1);
		SendMessage((PACKET_HEADER*)&chat);
	}
	break;
	case '@':{//To me only (To a specific client)
		PACKET_MSG_CHAT chat(-1,TARGET_SENDING_CLIENT,sendingBuf+1);
		SendMessage((PACKET_HEADER*)&chat);
	}
	break;
	default:{//To all
		PACKET_MSG_CHAT chat(-1,TARGET_ALL,sendingBuf);
		SendMessage((PACKET_HEADER*)&chat);
	}
	break;
	}
}

char receivingBuf[BUFSIZE+1];
int receivingSize;

int nMyID=-2;
void OnReceivingActivated()
{
	PACKET_HEADER* pMsg=ReceiveMessage();
	if(pMsg==NULL)
		return;
	
	printf("[OnReceivingActivated] %d\n", pMsg->msg1);

	switch(pMsg->msg1)
	{
	case MSG_ID:
	{
		
		PACKET_MSG_ID* pID = (PACKET_MSG_ID*)pMsg;

		if(pID->from==TARGET_SERVER)
		{	
			nMyID = pID->id; 
			printf("Client has ID:%d\n", pID->id); 
		}
	}
	break;
	case MSG_NEW_CLIENT:
	{
		PACKET_MSG_NEW_CLIENT* pID = (PACKET_MSG_NEW_CLIENT*)pMsg;
		printf("New Client(%d) has entered.\n", pID->id); 

		strcpy(strID[nMyID], tempStrID);


		PACKET_MSG_STRING_ID msg(nMyID,pID->id, nMyID, strID[nMyID],0);
		SendMessage(&msg);
	}
	break;
	case MSG_CHAT:
	{
		PACKET_MSG_CHAT* pChat = (PACKET_MSG_CHAT*)pMsg;
		printf("%s\n", pChat->msg); 
	}
	break;
	case MSG_STRING_ID:
	{
		PACKET_MSG_STRING_ID* pStringID 
			= (PACKET_MSG_STRING_ID*)pMsg;

		strcpy(strID[pStringID->id], pStringID->strID);

		printf("%s님이 입장했습니다.",strID[pStringID->id]);

		if(pStringID->nIsFromNewClient==0)
		{
			PACKET_MSG_STRING_ID msg(nMyID,pMsg->from, nMyID, tempStrID,1);
			SendMessage(&msg);
		}
	}
	break;
	}
}

void OnDisconnected()
{

}

int main(int argc, char* argv[])
{
	printf("Client is started.\nID:");
	
	fgets(tempStrID, MAX_STRING_ID, stdin);

	RemoveNewLineCharacter(tempStrID);
	
	Start("127.0.0.1",9000, OnConnected, OnSendingActivated, OnReceivingActivated, OnDisconnected);
	return 0;
}