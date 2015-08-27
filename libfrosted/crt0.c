#include <fcntl.h>
 
extern void exit(int code);
extern int main ();
extern void _init_signal();
 
/* no argv, argc for now */
void _start() {
    int ex;
    _init_signal();
    ex = main();
    exit(ex);
}
