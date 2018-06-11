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

class CServerSocket
{
public:
    CServerSocket();
    virtual ~CServerSocket();

	bool Bind();
	bool Accept();
    bool Listen();
	int getAcceptSock();
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

bool CServerSocket::Accept()
{
	socklen_t addrlen = sizeof(client_addr);
	if((connect_sockfd = ::accept(server_sockfd, (struct sockaddr*)&client_addr, &addrlen)) == -1)
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

int CServerSocket::getAcceptSock()
{
	return connect_sockfd;
}

bool CServerSocket::Send(char *sendbuf, int len)
{
	if(connect_sockfd < 0 || sendbuf==NULL || len < 0)
		return false;
	ssize_t ret;
	//do{
		ret = ::send(connect_sockfd, sendbuf, len, 0);
		if(ret<0)
		{
			perror("send error");
		}
/*	
	int dataleft = len, total = 0, ret =0;
	while(dataleft > 0)
	{
		ret = ::send(connect_sockfd, sendbuf+total, dataleft, 0);
		if(ret < 0)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				usleep(50000);
				ret = 0;
			}
		}
		total += ret;
		dataleft = len-total;
	}
	return total == len;
*/
	return ret == len;
}




bool CServerSocket::Recv(char *recvbuf, int len)
{
	if(connect_sockfd <0 || recvbuf==NULL || len < 0)
		return false;
	int ret = 0;
	//do{
		ret = ::recv(connect_sockfd, recvbuf, len, 0);
		if(ret==0)
		{
			perror("peer closed\n");
			//break;
		}
		else if(ret<0)
			perror("read from server error");
	//} while (ret < 0 && (errno == EAGAIN || errno == EINTR));
    return ret == len; 



/*	fd_set fds;
	struct timeval interval;
	interval.tv_sec = timeout;
	interval.tv_usec = 0;
	ssize_t recvlen = 0;

	while(true)
	{
		FD_ZERO(&fds);
		FD_SET(connect_sockfd, &fds);
		int res = ::select(connect_sockfd+1, &fds, NULL, NULL, &interval);
		if(res == 0)
		{
			perror("peer close");
			return false;
		}
		if(res < 0)
		{
			perror("read from server error");
			::close(connect_sockfd);
			connect_sockfd = -1;
			return false;
		}			
		else
		{
			if(FD_ISSET(connect_sockfd, &fds))
			{
				recvlen = ::recv(connect_sockfd, recvbuf, len, 0);
				return false;
			}
		}
	}
	return recvlen == len;
*/
	
}

