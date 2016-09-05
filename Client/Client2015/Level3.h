void SendMessage(int size, char* buf)
{
	Send(sizeof(int), (char*)&size);

	Send(size, buf);
}

void ReceiveMessage(int* size, char* buf)
{
	
	Receive(sizeof(int), (char*)size);
	Receive(*size, buf);
}

void RemoveNewLineCharacter(char* buf)
{
	if(buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';
}

void Start(char* ip, int port, void(*pfConnected)(), void(*pfSendActivate)(), 
		   void(*pfReceiveActivate)(), void(*pfDisconnected)())
{
	InitWSA();
	InitClientSocket(ip,port);
	Run(pfConnected, pfSendActivate, pfReceiveActivate, pfDisconnected);
	CloseClientSocket();
	CleanupWSA();
}