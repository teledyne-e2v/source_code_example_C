
#define CLEAR(x) memset(&(x), 0, sizeof(x))
#include <stdint.h>
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
struct Control_List control_list;

FILE * ctrls;
int fd=-1;
