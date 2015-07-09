EV_LIB
======
这是一个基于epoll的事件回调库。目前支持`io读写`事件，和`定时`事件。(可以看成是简化版的libev)

Why
======
平常对于事件回调和定时器用的比较多，很多项目都会用到一点，非常著名的libevent/libev/libuv等都是很好的工具库，不过我还是想重新造个小轮子，花了点时间写了一个简化版的libev。

目前在我的个人项目中，有很多地方都用到了这个事件回调库(有改动)
* web server中用到了io事件回调来处理连接，同时用到了定时事件来处理Keep-alive的情况
* 在线聊天室中后台用到了定时事件，来做ajax long polling的定时

How to use
=======
---
因为程序非常的简单，所以接口也非常的简单明了。
###in your program###
* 把src中的两个文件加入你的源文件
* `#include <ev_loop.h>`


---
###demo###
```c
#include "ev_loop.h"
#include <stdio.h>

ev_loop_t * loop = NULL;

void *cb_stdin(int fd, EV_TYPE events);

int main() 
{
    /** 100: max count of events
     *    1：use EPOLLET
     */
    loop = ev_create_loop(100, 1);
	ev_register(loop, 0, EV_READ, cb_stdin);
	ev_run_loop(loop);
	return 0;
}

void *cb_stdin(int fd, EV_TYPE events) {
	printf("you can read data now\n");
	return NULL;
}
```
---
###API###

* 创建事件循环

```c
//创建一个 event loop并返回
//et = 1: 用EPOLLET 模式， =0： 用EPOLLLT模式
ev_loop_t *ev_create_loop(int maxevent, int et);
```

* 注册事件(`EV_READ, EV_WRITE`)到循环中

```c
//在fd上注册感兴趣的events
//当fd上events发生时，调用cb
int ev_register(ev_loop_t*loop, int fd, EV_TYPE events, cb_func_t cb);
```
* 注销fd

```c
//注销fd,即fd上的所有事件脱离循环
int ev_unregister(ev_loop_t *loop, int fd);
```

* stop events

```c
//这个不同于注销，而是只停止某个时间，比如fd上同时注册了
//可读和可写event, 那么我可以值stop可读事件
int ev_stop(ev_loop_t *loop, int fd, EV_TYPE events);
```
* 启动loop

```c
//最后一步，启动Loop，开始监听各种事件
int ev_run_loop(ev_loop_t *loop);
```
