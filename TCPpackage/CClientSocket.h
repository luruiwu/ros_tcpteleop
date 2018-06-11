#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>      //*进程用的头文件*/
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream> 
#include <fcntl.h>  
#include <netdb.h>  
#include <netinet/tcp.h>  

#define MAXBYTEMUN  50
class CClientSocket
{
public:
    CClientSocket();//参数初始化
    virtual ~CClientSocket();
	bool Connect();//建立连接
	bool Send(char* sendbuf, int len);//发送数据
	bool Recv(char* recvbuf, int len);//接受数据
private:
	bool Addr_Init();
	bool Close();
public:
	uint server_port;
	char* server_host;
private:
	int client_sockfd;
	struct sockaddr_in server_addr;
};

CClientSocket::CClientSocket():
  server_port(8000),
  server_host("127.0.0.1")
{
	/*创建套接字*/
	if((client_sockfd = ::socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("creat socket failed");
        exit(1);
	}
	/*服务器地址参数*/
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(server_port);
    server_addr.sin_addr.s_addr=inet_addr(server_host);
    bzero(&server_addr.sin_zero,sizeof(struct sockaddr_in));//bzero位清零函数，将sin_zero清零，sin_zero为填充字段，必须全部为零
}

CClientSocket::~CClientSocket()
{
	if(!Close())
	{
		perror("close failed");
		exit(1);
	}
}

bool CClientSocket::Close()
{
	if(client_sockfd < 0)
		return false;
	else
	{
		::close(client_sockfd);
		client_sockfd = -1;
	}
		
	return true;
}

bool CClientSocket::Connect()
{
	char buf[MAXBYTEMUN];
 	int numbytes;
	if(::connect(client_sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
    {
        perror("connect failed");
        return false;
    }
    if((numbytes=::recv(client_sockfd,buf,MAXBYTEMUN,0))==-1)
        {
            perror("receive failed");
            return false;
        }
    buf[numbytes]='\0';//在字符串末尾加上\0，否则字符串无法输出
    printf("Received: %s\n",buf);

	return true;
}

bool CClientSocket::Send(char* sendbuf, int len)
{
	ssize_t ret;
	//do{
		ret = ::send(client_sockfd, sendbuf, len, 0);
		if(ret<0)
		{
			perror("send error");
		}
		
	//} while (ret < 0 && (errno == EAGAIN || errno == EINTR));

    return ret == len;
}

bool CClientSocket::Recv(char* recvbuf, int len)
{
	int ret = 0;
	do{
		ret = ::recv(client_sockfd, recvbuf, len, 0);
		if(ret==0)
		{
			perror("peer closed\n");
			break;
		}
		else if(ret<0)
			perror("read from server error");
	} while (ret < 0 && (errno == EAGAIN || errno == EINTR));
    return ret == len; 
}

