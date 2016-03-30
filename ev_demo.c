#include <stdio.h>
#include "ev_loop.h"
#include "ev_type.h"


ev_loop_t * loop = NULL;

void *cb_stdin1(ev_loop_t *loop, int fd, EV_TYPE events);
void *cb_timer1(ev_loop_t *loop, struct ev_timer *timer);



int main() 
{
    loop = ev_create_loop();

    ev_io_start(loop, 100, 1);
    ev_io_register(loop, 0, EV_READ, cb_stdin1, NULL);
    
    ev_timer_start(loop, 10);
    ev_timer_register(loop, 5.0, cb_timer1, 1, NULL);

    ev_run_loop(loop);
    return 0;
}

void *cb_stdin1(ev_loop_t *loop, int fd, EV_TYPE events) {
    printf("You have data to read\n");
    return NULL;
}
void *cb_timer1(ev_loop_t *loop, struct ev_timer *timer) {
    printf("Timeout!\n");
    return NULL;
}
