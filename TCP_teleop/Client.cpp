#include "CClientSocket.h"
#include "Joystick.h"
#include "prot.h"
int main(int argc, char *argv[])  
{
	CClientSocket client;
	client.server_host = argv[1];
	while(!client.Connect());

	pid_t pid;
	pid = fork();
	if(!pid)//创建新的子进程,用于接收数据
	{
		int event_count_=0;            
		char recvbuf[MAXBYTEMUN];
		struct Joy Joy_msg;
		//close(new_fd);//父进程不需要连接，所以关闭连接套接字
		while(1)
		{
			memset(recvbuf,0,sizeof(recvbuf));
			client.Recv(recvbuf,sizeof(struct Joy));
			event_count_++;
		  
			memset(&Joy_msg,0,sizeof(struct Joy));
			memcpy(&Joy_msg,recvbuf,sizeof(recvbuf));

			printf("---\n");
			printf("axes:[%f,%f,%f,%f]\n",Joy_msg.axes[0],Joy_msg.axes[1],Joy_msg.axes[2],Joy_msg.axes[3]);
			printf("button:[%d",Joy_msg.buttons[0]);
			for(int i=1;i<17;i++)
			printf(" %d",Joy_msg.buttons[i]);
			printf("]\n");
		}
		exit(EXIT_SUCCESS); 
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
