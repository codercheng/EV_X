###目录
* [ABOUT](#about)
* [DEMO](#demo)
	* [ev_httpd](#ev_httpd)
	* [ev_demo](#simple_demo)
* [API](#apis)
* [TEST](#test)

<a name="about"></a>
ABOUT
======
这是一个基于epoll+timerfd的事件回调库。目前支持`io读写`事件，和`定时`事件。

---
目前在我的个人项目中，有很多地方都用到了这个事件回调库(有改动)，具体用法参考 [API](#apis)
* web server中用到了io事件回调来处理连接，同时用到了定时事件来处理Keep-alive的情况
* 在线聊天室中后台用到了定时事件，来做ajax long polling的定时

<a name="demo"></a>
###DEMO###
<a name="ev_httpd"></a>
**ev_httpd**
* 这是一个基于此回调库而写的一个简单高效的http server, 该server能轻松应对并发问题(单进程+eventloop+IO复用+非阻塞IO).
该httpd目前的功能很简单，接收http请求，返回“hello evlib”。
*
```
make
./ev_httpd 8080
```
* 打开浏览器，http://127.0.0.1:8080/


<a name="simple_demo"></a>
**ev demo**

这是一个最简单的demo，可以清楚的看到event loop的用法，io/timer事件相关操作的用法。
`这个ev_demo，主要是监听stdin的可读事件，当stdin有数据
可读，那么打印you have data to read. 同时程序里面监听了定时事件，每隔5s打印Timeout。`

###in your program###
* SYNOPSIS
```
#include "ev_loop.h"
#include "ev_type.h"
```
* EXAMPLE PROGRAM
```c
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

```
<a name="apis"></a>
###API###
`具体可以在ev_loop.h ev_type.h 中查看`

**event loop**

* 创建事件循环

```c
//创建一个 event loop并返回
ev_loop_t *ev_create_loop();
```

* 启动event loop

```c
//启动Loop，开始监听各种事件
int ev_run_loop(ev_loop_t *loop);
```

**io event**
* 启动io事件
```c
// et = 1: 用EPOLLET 模式， =0： 用EPOLLLT模式
int ev_io_start(ev_loop_t *loop, int max_ev_num, int etmodel);
```

* 注册io事件(`EV_READ, EV_WRITE`)到循环中

```c
//在fd上注册感兴趣的events
//当fd上events发生时，调用cb
//可以重复注册，改变cb
int ev_io_register(ev_loop_t*loop, int fd, EV_TYPE events, cb_func_t cb, void *ptr);
```

* 注销fd上io事件

```c
//注销fd,即fd上的所有事件脱离循环
int ev_io_unregister(ev_loop_t *loop, int fd);
```

* 停止io事件

```c
//这个不同于注销，而是只停止某个事件，比如fd上同时注册了
//可读和可写event, 那么我可以值stop可读事件
int ev_io_stop(ev_loop_t *loop, int fd, EV_TYPE events);
```
**timer event**

* 开启定时事件

```c
// capacity: 初始化timer可达数量
int ev_timer_start(ev_loop_t *loop, int capacity);
```

* 注册定时事件

```c
// timeout: 单位s，可精确到小数点后9位，也就是纳秒
// repeat: 0-->只定时一次， 1-->循序定时，无穷无尽
int ev_timer_register(ev_loop_t *loop, double timeout, cb_timer_t cb, uint8_t repeat, void *ptr);
```

* 注销定时事件 
```c
int ev_timer_unregister(ev_loop_t *loop, ev_timer_t *timer);
```
<a name="test"></a>
TestCase
======
* timer
	* 后加入的timer反而比目前已经注册的timer先到期的情况
	* timer 数量由0变1,和由1变0需要特别注意，都需要timerfd_settime，前者设置为给定值，后者置0
	* ts为0和负的情况，考虑到
	* 单次和无限循序定时混合test
	* timer 10000 count

* IO

* valgrind memleak test && leak fd test
```
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./demo
```
---
