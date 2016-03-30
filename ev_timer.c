#include "ev_type.h"
#include "ev_loop.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/timerfd.h>

#define RSHIFT(x) ((x) >> 1)
#define LSHIFT(x) ((x) << 1)
#define LCHILD(x) LSHIFT(x)
#define RCHILD(x) (LSHIFT(x)|1)
#define PARENT(x) (RSHIFT(x))

#define ONESECOND 1000000000 //in nanosecond

static
int timer_cmp_lt(struct timespec ts1, struct timespec ts2) {
	if (ts1.tv_sec > ts2.tv_sec) {
		return 0;
	}
	else if (ts1.tv_sec == ts2.tv_sec) {
		if (ts1.tv_nsec > ts2.tv_nsec) {
			return 0;
		}
		else if (ts1.tv_nsec == ts2.tv_nsec) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		return 1;
	}
}

static
void heap_percolate_up(ev_timer_t **heap, int pos) {
	ev_timer_t *timer = heap[pos];
	while ((pos > 1) && (timer_cmp_lt(timer->ts, heap[PARENT(pos)]->ts))) {
		heap[pos] = heap[PARENT(pos)];
		pos = PARENT(pos);
	}
	heap[pos] = timer;
}

static
void heap_percolate_down(ev_timer_t **heap, int pos, int heap_size) {
	ev_timer_t *timer = heap[pos];
	while (LCHILD(pos) <= heap_size) {
		int s_pos = LCHILD(pos);
		/* right child exist and right is smaller */
		if (s_pos + 1 <= heap_size && timer_cmp_lt(heap[s_pos + 1]->ts, heap[s_pos]->ts)) {
			s_pos++;
		}

		if (timer_cmp_lt(timer->ts, heap[s_pos]->ts)) {
			break;
		}
		heap[pos] = heap[s_pos];
		pos = s_pos;
	}
	heap[pos] = timer;
}

static
void heap_add(ev_loop_t *loop, ev_timer_t *timer) {
	loop->heap[++(loop->heap_size)] = timer;
	heap_percolate_up(loop->heap, loop->heap_size);
}

/* timeout --> ts, which is the expired time */
static
struct timespec double2timespec(double timeout) {
	long long int sec = (long long int)timeout;
	long long int nsec = (long long int)((timeout - (double)sec) * ONESECOND);

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec += sec;
	ts.tv_nsec += nsec;
	if (ts.tv_nsec >= ONESECOND) {
		ts.tv_nsec %= ONESECOND;
		ts.tv_sec++;
	}
	return ts;
}

static
ev_timer_t* heap_top(ev_timer_t **heap) {
	return heap[1];
}

static
void heap_pop(ev_loop_t *loop) {
	if (loop->heap_size < 1)
		return;
	free(loop->heap[1]);
	loop->heap[1] = loop->heap[loop->heap_size];
	loop->heap[loop->heap_size] = NULL;
	loop->heap_size--;
	heap_percolate_down(loop->heap, 1, loop->heap_size);
}

/* get next timeout */
static
struct timespec get_next_ts(ev_loop_t *loop) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	/* process timeout events*/
	while (heap_top(loop->heap) != NULL && !timer_cmp_lt(ts, heap_top(loop->heap)->ts)) {
		//////////////////////////////////////////
		//heap != null, and delete all the cb==null timer in the head
		int bcontinue = 0;
		while (heap_top(loop->heap) != NULL && heap_top(loop->heap)->cb == NULL) {
			heap_pop(loop);
			bcontinue = 1;
		}
		if (bcontinue) {
			continue;
		}
		/* callback function */
		(*(heap_top(loop->heap)->cb))(loop, heap_top(loop->heap));

		if (!heap_top(loop->heap)->repeat) {
			heap_pop(loop);
		}
		else {
			heap_top(loop->heap)->ts = double2timespec(heap_top(loop->heap)->timeout);
			heap_percolate_down(loop->heap, 1, loop->heap_size);
		}
		/* important: update the current time, because you */
		/* never know how long the callback func costs  */
		clock_gettime(CLOCK_MONOTONIC, &ts);
	}

	/* set to 0 to disarm the timer*/
	if (heap_top(loop->heap) == NULL) {
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
		return ts;
	}

#ifdef EV_TEST
	printf(".............. *****.*********\n");
	printf("after heap:... %ld.%ld\n", heap_top(loop->heap)->ts.tv_sec, heap_top(loop->heap)->ts.tv_nsec);
	printf("after   ts:... %ld.%ld\n", ts.tv_sec, ts.tv_nsec);
