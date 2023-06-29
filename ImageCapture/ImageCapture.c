/* V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@kernel.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <unistd.h>

struct buffer
{
        void *start;
        size_t length;
};

// Can configure the Format and resolution of the sensor.
// look at the optimom driver documentation for more informations
struct Sensor_modes
{
        unsigned int format;
        unsigned int width;
        unsigned int height;
};

static const struct Sensor_modes sensor_modes[4] = {
    {.format = V4L2_PIX_FMT_Y10, .width = 1920, .height = 1080},
    {.format = V4L2_PIX_FMT_Y10, .width = 1920, .height = 800},
    {.format = V4L2_PIX_FMT_GREY, .width = 1920, .height = 1080},
    {.format = V4L2_PIX_FMT_GREY, .width = 1920, .height = 800}};

#define CLEAR(x) memset(&(x), 0, sizeof(x))

// You can find the ID of the controles using v4l2-ctl -l
enum Controls
{
        analog_gain = 0x009a206f,
        exposure = 0x009a200a,
        sensor_mode = 0x009a2008,
        frame_rate = 0x009a200b,
        digital_gain = 0x009a2070,
        test_pattern = 0x009a2071,
        flip = 0x009a2072
};

int64_t get_control(int fd, int64_t code);
void set_control(int fd, int64_t code, int64_t value);
void capture_image(int fd, char *image_name, struct buffer *buffers, struct v4l2_buffer buf);
void capture_image_sequence(int fd, struct buffer *buffers, struct v4l2_buffer buf);
void initializeFormat(int fd, int SENSOR_MODE);
int openV4L2Node(const char *nodeName);

static void xioctl(int fh, int request, void *arg) // ioctl checking errors and trying while not working
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

int64_t get_control(int fd, int64_t code) // set control by code
{
        struct v4l2_ext_controls ecs;
        struct v4l2_ext_control ec;
        memset(&ecs, 0, sizeof(ecs));
        memset(&ec, 0, sizeof(ec));
        ec.id = code; // this code can be obtain using the command v4l2-ctl -l
        ecs.controls = &ec;
        ecs.count = 1;
        ecs.ctrl_class = V4L2_CTRL_CLASS_CAMERA;
        xioctl(fd, (int)VIDIOC_G_EXT_CTRLS, &ecs);
        return ec.value64;
}

void set_control(int fd, int64_t code, int64_t value) // set control by code
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
        xioctl(fd, (int)VIDIOC_S_EXT_CTRLS, &ecs);
}

void capture_image(int fd, char *image_name, struct buffer *buffers, struct v4l2_buffer buf)
{
        FILE *fout;

        CLEAR(buf); // clear the buffer
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // get the image from Queue WARNING : the queue is filled by 3 images,
        // if you control the sensor it may be shown after this 3 images,
        // you can remove from Queue with a simple  loop if you want
        xioctl(fd, (int)VIDIOC_DQBUF, &buf);

        fout = fopen(image_name, "w");
        if (!fout)
        {
                perror("Cannot open image");
                exit(EXIT_FAILURE);
        }

        // The image is stored in buffers[buf.index].start
        // you can apply extract the image from the buffers here if you want to
        // do some process on it before writing in a file

        fwrite(buffers[buf.index].start, buf.bytesused, 1, fout); // write raw image
        fclose(fout);

        xioctl(fd, (int)VIDIOC_QBUF, &buf);
}

void capture_image_sequence(int fd, struct buffer *buffers, struct v4l2_buffer buf) // capture a sequence of image
{

        set_control(fd, exposure, 10000); // initialization values
        // exposure analog gains and other controls are defined in an enum above

        for (int i = 1; i < 6; i++)
        {
                char image_name[128];
                set_control(fd, analog_gain, i);
                usleep(300000); // sleep 300ms
                sprintf(image_name, "image_number_%d.raw", i);
                capture_image(fd, image_name, buffers, buf);
        }
}

void initializeFormat(int fd, int SENSOR_MODE) // initialize format

{
        struct v4l2_format fmt; // struct containing
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = sensor_modes[SENSOR_MODE].width;
        fmt.fmt.pix.height = sensor_modes[SENSOR_MODE].height;
        fmt.fmt.pix.pixelformat = sensor_modes[SENSOR_MODE].format;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        xioctl(fd, (int)VIDIOC_S_FMT, &fmt);

        if (fmt.fmt.pix.pixelformat != sensor_modes[SENSOR_MODE].format)
        {
                printf("Libv4l didn't accept the current format. Can't proceed.\\n");
        }
        if ((fmt.fmt.pix.width != sensor_modes[SENSOR_MODE].width) || (fmt.fmt.pix.height != sensor_modes[SENSOR_MODE].height))
        {
                printf("Warning: driver is sending image at %dx%d\\n",
                       fmt.fmt.pix.width, fmt.fmt.pix.height);
        }
}

int openV4L2Node(const char *nodeName)
{
        int fd = open(nodeName, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0)
        {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }
        return fd;
}

int main(int argc, char **argv)
{
        int SENSOR_MODE;
        struct v4l2_buffer buf;
        struct v4l2_requestbuffers req;
        enum v4l2_buf_type type;
        int fd = -1;
        char cmd[100];
        unsigned int n_buffers;
        struct buffer *buffers;

        if (argc != 2)
        {
                printf("Sensor mod not precised, sensor mode 0 will be used\n");
                SENSOR_MODE = 0;
        }
        else
        {
                SENSOR_MODE = atoi(argv[1]);
        }



        printf("open the V4L2 node \n");
        fd = openV4L2Node("/dev/video0");

        printf("Configuration of the sensor mode\n");

        sprintf(cmd, "v4l2-ctl -c sensor_mode=%d", SENSOR_MODE);
        system(cmd);
        usleep(100000);

        printf("Initialization of the format\n");

        initializeFormat(fd, SENSOR_MODE);

        printf("Initialization of the request buffers\n");
        // initialize request buffer
        CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, (int)VIDIOC_REQBUFS, &req);

        buffers = calloc(req.count, sizeof(*buffers));

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) // initialize buffers
        {
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                xioctl(fd, (int)VIDIOC_QUERYBUF, &buf);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start = mmap(NULL, buf.length,
                                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                                fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }

        // put the buffers in queue
        for (unsigned int i = 0; i < n_buffers; ++i)
        {
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                xioctl(fd, (int)VIDIOC_QBUF, &buf);
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        // Start stream (fill the buffers) now you can control the sensor (may require a bit of time to apply the first control)
        xioctl(fd, (int)VIDIOC_STREAMON, &type);

        capture_image_sequence(fd, buffers, buf);

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, (int)VIDIOC_STREAMOFF, &type); // stream off
        for (unsigned int i = 0; i < n_buffers; ++i)       // free buffers
                munmap(buffers[i].start, buffers[i].length);
        close(fd); // closing device.

        return 0;
}
