#include "CServerSocket.h"
#include "prot.h"
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
			exit(EXIT_SUCCESS);
		}
		else   //f=父进程用于接收数据
		{
			char buf[MAXBYTEMUN];
			int key_pressed;
			mv_msg_t joy_mv;
			while( 1 ){
				key_pressed = Getkey();
				memset(&joy_mv,0,sizeof(joy_mv));
				switch(key_pressed)
				{
					case 119:
					case 87://W
						joy_mv.x_linear = 1;
						break;
					case 115:
					case 83://S
						joy_mv.x_linear = -1;
						break;
					case 100:
					case 65://A
						joy_mv.z_angular = -2;
						break;
					case 97:
					case 68://D
						joy_mv.z_angular = 2;
						break;
					default:
						memset(&joy_mv,0,sizeof(joy_mv));
				}
				memset(&tele_msg,0,sizeof(tele_msg));
				msg_enpack(&tele_msg,&joy_mv);
				memcpy(buf,&tele_msg,sizeof(tele_msg));
				printf("linear:%f,angular:%f\n",joy_mv.x_linear,joy_mv.z_angular);
				server.Send(buf,sizeof(buf));
				usleep(10);
			}
			exit(EXIT_SUCCESS);
		}
	}
	return 0;
}

