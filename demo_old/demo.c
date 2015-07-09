#include "../src/ev_loop.h"

#include <stdio.h>

ev_loop_t * loop = NULL;

void *test(int fd, EV_TYPE events);

int main() 
{
	loop = ev_create_loop(10, 0);
	ev_register(loop, 0, EV_READ, test);

	ev_run_loop(loop);


	return 0;
}

void *test(int fd, EV_TYPE events) {
	printf("ready read\n");
	return NULL;
}