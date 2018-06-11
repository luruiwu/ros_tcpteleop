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

#define MAXBYTEMUN  120
#define MyPort 8000

class CServerSocket
{
public:
    CServerSocket();
    virtual ~CServerSocket();

	bool Bind();
	bool Accept();
    bool Listen();
	bool Send(char* sendbuf, int len);
	bool Recv(char* recvbuf, int len);
private:
	bool Close();
private:
	uint server_port;
	int server_sockfd;//监听
	int connect_sockfd;//连接
	struct sockaddr_in server_addr;//服务器的地址信息
	struct sockaddr_in client_addr;//客户机的地址信息
};

CServerSocket::CServerSocket():
  server_port(-1),
  server_sockfd(-1),
  connect_sockfd(-1)
{
	if((server_sockfd = ::socket(AF_INET, SOCK_STREAM, 0))==-1)
	{
		perror("creat socket failed");
        exit(1);
	}
	server_port = 8000;
	 /*服务器地址参数*/
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(server_port);
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bzero(&server_addr.sin_zero,sizeof(struct sockaddr_in));//bzero位清零函数，将sin_zero清零，sin_zero为填充字段，必须全部为零
}

CServerSocket::~CServerSocket()
{
	if(!Close())
	{
		perror("close failed");
		exit(1);
	}
}

bool CServerSocket::Close()
{
	//关闭套接字
	if(server_sockfd)
		close(server_sockfd);
	else
		return false;
	if(connect_sockfd)
	    close(connect_sockfd);
	else
		return false;
	server_sockfd = connect_sockfd = server_port = -1;
	return true;
}

bool CServerSocket::Bind()
{
	int on=1; //表示开启reuseaddr
    if(setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)//打开地址、端口重用
        perror("setsockopt");

	/*绑定地址和端口*/
    if(bind(server_sockfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
    {
        perror("bind failed");
        return false;
    }
	return true;
}

bool CServerSocket::Listen()
{
	//监听
	if(::listen(server_sockfd, 5) == -1)
	{
		perror("listen failed");
		return false;
	}
	return true;
}

bool CServerSocket::Accept()
{
	socklen_t addrlen = sizeof(struct sockaddr_in);
	connect_sockfd = ::accept(server_sockfd, (struct sockaddr*)&client_addr, &addrlen);
	if(connect_sockfd == -1)
	{
		perror("accpet failed");
		return false;
	}
	printf("Get Connect From IP= %s prot= %d \n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
	char hello[30] = "Hello client,I am server!\n";
    if(send(connect_sockfd,hello,30,0)==-1)
    	perror("send failed");
	return true;
}

bool CServerSocket::Send(char *sendbuf, int len)
{
	if(connect_sockfd < 0 || sendbuf==NULL || len < 0)
	{
		perror("sendbuf error");
		return false;
	}
	ssize_t ret;
	ret = ::send(connect_sockfd, sendbuf, len, 0);
	if(ret<0)
	{
		perror("send error");
	}
	return ret == len;
}

bool CServerSocket::Recv(char *recvbuf, int len)
{
	if(connect_sockfd < 0)
		return false;
	if(recvbuf==NULL || len < 0)
		perror("sendbuf error");
	ssize_t ret = 0;
	ret = ::recv(connect_sockfd, recvbuf, len, 0);
	if(ret==0)
	{
		perror("peer closed");
		return false;
	}
	else if(ret<0)
		perror("read from server error");
    return ret == len;
}
