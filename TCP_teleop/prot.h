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
#include "Joystick.h"
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

bool msg_enpack(struct teleop_message* teleop_msg, struct Joy* Joy_msg)
{
	//needed Joystick's message
	mv_msg_t joy_mv;
	joy_mv.x_linear = Joy_msg->axes[1];
	joy_mv.z_angular = Joy_msg->axes[0];

	//TCP package's message
	teleop_msg->start = pkg_start;
	teleop_msg->len = sizeof(joy_mv);
	teleop_msg->msg = type_move_msg;
	memcpy(&(teleop_msg->payload64), &joy_mv, sizeof(joy_mv));
	teleop_msg->end = pkg_end;
	return true;
}

bool msg_depack( struct teleop_message* teleop_msg, struct move_message* mv_msg )
{
	//TCP package's message
	if( (pkg_start != teleop_msg->start) || (pkg_end != teleop_msg->end) )
		return false;
	else
	{
		switch( teleop_msg->msg )
		{
			case type_move_msg:
				memcpy(&mv_msg, &(teleop_msg->payload64), teleop_msg->len);
				break;
			default:
				perror("message type error");
		}
	}
	return true;
}

#endif /* PROT_H_ */
