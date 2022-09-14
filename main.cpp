#include <cstdio>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <iostream>
#include <sys/socket.h>

#include "./threadpool/threadpool.h"
#include "http_conn.h"
#include "untils.h"
#include "webserver.h"

// extern void addfd(int epollfd, int fd, bool one_shot);
// extern void removefd(int epollfd, int fd);

void addsig(int sig, void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char *argv[]) {
    int port = 9999;
    if (argc <= 1) {
        printf("usage: %s port_number\n", basename(argv[0]));
        printf("if not set port, it will set 9999\n");
    } else {
        port = atoi(argv[1]);
    }

    Webserver webserver{port};

    printf("--------- web server ---------\n");
    printf("[*] port: %d\n", port);

    webserver.set_bind();
    webserver.set_listen();

    addsig(SIGPIPE, SIG_IGN);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = webserver.epoll_init();

    webserver.run();
    return 0;
}
