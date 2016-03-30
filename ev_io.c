#include "ev_type.h"
#include "ev_loop.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <string.h>
#include <stdint.h>

#define MINEVENTNUMBER 128

int ev_io_init(ev_loop_t *loop, int max_ev_num, int etmodel) {
    /* set a default minumum num of evnets*/
    if (max_ev_num < MINEVENTNUMBER) {
        max_ev_num = MINEVENTNUMBER;
    }
    loop->maxevent = max_ev_num;
	loop->etmodel = etmodel;
	loop->events = (struct epoll_event *)malloc(max_ev_num * sizeof(struct epoll_event));
	if (loop->events == NULL) {
		fprintf(stderr, "No enough memory for loop->events\n");
		return -1;
	}
	
	loop->iomap = (ev_io_t *)malloc(max_ev_num * sizeof(ev_io_t));
	if (loop->iomap == NULL) {
		fprintf(stderr, "No enough memory for loop->iomap\n");
		return -1;
	}
	int i;
	for (i = 0; i < max_ev_num; i++) {
		loop->iomap[i].active = 0;
		loop->iomap[i].events = 0;
		loop->iomap[i].cb_read = NULL;
		loop->iomap[i].cb_write = NULL;
		loop->iomap[i].ptr = NULL;
	}
	return 0;
}

int ev_io_register(ev_loop_t* loop, int fd, EV_TYPE events, cb_io_t cb, void *ptr) {
	if (!(events & EV_READ || events & EV_WRITE)) {
		fprintf(stderr, "invalid events\n");
		return -1;
	}
	if (fd < 0) {
		fprintf(stderr, "invalid fd:%d\n", fd);
		return -1;
	}
	if (ptr != NULL) {
		loop->iomap[fd].ptr = ptr;
	}
	/* events registerd already, just change the cb */
	if ((loop->iomap[fd].events & events) == events) {
		if (loop->iomap[fd].events & EV_READ) {
			loop->iomap[fd].cb_read = cb;
		}
		if (loop->iomap[fd].events & EV_WRITE) {
			loop->iomap[fd].cb_write = cb;
		}
	}
	/* new events going to be added */
	else { 
		if (EV_READ & events) {
			loop->iomap[fd].cb_read = cb;
		}
		if (EV_WRITE & events) {
			loop->iomap[fd].cb_write = cb;
		}
		loop->iomap[fd].events |= events;

		struct epoll_event ev;
		ev.events = loop->iomap[fd].events;
		if (loop->etmodel) {
			ev.events |= EPOLLET;
		}
		ev.data.fd = fd;

		if(loop->iomap[fd].active) {/*mod*/
			if(-1 == epoll_ctl(loop->epfd, EPOLL_CTL_MOD, fd, &ev)) {
				fprintf(stderr, "ERROR: epoll_ctl mod in ev_register: %s\n", strerror(errno));
				ev_io_clear(loop, fd);
				return -1;
			}
		} 
		else {/*add*/
			if(-1 == epoll_ctl(loop->epfd, EPOLL_CTL_ADD, fd, &ev)) {
				fprintf(stderr, "ERROR: epoll_ctl add in ev_register: %s\n", strerror(errno));
				ev_io_unregister(loop, fd);
				return -1;
			}
		}
	}
	loop->iomap[fd].active = 1;
	return 0;
}

int ev_io_unregister(ev_loop_t* loop, int fd) {
	struct epoll_event ev;
	if (-1 == epoll_ctl(loop->epfd, EPOLL_CTL_DEL, fd, &ev)) {
		fprintf(stderr, "ERROR: epoll_ctl del in ev_unregister: %s\n", strerror(errno));
		ev_io_clear(loop, fd);
		return -1;
	}
	ev_io_clear(loop, fd);
	return 0;
}

int ev_io_stop(ev_loop_t* loop, int fd, EV_TYPE events) {
	if (loop->iomap[fd].active && (loop->iomap[fd].events & events)) {
		loop->iomap[fd].events &= (~events);
		struct epoll_event ev;
		ev.events = loop->iomap[fd].events;
		if (loop->etmodel) {
			ev.events |= EPOLLET;
		}
		ev.data.fd = fd;

		if (-1 == epoll_ctl(loop->epfd, EPOLL_CTL_MOD, fd, &ev)) {
			fprintf(stderr, "ERROR: epoll_ctl mod in ev_stop: %s\n", strerror(errno));
			ev_io_clear(loop, fd);
			return -1;
		}
	}
	return 0;
}


int ev_io_clear(ev_loop_t *loop, int fd) {
	loop->iomap[fd].active = 0;
	loop->iomap[fd].cb_read = NULL;
	loop->iomap[fd].cb_write = NULL;
	loop->iomap[fd].ptr = NULL;
	return 0;
}
