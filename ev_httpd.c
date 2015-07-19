#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "ev_type.h"
#include "ev_loop.h"

static int setnonblocking(int fd);
static int ev_listen(int port);

void *cb_accept(ev_loop_t *loop, int fd, EV_TYPE events);
void *cb_parse_request(ev_loop_t *loop, int fd, EV_TYPE events);
void *cb_do_response(ev_loop_t *loop, int fd, EV_TYPE events);


int main() {
    ev_loop_t *loop = NULL;
    int listen_sock;
    
    listen_sock = ev_listen(8080);
    if (listen_sock == -1) {
        return -1;
    }

    loop = ev_create_loop();
    ev_io_start(loop, 1024, 0);
    ev_io_register(loop, listen_sock, EV_READ,  cb_accept, NULL);
    
    ev_run_loop(loop);
    return 0;
}
void *cb_accept(ev_loop_t *loop, int fd, EV_TYPE events) {
	struct sockaddr_in client_sock;
	socklen_t len = sizeof(client_sock);
	int conn_fd;
	while ((conn_fd = accept(fd, (struct sockaddr *)&client_sock, &len)) > 0)	{
		/*limit the number of connections*/
		if (conn_fd >= loop->maxevent) {
			fprintf(stderr, "Warn: too many connections come, \
                        exceeds the maximum num of the configuration!\n");
			close(conn_fd);
			return NULL;
		}

		setnonblocking(conn_fd);

		int ret = ev_io_register(loop, conn_fd, EV_READ, cb_parse_request, NULL);
		if (ret == -1) {
		    fprintf(stderr, "ev io_register err in cd_accept()\n");
			close(conn_fd);
            return NULL;
		}
	}
	if (-1 == conn_fd) {
		if (errno != EAGAIN) {
			fprintf(stderr, "accpet err: %s\n", strerror(errno));
            return NULL;
		}
	}
	return NULL;
}

void *cb_parse_request(ev_loop_t *loop, int fd, EV_TYPE events) {
    //printf("in read\n");
    char tmp[1024];
    while(read(fd, tmp, 1024) > 0);
    int ret;
    ret = ev_io_stop(loop, fd, EV_READ);
    ret = ev_io_register(loop, fd, EV_WRITE, cb_do_response, NULL);
    if (ret == -1) {
        close(fd);
    }
    return NULL;
}
#define OUTPUT_STRING "hello evlib"

void *cb_do_response(ev_loop_t *loop, int fd, EV_TYPE events) {
    //printf("in write\n");
    write(fd, OUTPUT_STRING, strlen(OUTPUT_STRING));
    ev_io_unregister(loop, fd);
    close(fd);
    return NULL;
}

static
int ev_listen(int port) {
	signal(SIGPIPE, SIG_IGN);
	struct sockaddr_in server_addr;
	int listen_sock;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == listen_sock) {
        fprintf(stderr, "create sock err: %s\n", strerror(errno));
        return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_port = htons(port);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret;
	ret = bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (-1 == ret) {
		fprintf(stderr, "bind err: %s\n", strerror(errno));
		return -1;
	}

	setnonblocking(listen_sock);

	int reuse = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	if (-1 == listen(listen_sock, SOMAXCONN)) {
        fprintf(stderr, "listen err: %s\n", strerror(errno));
		return -1;
	}
	return listen_sock;
}

static
int setnonblocking(int fd) {
	int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
		perror("setnonblocking error");
		return -1;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


