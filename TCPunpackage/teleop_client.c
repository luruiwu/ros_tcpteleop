#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include<sys/wait.h>      //*进程用的头文件*/
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAXBYTEMUN	50
#define MyPort 8888
int main(int argc,char *argv[])
{
	int sock_fd,numbytes;
	char buf[MAXBYTEMUN];
	struct sockaddr_in client_addr;//客户机的地址信息
	ssize_t n;
	/*执行客户端程序时，输入客户端程序名称和其IP地址*/
	if(argc!=2)
	{
		fprintf(stderr,"usage: client IPAddress\n");
		exit(1);
	}
	/*struct sockaddr_in their_addr;*/
	printf("break!");
	/*创建套接字*/
	sock_fd=socket(AF_INET,SOCK_STREAM,0);//采用IPv4协议
	if(sock_fd==-1)
	{
		perror("creat socket failed");
		exit(1);
	}
	//while((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1);
	printf("We get the sockfd~\n");

	/*服务器地址参数*/
	client_addr.sin_family=AF_INET;  
	client_addr.sin_port=htons(MyPort);
	client_addr.sin_addr.s_addr=inet_addr(argv[1]);
	bzero(&client_addr.sin_zero,sizeof(struct sockaddr_in));//bzero位清零函数，将sin_zero清零，sin_zero为填充字段，必须全部为零
	/*连接到服务器*/
	if(connect(sock_fd,(struct sockaddr*)&client_addr,sizeof(struct sockaddr))==-1)
	{
		perror("connect failed");
		exit(1);
	}
	if((numbytes=recv(sock_fd,buf,MAXBYTEMUN,0))==-1)
	{       
		perror("receive failed");
		exit(1);
	}
	buf[numbytes]='\0';//在字符串末尾加上\0，否则字符串无法输出
	printf("Received: %s\n",buf);

	/*while(connect(sockfd,(struct sockaddr*)&their_addr,sizeof(struct sockaddr)) == -1);
	 *printf("Get the Server~Cheers!\n");
	 */
	pid_t pid;
	pid=fork();
	if(!pid)//创建新的子进程,用于接收数据
	{
		char recvbuf[MAXBYTEMUN];
		while(1)
		{
			memset(recvbuf,0,sizeof(recvbuf));
			n=recv(sock_fd,recvbuf,MAXBYTEMUN,0);
			if(n==0)
			{
				printf("peer closed\n");
				break;
			}
			else if(n<0)
				perror("read from server error");    
			fputs(recvbuf,stdout);
		}
		exit(EXIT_SUCCESS);
		close(sock_fd);
	}
	else   //f=父进程用于发送数据
	{
		//close(sock_fd);//父进程不需要连接，所以关闭连接套接字
		while(fgets(buf,sizeof(buf),stdin)!=NULL)
		{
			send(sock_fd,buf,MAXBYTEMUN,0);
			memset(buf,0,sizeof(buf));   //清空，以免和下一次混淆
		}
		exit(EXIT_SUCCESS);
		close(sock_fd);
	}
	return 0;
}
