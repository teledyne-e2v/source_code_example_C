# Version 1.0

#### The purpose of this code is to provide a simple example of using the V4L2 driver of the Topaz module in C.

# Compilation 

```
meson build
ninja -C build install
```
All executables will be in the directory "executables"

# GPIO

GPIO_example.c brings an example of control of the pins of the nano jetson allowing with an adjustment of the frequency or the number of signals to send. See the code for more information.

## Usage 

``` 
cd executables
./GPIO
```

# Image Capture

ImageCapture.c provides an example of taking a sequence of images by changing some sensor parameters between each image. In this file you can also find functions to drive the sensor which are different from the api (less complex, address of the controls defined in static).
You can compile the program to take a sequence of RAW images and customize the controls applied to each image as you wish.
It is also possible to change the format to take picture in Y10 or GRAY8. Y10 images will be saved in 16bit GRAY raw image.

## Usage 

Image capture take in argument the sensor mode. 
The sensor mode configure the Format and resolution of the sensor.

``` 
cd executables
./ImageCapture <sensor mode>
```

Example To have GRAY8 format in 1920x1080:

``` 
./ImageCapture 2
```
sensor mode = 0 -> Y10  in 1920x1080 (default)

sensor mode = 1 -> Y10  in 1920x800

sensor mode = 2 -> GRAY8  in 1920x1080

sensor mode = 3 -> GRAY8  in 1920x800
