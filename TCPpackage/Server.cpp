    #include "CServerSocket.h"  
    #include <string.h>  
      
    int main(int argc, char *argv[])  
    {  
        CServerSocket server;  
        if(server.Bind())
		{
			printf("Bind success");
		}  
		if(server.Listen())
		{
			printf("Listening...\n");
			printf("Ready for Accept, Waiting...\n");
		}
		
		while(server.Accept())  
        {	
		    pid_t pid=fork();//父进程建立套接字的连接之后，创建子进程用于通信
		    if(pid<0)
		           perror("fork error\n");
		    if(!pid)//创建新的子进程,用于发送数据
		    {
				char buf[MAXBYTEMUN];
		        while(fgets(buf,sizeof(buf),stdin)!=NULL)
		        {
		            server.Send(buf,strlen(buf));
		            memset(buf,0,sizeof(buf));
		        }
		        exit(EXIT_SUCCESS);
		    }
		    else   //f=父进程用于接收数据
		    {
		        char recvbuf[MAXBYTEMUN];
				while(true)
				{
					memset(recvbuf, 0, sizeof recvbuf);
					server.Recv(recvbuf,sizeof(recvbuf),0);
				}
		        exit(EXIT_SUCCESS);
		    }
		}
        return 0;  
    }  
