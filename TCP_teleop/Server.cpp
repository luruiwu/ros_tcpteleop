#include "CServerSocket.h"
#include "Joystick.h"
#include "prot.h"
int main(int argc, char *argv[])  
{  
	CServerSocket server;  
	if(server.Bind())
	{
		printf("Bind success\n");
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
			clock_gettime(CLOCK_MONOTONIC, &now);	
			clock_gettime(CLOCK_MONOTONIC, &start_time);
			double autorepeat_interval = 1 / autorepeat_rate_;

 			struct timeval tv;
			fd_set set;
			event_count_ = 0;
			bool joy_ok = true;
			while(joy_ok)
			{
			  open_ = false;
			  bool first_fault = true;
			  int open_count=10;
			  while(!open_joystick(joy_dev_) && open_count)
			  {
					if(first_fault)
					{
						perror("open joystick failed");
						first_fault = false;
					}
					sleep(1.0);
					open_count--;
			  }
				memset(&joy_msg,0,sizeof(struct Joy));
				memset(&sticky_buttons_joy_msg,0,sizeof(struct Joy));
				memset(&tele_msg,0,sizeof(struct teleop_message));

			  open_ = true;
			  bool tv_set = false;
			  bool publication_pending = false;
			  tv.tv_sec = 1;
			  tv.tv_usec = 0;
			  while(joy_ok)
			  {
				  FD_ZERO(&set);
				  FD_SET(joystick_fd, &set);
				  int select_out = select(joystick_fd+1, &set, NULL, NULL, &tv);
				  if (select_out == -1){
						tv.tv_sec = 0;
						tv.tv_usec = 0;
						continue;
				  }
					if(FD_ISSET(joystick_fd, &set))
						if(!get_joystick_status())
							break;//read the JOYSTICKS input
					else if(tv_set)// Assume that the timer has expired.
						publish_now = true;
					if (publish_now) {
						// Assume that all the JS_EVENT_INIT messages have arrived already.
						// This should be the case as the kernel sends them along as soon as
						// the device opens.
						if (sticky_buttons_ == true) {
						  // cycle through buttons
							for (size_t i = 0; i < JS_BUTTONS_NUM; i++) {
						    // change button state only on transition from 0 to 1
						    if (joy_msg.buttons[i] == 1 && last_published_joy_msg.buttons[i] == 0) {
								  sticky_buttons_joy_msg.buttons[i] = sticky_buttons_joy_msg.buttons[i] ? 0 : 1;
						    } else {
								  // do not change the message sate
								  //sticky_buttons_joy_msg.buttons[i] = sticky_buttons_joy_msg.buttons[i] ? 0 : 1;
						    }
							}
						  // update last published message
						  last_published_joy_msg = joy_msg;
						  // fill rest of sticky_buttons_joy_msg (time stamps, axes, etc)
						  sticky_buttons_joy_msg.header.stamp.tv_usec = joy_msg.header.stamp.tv_usec;
						  sticky_buttons_joy_msg.header.stamp.tv_sec  = joy_msg.header.stamp.tv_sec;
						  sticky_buttons_joy_msg.header.frame_id   = joy_msg.header.frame_id;
							for(size_t i=0; i < JS_AXIS_NUM; i++){
								sticky_buttons_joy_msg.axes[i] = joy_msg.axes[i];
							}
								
							memset(buf,0,sizeof(buf));
							memcpy(buf,&sticky_buttons_joy_msg,sizeof(struct Joy));
							server.Send(buf, sizeof(buf));
						} else {

							memset(buf,0,sizeof(buf));
							msg_enpack(&tele_msg,&joy_msg);
							memcpy(buf,&joy_msg,sizeof(struct Joy));
							server.Send(buf, sizeof(buf));
						}
						publish_now = false;
						tv_set = false;
						publication_pending = false;
						publish_soon = false;
						pub_count_++;
					}
					// If an axis event occurred, start a timer to combine with other events.
					if (!publication_pending && publish_soon)
					{
						tv.tv_sec = trunc(coalesce_interval_);
						tv.tv_usec = (coalesce_interval_ - tv.tv_sec) * 1e6;
						publication_pending = true;
						tv_set = true;
					}

					// If nothing is going on, start a timer to do autorepeat.
					if (!tv_set && autorepeat_rate_ > 0)
					{
						tv.tv_sec = trunc(autorepeat_interval);
						tv.tv_usec = (autorepeat_interval - tv.tv_sec) * 1e6;
						tv_set = true;
					}

					if (!tv_set)
					{
						tv.tv_sec = 1;
						tv.tv_usec = 0;
					}
				} // End of joystick open loop.
				close(joystick_fd);
			}
		}
		else   //f=父进程用于接收数据
		{
			char recvbuf[MAXBYTEMUN];
			while(1)
			{
				server.Recv(recvbuf,sizeof(recvbuf));
				fputs(recvbuf,stdout);
				memset(recvbuf, 0, sizeof recvbuf);
			}
			exit(EXIT_SUCCESS);
		}
	}
	return 0;  
}  
