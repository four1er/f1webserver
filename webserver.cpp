#include "./webserver.h"
#include <cstdio>

void Webserver::set_reuse() {
    int reuse = 1;
    setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
}

Webserver::Webserver(int port) : m_port(port), pool(nullptr) {
    m_saddr.sin_port = htons(m_port);
    m_saddr.sin_addr.s_addr = INADDR_ANY;
    m_saddr.sin_family = AF_INET;
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0) {
        std::cout << "create socket error!" << std::endl;
        exit(1);
    }
    set_reuse();
    try {
        pool = new Threadpool<Http_conn>;
    } catch (...) {
        perror("create threadpool error\n");
        return;
    }

    users = new Http_conn[MAX_FD];
}

Webserver::~Webserver() {
    if (m_listen_fd != -1) {
        close(m_listen_fd);
    }
    if (m_epfd != -1) {
        close(m_epfd);
    }
    delete[] users;
    users = nullptr;
    delete pool;
    pool = nullptr;
}

int Webserver::get_port() { return m_port; }

void Webserver::set_bind() {
    int ret = bind(m_listen_fd, (struct sockaddr *)&m_saddr, sizeof m_saddr);
    if (ret < 0) {
        std::cout << "bind error!\n";
        exit(-1);
    }
    std::cout << "bind success!\n";
}

void Webserver::set_listen(int queue_len) {
    int ret = listen(m_listen_fd, queue_len);
    if (ret < 0) {
        std::cout << "listen error!\n";
        exit(-1);
    }
    std::cout << "listen success!\n";
}

int Webserver::epoll_init() {
    printf("webserver epoll init....\n");
    m_epfd = epoll_create(100);
    // m_epev.data.fd = m_epfd;
    // m_epev.events = EPOLLIN | EPOLLRDHUP;
    // epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_listen_fd, &m_epev);
    addfd(m_epfd, m_listen_fd, false);
    Http_conn::m_epollfd = m_epfd;
    printf("webserver epoll fd: %d\n", m_epfd);
    return m_epfd;
}

void Webserver::run() {
    printf("webserver run....\n");
    while (1) {
        int number = epoll_wait(m_epfd, m_epevs, MAX_EVENT_NUMBER, -1);
        if ((number < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }

        for (int i = 0; i < number; i++) {
            int sockfd = m_epevs[i].data.fd;
            if (sockfd == m_listen_fd) {
                // 新的客户端接入连接
                struct sockaddr_in cliaddr;
                socklen_t len = 0;
                int connfd =
                    ::accept(m_listen_fd, (struct sockaddr *)&cliaddr, &len);

                if (connfd < 0) {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if (Http_conn::m_user_count >= MAX_FD) {
                    close(connfd);
                    continue;
                }
                users[connfd].init(connfd, cliaddr);
            } else if (m_epevs[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                users[sockfd].close_conn();
            } else if (m_epevs[i].events & EPOLLIN) {
                // 如果有数据进来
                if (users[sockfd].read()) {
                    pool->append(users + sockfd);
                } else {
                    users[sockfd].close_conn();
                }
            } else if (m_epevs[i].events & EPOLLOUT) {
                // 写数据
                if (!users[sockfd].write()) {
                    users[sockfd].close_conn();
                }
            }
        }
    }
}

int Webserver::get_fd() { return m_listen_fd; }