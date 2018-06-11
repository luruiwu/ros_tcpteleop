#include "CClientSocket.h"  
#include "joystick.h"

#define MAXBYTEMUN  50

int main(int argc, char *argv[])  
{
	//if(argc!=2)
    //{
    //    fprintf(stderr,"usage: server IPAddress\n");
    //    exit(1);
    //}
	CClientSocket client;
	client.server_host = argv[1];
	client.Connect();

	pid_t pid;
	pid = fork();
	if(!pid)//创建新的子进程,用于接收数据
	{
		char recvbuf[MAXBYTEMUN];		
		while(true)
		{
			client.Recv(recvbuf,sizeof(recvbuf))
			fputs(recvbuf,stdout);
			memset(recvbuf,0,sizeof(recvbuf));
		}
	}
	else//父进程用于发送数据
	{
		char buf[MAXBYTEMUN];
		while(fgets(buf,sizeof(buf),stdin)!=NULL)
		{
			client.Send(buf,sizeof(buf));
			memset(buf,0,sizeof(buf));
		} 
	}
	client.~CClientSocket();
	return 0;
}
