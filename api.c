
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "api.hpp"

Control_List control_list;

FILE *ctrls;
int fd = -1;

static void xioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1)
    {
        fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int get_control(char *name)
{
    int i = 0;
    while (strcmp(name, control_list.controls[i].name) != 0)
    {
        if (i == control_list.number_of_controls)
        {
            printf("control not found\n");
            return 0;
        }
        i++;
    }
    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.id = control_list.controls[i].id; // this code can be obtain using the command v4l2-ctl -l
    ecs.controls = &ec;
    ecs.count = 1;
    ecs.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    xioctl(fd, VIDIOC_G_EXT_CTRLS, &ecs);
    return ec.value64;
}

void set_control(char *name, int value)
{
    int i = 0;
    while (strcmp(name, control_list.controls[i].name) != 0)
    {
        if (i == control_list.number_of_controls)
        {
            printf("control not found\n");
            return;
        }
        i++;
    }
    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.id = control_list.controls[i].id; // this code can be obtain using the command v4l2-ctl -l
    ecs.controls = &ec;
    ecs.count = 1;
    ecs.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ec.value64 = value;
    ec.size = 0;
    xioctl(fd, VIDIOC_S_EXT_CTRLS, &ecs);
}

struct Control_List *get_control_list()
{
    return &control_list;
}

int get_control_by_code(int code)
{
    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.id = code; // this code can be obtain using the command v4l2-ctl -l
    ecs.controls = &ec;
    ecs.count = 1;
    ecs.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    xioctl(fd, VIDIOC_G_EXT_CTRLS, &ecs);
    return ec.value64;
}

void set_control_by_code(int code, int value)
{
    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.id = code; // this code can be obtain using the command v4l2-ctl -l
    ecs.controls = &ec;
    ecs.count = 1;
    ecs.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
    ec.value64 = value;
    ec.size = 0;
    xioctl(fd, VIDIOC_S_EXT_CTRLS, &ecs);
}

void print_control_list()
{
    printf("Number of controls: %d\n", control_list.number_of_controls);
    for (int i = 0; i < control_list.number_of_controls; i++)
    {
        printf("Control %d:\n", i + 1);
        printf("\tName: %s\n", control_list.controls[i].name);
        printf("\tID: %x\n", control_list.controls[i].id);
        printf("\tType: %s\n", control_list.controls[i].type);
        printf("\tMinimum: %ld\n", control_list.controls[i].minimum);
        printf("\tMaximum: %ld\n", control_list.controls[i].maximum);
        printf("\tStep: %d\n", control_list.controls[i].step);
        printf("\tDefault value: %ld\n", control_list.controls[i].default_value);
    }
}

void close_driver_access()
{
    fclose(ctrls);
}

void initialization(char *v4l2_device, int sensor_mode)
{
    fd = open(v4l2_device, O_RDWR | O_NONBLOCK, 0);
    system("v4l2-ctl -l > /tmp/ctrls_list.txt");
    ctrls = fopen("/tmp/ctrls_list.txt", "r"); // open this file

    char line[256];
    char sensor_mode_name[]="sensor_mode";
    int j = 0;

    while (fgets(line, 256, ctrls))
    { // Parse the file to get information
        char *splited[256];
        int index = 0;

        // Split the line into words
        char *word = strtok(line, " \n");
        while (word != NULL)
        {
            splited[index++] = word;
            word = strtok(NULL, " \n");
        }

        if (index > 2 && strlen(splited[1]) > 2)
        {
            if (strncmp(splited[1], "0x", 2) == 0)
            {
                // Create a struct to store the information

                strcpy(control_list.controls[j].name, splited[0]);
                /*
                        control_list.controls[i].minimum = 0;
                        control_list.controls[i].maximum = 0;
                        control_list.controls[i].step = 0;
                        control_list.controls[i].default_value = 0;*/

                for (int i = 2; i < index; i++)
                {
                    if (strlen(splited[i]) > 4 && splited[i][0] == '(' && splited[i][strlen(splited[i]) - 1] == ')')
                    {

                        strncpy(control_list.controls[j].type, splited[i] + 1, strlen(splited[i]) - 2);

                        control_list.controls[j].type[strlen(splited[i]) - 2] = '\0';

                        if (strcmp(control_list.controls[j].type, "bool") == 0)
                        {
                            control_list.controls[j].minimum = 0;
                            control_list.controls[j].maximum = 1;
                            control_list.controls[j].step = 1;
                        }
                    }
                    else if (strlen(splited[i]) > 8 && strncmp(splited[i], "default=", 8) == 0)
                    {

                        control_list.controls[j].default_value = atoi(splited[i] + 8);
                    }
                    else if (strlen(splited[i]) > 4 && strncmp(splited[i], "min=", 4) == 0)
                    {
                        control_list.controls[j].minimum = atoll(splited[i] + 4);
                    }
                    else if (strlen(splited[i]) > 4 && strncmp(splited[i], "max=", 4) == 0)
                    {
                        control_list.controls[j].maximum = atoll(splited[i] + 4);
                    }
                    else if (strlen(splited[i]) > 5 && strncmp(splited[i], "step=", 5) == 0)
                    {
                        control_list.controls[j].step = atoi(splited[i] + 5);
                    }
                }

                // Add the information to the struct

                unsigned int value = strtoul(splited[1], NULL, 16);
                control_list.controls[j].id = (int)value;

                j++;
            }
        }
    }
    control_list.number_of_controls = j;

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (sensor_mode == 2)
    {
        fmt.fmt.pix.width = 1920;
        fmt.fmt.pix.height = 1080;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        set_control(sensor_mode_name, 2);
    }
    else if (sensor_mode == 0)
    {
        fmt.fmt.pix.width = 1920;
        fmt.fmt.pix.height = 1080;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_Y10;
        set_control(sensor_mode_name, 0);
    }
    else if (sensor_mode == 1)
    {
        fmt.fmt.pix.width = 1920;
        fmt.fmt.pix.height = 800;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_Y10;
        set_control(sensor_mode_name, 1);
    }
    else if (sensor_mode == 3)
    {
        fmt.fmt.pix.width = 1920;
        fmt.fmt.pix.height = 80;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        set_control(sensor_mode_name, 3);
    }
    else
    {
        printf("sensor mode not supported\n");
        exit(0);
    }

    ioctl(fd, VIDIOC_S_FMT, &fmt);
}