#endif

	long int sec_tmp  = heap_top(loop->heap)->ts.tv_sec;
	long int nsec_tmp = heap_top(loop->heap)->ts.tv_nsec;

	if (ts.tv_nsec > heap_top(loop->heap)->ts.tv_nsec) {
		sec_tmp--;
		nsec_tmp += ONESECOND;
	}
	ts.tv_sec = sec_tmp - ts.tv_sec;
	ts.tv_nsec = nsec_tmp - ts.tv_nsec;

	return ts;
}

static
void* cb_check_timer(ev_loop_t *loop, int tfd, EV_TYPE events) {
	uint64_t data;
	read(loop->timer_fd, &data, 8);

	struct timespec ts;
	ts = get_next_ts(loop);
	struct itimerspec newValue;
	bzero(&newValue, sizeof(newValue));
	newValue.it_value = ts;

	int ret;
	ret = timerfd_settime(loop->timer_fd, 0, &newValue, NULL);
	if (ret == -1) {
		printf("ERROR: timerfd_settime err:%s\n", strerror(errno));
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////

int ev_timer_start(ev_loop_t *loop, int capacity) {
	loop->heap = (ev_timer_t**)malloc((capacity + 1)*sizeof(ev_timer_t*));
	if (loop->heap == NULL) {
		fprintf(stderr, "ERROR: no enough memory!\n");
		return -1;
	}
	int i;
	for (i = 0; i <= capacity; i++) {
		loop->heap[i] = NULL;
	}
	loop->heap_size = 0;
	loop->heap_capacity = capacity;


	if ((loop->timer_fd = timerfd_create(CLOCK_MONOTONIC, 0)) < 0) {
		fprintf(stderr, "ERROR: init timerfd error\n");
		return -1;
	}

	struct itimerspec newValue;
	bzero(&newValue, sizeof(newValue));
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
	newValue.it_value = ts;
	if (timerfd_settime(loop->timer_fd, 0, &newValue, NULL) != 0) {
		fprintf(stderr, "ERROR: timerfd_settime error: %s\n", strerror(errno));
		return -1;
	}
	return ev_io_register(loop, loop->timer_fd, EV_READ, cb_check_timer, NULL);
}

int ev_timer_register(ev_loop_t *loop, double timeout, cb_timer_t cb, uint8_t repeat, void *ptr) {
	if (loop->heap_size >= loop->heap_capacity) {
		ev_timer_t **temp = (ev_timer_t **)malloc((2 * (loop->heap_capacity) + 1)*sizeof(ev_timer_t *));
		if (temp == NULL) {
			fprintf(stderr, "ERROR: in ev_timer_register when malloc:%s\n", strerror(errno));
			return -1;
		}
		int i;
		for (i = 0; i < 2 * (loop->heap_capacity) + 1; i++) {
			temp[i] = NULL;
		}
		loop->heap_capacity *= 2;
		for (i = 0; i <= loop->heap_size; i++) {
			temp[i] = loop->heap[i];
		}
		free(loop->heap);
		loop->heap = temp;
	}

	ev_timer_t *timer = (ev_timer_t*)malloc(sizeof(ev_timer_t));
	if (timer == NULL) {
		fprintf(stderr, "ERROR: malloc error:%s\n", strerror(errno));
		return -1;
	}
	struct timespec ts;
	ts = double2timespec(timeout);

	timer->timeout = timeout;
	timer->ts = ts;
	timer->cb = cb;
	timer->repeat = repeat;
	timer->ptr = ptr;

	heap_add(loop, timer);

	/* two special conditions which need to settime */
	/* 1. first timer event                         */
	/* 2. the newly add timer is the new heap top   */
	/*    that means new's ts < old heap top's ts   */
	if (loop->heap_size == 1 || heap_top(loop->heap) == timer) {
		ts = get_next_ts(loop);
		struct itimerspec newValue;
		bzero(&newValue, sizeof(newValue));
		newValue.it_value = ts;

		if (timerfd_settime(loop->timer_fd, 0, &newValue, NULL) != 0) {
			fprintf(stderr, "ERROR: timerfd_settime error: %s\n", strerror(errno));
			return -1;
		}
	}
	
	return 0;
}
int ev_timer_unregister(ev_loop_t *loop, ev_timer_t *timer) {
	/* just set cb = NULL*/
	timer->cb = NULL;
	return 0;
}

