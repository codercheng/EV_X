#include <stdio.h>
#include "ev_loop.h"
#include "ev_type.h"


ev_loop_t * loop = NULL;

void *cb_stdin(ev_loop_t *loop, int fd, EV_TYPE events);
void *cb_stdin2(ev_loop_t *loop, int fd, EV_TYPE events);
int main() 
{
    loop = ev_create_loop();

    ev_io_start(loop, 100, 1);
    ev_io_register(loop, 0, EV_READ, cb_stdin, NULL);

    ev_run_loop(loop);
    return 0;
}

void *cb_stdin(ev_loop_t *loop, int fd, EV_TYPE events) {
    printf("you can read data now\n");
    ev_io_register(loop, fd, EV_READ, cb_stdin2, NULL);
    return NULL;
}
void *cb_stdin2(ev_loop_t *loop, int fd, EV_TYPE events) {
	printf("********************hahaha************\n");
	ev_io_stop(loop, fd, EV_READ);
	return NULL;
}
