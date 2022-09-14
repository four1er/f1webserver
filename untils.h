#ifndef UNTILS_H
#define UNTILS_H

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

int setnonblocking(int fd);

void addfd(int epollfd, int fd, bool one_shot);

void removefd(int epollfd, int fd);

void modfd(int epollfd, int fd, int ev);

#endif