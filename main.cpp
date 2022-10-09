#include<iostream>
#include<sys/types>
#include<sys/socket>
#include<arpa/inet.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "args cnt less than 2" << std::endl;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htos(port);
    return 0;
}
