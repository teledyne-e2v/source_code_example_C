#### The purpose of this code is to provide a simple example of using the V4L2 driver of the Topaz module in C.


# API

api.c and api.h provide an example of driver control by automatically retrieving the driver controls produced by the v4l2-ctl -l command. 
It can be used as an api to simplify your acces to the sensor controls in C.



This api provides several functions: 

- void initialization(char *v4l2_device, int sensor_mode) allows to start the driver with a configurable sensor mode. (Initialization is necessary before using any other function)

- void close_driver_access() closes the access to the driver, must be used last, no other function must be called after this one (except if you want to do an initialization again)

- int get_control(char* name): Allows to retrieve the whole value of a control from its name.

- int get_control_by_code(int code) : Allows to get the whole value of a control from its id.

- void set_control(char* name, int value) : Allows to assign a value to a control from its name.

- void set_control_by_code(int code, int value): Allows to assign a value to a control from its id.

- void print_control_list() allows to print the list of available controls

# GPIO

GPIO_example.c brings an example of control of the pins of the nano jetson allowing with an adjustment of the frequency or the number of signals to send. See the code for more information.

# Image Capture

ImageCapture.c provides an example of taking a sequence of images by changing some sensor parameters between each image. In this file you can also find functions to drive the sensor which are different from the api (less complex, address of the controls defined in static).
You can compile the program to take a sequence of RAW images and customize the controls applied to each image as you wish.