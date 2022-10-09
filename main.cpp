#include <iostream>
#include <sys/types>
#include <sys/socket> // socket
#include <arpa/inet.h> // inet_pton
#include <stdlib.h> // atoi
#include <string.h> // bzero
#include <assert.h> // assert
#include <errno.h> // errno

const int FD_LIMIT = 65535;
const int MAX_EVENT_NUM = 1024;


static int pipefd; // 信号和定时器通过管道统一为fd处理

int setNonBlock(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

void addfd(int fd, int epollfd) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.data = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setNonBlock(fd);
}

void sig_handler(int sig) {
    int back_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0); // 参数意思?
    errno = back_errno;
}

void addsig(int sig) {
    sigaction sa;
    memeset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "args cnt less than 2" << std::endl;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htos(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    
    int res = 0;
    
    res = bind(listenfd,(addr_in*)&addr, sizeof(addr));
    assert(res != -1);

    res = listen(listenfd, 5);
    assert(res != -1);

    epoll_event evs[MAX_EVENT_NUM];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(listenfd, epollfd);

    res = socketpair(PF_INET, SOCK_STREAM, 0, pipefd);
    assert(res != -1);
    setNonBlock(pipefd[1]); // 非阻塞写
    addfd(pipefd[0], epollfd); // 监听是否可读

    addsig(SIGALRM);
    addsig(SIGTERM);
    
    ClientData* users = new ClientData[FD_LIMIT];
    bool time_out = false;
    bool stop = false;
    while (!stop) {
        int num = epoll_wait(epollfd, evs, MAX_EVENT_NUM, -1); // 参数意思
        if (num < 0 && errno != EINTR) {
            std::cout << "epoll failure" << std::endl;
            break;
        }
        for (int i = 0; i < num; i++) {
            int sockfd = evs[i].data.fd;
            if (sockfd == listenfd) {
                // 处理连接事件
            } else if (sockfd == pipefd[0] && (evs[i].events & EPOLLIN)) {
                // 处理信号事件
            } else if (evs.events & EPOLLIN) {
                // 处理读数据
            } else {
                // 其他
            }
        }
    }
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] users;
    return 0;
}
