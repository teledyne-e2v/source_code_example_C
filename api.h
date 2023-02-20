#pragma once
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>



#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#define CLEAR(x) memset(&(x), 0, sizeof(x))
struct Control {
    char name[64];
    int id;
    char type[32];
    int minimum;
    int maximum;
    int step;
    int default_value;
};

struct Control_List {
    struct Control controls[32];
    int number_of_controls;
};


struct buffer
{
        void *start;
        size_t length;
};



int get_control(char* name);
void set_control(char* name, int value);
int get_control_by_code(int code);
void set_control_by_code(int code, int value);
void print_control_list();
void close_driver_access();
void initialization(char *v4l2_device, int sensor_mode);
