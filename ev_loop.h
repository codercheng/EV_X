#ifndef _EV_LOOP_H
#define _EV_LOOP_H

#include "ev_type.h"
/* main loop */
extern ev_loop_t* ev_create_loop();
extern int ev_run_loop(ev_loop_t* loop);

/* io event */
extern int ev_io_start(ev_loop_t *loop, int max_ev_num, int etmodel);
extern int ev_io_register(ev_loop_t* loop, int fd, EV_TYPE events, cb_io_t cb, void *ptr);
extern int ev_io_unregister(ev_loop_t* loop, int fd);
extern int ev_io_stop(ev_loop_t* loop, int fd, EV_TYPE events);
extern int ev_io_clear(ev_loop_t* loop, int fd);



extern int ev_timer_start(ev_loop_t *loop, int capacity);
extern int ev_timer_register(ev_loop_t *loop, double timeout, cb_timer_t cb, uint8_t repeat, void *ptr);
extern int ev_timer_unregister(ev_loop_t *loop, ev_timer_t *timer);

/*

extern void timer_heap_init(ev_loop_t *loop, int capacity);
extern void add_timer(ev_loop_t *loop, double timeout, cb_timer_t cb, uint8_t repeat,  void *ptr);
extern struct timespec tick(ev_loop_t *loop);
extern void* check_timer(ev_loop_t *loop, int tfd, EV_TYPE events);
extern void delete_timer(ev_loop_t *loop, int sockfd);



extern int tcp_server(int port);
extern int setnonblocking(int fd);
*/

#endif