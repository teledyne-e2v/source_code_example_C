/*
You need to execute the program with sudo or you will get the error : Error: cannot open the direction file!: Permission denied
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>


static const char gpio_path[] = "/sys/class/gpio/";

void unexport(char* pin) // unexport pin (if not done pin will be unusable on next launch)
{

   char unexport[128];
   strcpy(unexport,gpio_path);
   strcat(unexport,"unexport");
    int fd = open(unexport, O_WRONLY);
    if (fd == -1) {
        perror("Error: cannot open the unexport file!");
        exit(EXIT_FAILURE);
    }

    if (write(fd, pin, 2) != 2) {
        perror("Error: cannot write the unexport!");
        exit(EXIT_FAILURE);
    }

    close(fd);
}


void export(char* pin) // Check and setup export
{
   char export[128];
   strcpy(export,gpio_path);
   strcat(export,"export");
   int fd = open(export, O_WRONLY);

    if (fd == -1) {
        perror("Error: cannot open the export file! check permissions");
        exit(EXIT_FAILURE);
    }

    if (write(fd, pin, 2) != 2) {
	unexport(pin); // try to unexport the pin (fix the most common error, try to relaunch)
        perror("Error: cannot write to the export! pin is maybe busy try to relaunch");
        exit(EXIT_FAILURE);
    }

    close(fd);
}


void direction(char* pin)  // Check and setup direction to output
{
    char direction[128];
    strcpy(direction,gpio_path);
    strcat(direction,"gpio");
    strcat(direction,pin);
    strcat(direction,"/direction");
    int fd = open(direction, O_WRONLY);
    if (fd == -1) {
        perror("Error: cannot open the direction file!");
        exit(EXIT_FAILURE);
    }

    if (write(fd, "out", 3) != 3) {
        perror("Error: cannot write the direction!");
        exit(EXIT_FAILURE);
    }

    close(fd);
}



int main()
{
	char high[2] = "1";
	char low[2] = "0";
	char pin[4]="79";

	char value[128]; // the value file is where we have to change the GPIO value (write 1 will outpout GPIO tension of 3.3V 0 will output tension of 0V)
	strcpy(value,gpio_path);
	strcat(value,"gpio");
	strcat(value,pin);
	strcat(value,"/value");
	export(pin);
    	direction(pin);
	

    int fd = open(value, O_WRONLY);
    if (fd == -1) {
        perror("Error: cannot open the value file!");
        exit(EXIT_FAILURE);
    }

    float frequency=100; //frequency of 100hz

    for (int i = 0; i < 1000; i++) {
        if (write(fd, high, 1) != 1) {
            perror("Error: cannot write the value!");
            exit(EXIT_FAILURE);
        }
        usleep(1000000/(2* frequency));

        if (write(fd, low, 1) != 1) {
            perror("Error: cannot write the value!");
            exit(EXIT_FAILURE);
        }
        usleep(1000000/(2* frequency));
    }
    unexport(pin);
    close(fd);

   

    return 0;
}
