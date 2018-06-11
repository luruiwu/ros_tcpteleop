/*
 * prot.h
 *
 *  Created on: Jun 4, 2018
 *      Author: ubuntu
 */

#ifndef PROT_H_
#define PROT_H_

#define pkg_start 0xAA
#define pkg_end 0x55
enum msg_type
{
	type_move_msg = 0x01
};

enum msg_size{
	size_move_msg = 8
};

typedef struct teleop_message{
	uint8_t start;
	uint8_t len;
	uint8_t msg;
	uint64_t payload64[8];
	uint8_t end;
} teleop_message_t;

teleop_message_t tele_msg;

typedef struct move_message{
	float z_angular;
	float x_linear;
} mv_msg_t;


#include <termio.h>
#include <stdio.h>


int Getkey()
{
	int in;
	struct termios new_settings;
	struct termios stored_settings;
	tcgetattr(0,&stored_settings);
	new_settings = stored_settings;
	new_settings.c_lflag &= (~ICANON);
	new_settings.c_cc[VTIME] = 0;
	tcgetattr(0,&stored_settings);
	new_settings.c_cc[VMIN] = 1;
	tcsetattr(0,TCSANOW,&new_settings);

	in = getchar();

	tcsetattr(0,TCSANOW,&stored_settings);
	return in;
}


bool msg_enpack(struct teleop_message* teleop_msg, struct move_message* joy_mv)
{
	//TCP package's message
	teleop_msg->start = pkg_start;
	teleop_msg->len = sizeof(joy_mv);
	teleop_msg->msg = type_move_msg;
	memcpy(teleop_msg->payload64, joy_mv, sizeof(joy_mv));
	teleop_msg->end = pkg_end;
	return true;
}


#endif /* PROT_H_ */
