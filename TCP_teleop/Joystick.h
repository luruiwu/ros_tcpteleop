/*
 * Joystick.h
 *
 *  Created on: May 26, 2018
 *      Author: ubuntu
 */
#ifndef JOYSTICK_H_
#define JOYSTICK_H_
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/times.h>
#include <errno.h>
#include <cmath>

#define JS_EVENT_BUTTON 0x01 /* button pressed/released */
#define JS_EVENT_AXIS 0x02 /* joystick moved */
#define JS_EVENT_INIT 0x80 /* initial state of device */

#define JS_AXIS_NUM 4   /*the number of joystick axis */
#define JS_BUTTONS_NUM 17    /*the number of joystick buttons */
#define MAX_AXIS_SCALE_ 32767    /*the scale of axis */


int joystick_fd=-1;
bool open_=false;
bool sticky_buttons_=false;
bool default_trig_val_=false;
bool publish_now=false;
bool publish_soon=false;
char* joy_dev_name_ = "/dev/input/js0";
char* joy_dev_ = joy_dev_name_;
double deadzone_=0.05;
double autorepeat_rate_=0;  // in Hz.  0 for no repeat.
double coalesce_interval_=0.01; // Defaults to 100 Hz rate limit.
int event_count_=0;
int pub_count_=0;


struct Header{
	uint seq;
	struct timeval stamp;
	char frame_id;
};

struct Joy{
	struct Header header;
	float axes[JS_AXIS_NUM];
	int buttons[JS_BUTTONS_NUM];
};

struct js_event {
    unsigned int time;    /* event timestamp in milliseconds */
    short value; /* value */
    unsigned char type; /* event type */
    unsigned char number; /* axis/button number */
};


struct timespec start_time;
struct timespec now;

struct Joy joy_msg; // Here because we want to reset it on device close.
struct Joy last_published_joy_msg,
           sticky_buttons_joy_msg; // used for sticky buttons option

bool open_joystick(char *joystick_device);
bool read_joystick_event(struct js_event *jse);
int get_joystick_status();

bool open_joystick(char *joystick_device)
{
    joystick_fd = open(joystick_device, O_RDONLY | O_NONBLOCK); /* rea	d write for force feedback? */
    if (joystick_fd < 0){
    	printf("open failed.\n");
    	return joystick_fd;
    }

    /* maybe ioctls to interrogate features here? */

    return joystick_fd;
}

bool read_joystick_event(struct js_event *jse)
{
    int bytes;

    bytes = read(joystick_fd, jse, sizeof(*jse));

    if (bytes == -1)
        return 0;

    if (bytes == sizeof(*jse))
        return 1;

    printf("Unexpected bytes from joystick:%d\n", bytes);

    return -1;
}

int get_joystick_status()
{
  double val;
  struct js_event event;
  double scale = -1. / (1. - deadzone_) / MAX_AXIS_SCALE_;
  double unscaled_deadzone = MAX_AXIS_SCALE_ * deadzone_;
  if (!read_joystick_event(&event))
    return false; // Joystick is probably closed. Definitely occurs.
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  joy_msg.header.stamp.tv_sec = start_time.tv_sec;
  joy_msg.header.stamp.tv_usec = start_time.tv_nsec*10e3;
  event_count_++;
  switch(event.type)
  {
    case JS_EVENT_BUTTON:
    case JS_EVENT_BUTTON | JS_EVENT_INIT:
      if(event.number >= JS_BUTTONS_NUM)
        perror("JS_BUTTONS_NUM is too small.");
  	  joy_msg.buttons[event.number] = (event.value ? 1 : 0);
      // For initial events, wait a bit before sending to try to catch
      // all the initial events.
      if (!(event.type & JS_EVENT_INIT))
        publish_now = true;
      else
        publish_soon = true;
      break;
    case JS_EVENT_AXIS:
    case JS_EVENT_AXIS | JS_EVENT_INIT:
      val = event.value;
      if(event.number >= JS_AXIS_NUM)
        perror("JS_BUTTONS_NUM is too small.");
      if(default_trig_val_){
        // Allows deadzone to be "smooth"
        if (val > unscaled_deadzone)
    	  val -= unscaled_deadzone;
    	else if (val < -unscaled_deadzone)
    	  val += unscaled_deadzone;
        else
    	  val = 0;
    	joy_msg.axes[event.number] = val * scale;
    	// Will wait a bit before sending to try to combine events.
    	publish_soon = true;
    	break;
      } else {
    	if (!(event.type & JS_EVENT_INIT)){
    	  val = event.value;
    	  if(val > unscaled_deadzone)
    	    val -= unscaled_deadzone;
    	  else if(val < -unscaled_deadzone)
    		val += unscaled_deadzone;
    	  else
    		val=0;
    	  joy_msg.axes[event.number]= val * scale;
      }

      publish_soon = true;
      break;
      default:
    	perror("Unknown event type.");
      break;
    }
  }//swicth end
  return true;
}


#endif /* JOYSTICK_H_ */
