#ifndef _EV_TYPE_H
#define _EV_TYPE_H


#include <unistd.h>
#include <sys/epoll.h>

#define EV_TYPE __uint32_t

/* forward declaration */
struct ev_timer;
struct ev_io;

enum {
	EV_READ = EPOLLIN,  
	EV_WRITE = EPOLLOUT 
};

/* main loop */
typedef struct ev_loop {
	/* io */
	int epfd;
	int maxevent;
	int etmodel;
	struct epoll_event *events;
	struct ev_io *iomap;

	/* timer */
	struct ev_timer **heap;
	int heap_size;
	int heap_capacity;
	int timer_fd;
} ev_loop_t;


/* io */
typedef void* (*cb_io_t) (ev_loop_t *loop, int fd, EV_TYPE events);

typedef struct ev_io {
	int active;

	EV_TYPE events;
	cb_io_t cb_read;
	cb_io_t cb_write;

	/*for custom use*/
	void *ptr; 
} ev_io_t;


/* timer */
typedef void* (*cb_timer_t) (ev_loop_t *loop, struct ev_timer *timer);

typedef struct ev_timer {
	uint8_t repeat;  /* do once or forever*/
	double timeout; /* e.g. 9.1 => 9.1 s*/
	struct timespec to_ts;   /* timeout time in timespect type*/
	cb_timer_t cb;      /* callback function*/
	
	void * ptr;     /* for custom use*/
} ev_timer_t;



#endif


