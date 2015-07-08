#include "ev_type.h"
#include "ev_loop.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

ev_loop_t * ev_create_loop() {
	ev_loop_t *loop;
	loop = (ev_loop_t *)malloc(sizeof(ev_loop_t));

	loop->epfd = epoll_create1(0);
	if (loop->epfd == -1) {
		fprintf(stderr, "create epoll error:%s\n", strerror(errno));
		return NULL;
	}
	return loop;
}

int ev_run_loop(ev_loop_t* loop) {
	while (1) {
		int num;
		num = epoll_wait(loop->epfd, loop->events, loop->maxevent, -1);
		if (num == -1) {
			/* fix gbd can not run */
			if (errno == EINTR)
				continue;
			fprintf(stderr, "ERROR: epoll wait error: %s\n", strerror(errno));
			return -1;
		}
		int i;
		for (i = 0; i < num; i++) {
			int fd = loop->events[i].data.fd;
			ev_io_t io_record = loop->iomap[fd];

			if (EV_READ & loop->events[i].events) {
				if (io_record.cb_read != NULL)
					(*(io_record.cb_read))(loop, fd, EV_READ);
			}
			/*pre-step may have unregisterd the fd, make sure the fd is active!*/
			if ((EV_WRITE & loop->events[i].events) && io_record.active) {
				if (io_record.cb_write != NULL)
					(*(io_record.cb_write))(loop, fd, EV_WRITE);
			}
		}
	}
	return 0;
}