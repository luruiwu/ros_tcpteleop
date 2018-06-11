#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>
#include "boost/thread/mutex.hpp"
#include "boost/thread/thread.hpp"
#include "ros/console.h"

#include "CClientSocket.h"
#include "prot_client.h"

int main(int argc, char **argv)
{
	CClientSocket client;
	client.server_host = "192.168.31.100";
	while(!client.Connect());

	pid_t pid = fork();
	if(pid<0)
		perror("fork error\n");
	if ( 0==pid ){
		int event_count_ = 0;
		char recvbuf[MAXBYTEMUN];
		tele_message_t teleop_msg;
		mv_msg_t mv_msg;
		memset(&mv_msg,0,sizeof(mv_msg));
		
		//ros initialize
		ros::init(argc,argv,"remote_joy");//name remapping
		int linear_, angular_;
		linear_=1;angular_=0;
		double l_scale_,a_scale_;
		l_scale_ = 2; a_scale_ = 1.8;
		bool status_ros_ = true;
		bool pressed = false;
		ros::NodeHandle nh_;

		ros::Rate loop_rate(100);//frequency
		ros::Publisher remote_pub_ = nh_.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel",100);//topic
		geometry_msgs::Twist vel;
		//close(new_fd);//父进程不需要连接，所以关闭连接套接字
		while(ros::ok())
		{
			memset(recvbuf,0,sizeof(recvbuf));
			flags_recv=recv(new_fd,recvbuf,MAXBYTEMUN,0);
			if(flags_recv==0){    
				printf("peer closed\n");    
				break;
			}
			else if(flags_recv<0)
				perror("read from client error");
				
			memcpy(&teleop_msg,recvbuf,sizeof(struct teleop_msg));
			msg_depack(&teleop_msg, &mv_msg);
			vel.linear.x = mv_msg.x_linear;
			vel.angular.z = mv_msg.z_angular;

			remote_pub_.publish(vel);

			ROS_INFO("linear:%f,angular:%f",vel.linear.x,vel.angular.z);
			ros::spinOnce();
			loop_rate.sleep();
			++event_count_;
		}
		exit(EXIT_SUCCESS); 
	} else {
		char buf[MAXBYTEMUN];
		while(fgets(buf,sizeof(buf),stdin)!=NULL) // get the input
		{
			write(new_fd,buf,strlen(buf));
			memset(buf,0,sizeof(buf));   //清空，以免和下一次混淆
      //exit(EXIT_SUCCESS);
		}
		exit(EXIT_SUCCESS);
	}
    return 0;
}
