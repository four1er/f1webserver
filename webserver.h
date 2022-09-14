#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "./http_conn.h"
#include "./threadpool/threadpool.h"
#include <cstdio>
#include <cstring>
#include <iostream>

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000

class Webserver {
  public:
    Webserver(int port = 9999);
    ~Webserver();
    void set_reuse();
    void set_bind();
    void set_listen(int queue_len = 10);
    int epoll_init();
    void run();
    int get_fd();
    int get_port();

  private:
    int m_listen_fd;
    int m_port;
    struct sockaddr_in m_saddr;
    int m_epfd;
    struct epoll_event m_epev;
    struct epoll_event m_epevs[MAX_EVENT_NUMBER];
    Threadpool<Http_conn> *pool;
    Http_conn *users;
};

#endif