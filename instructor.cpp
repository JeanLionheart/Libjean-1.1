#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <iostream>
#include <malloc.h>
#include <vector>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <signal.h>
#include <condition_variable>
#include <time.h>

int setNoneBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Socket(const char *ip, int port, int backlog)
{
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(tcp_sock >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(address.sin_addr));
    address.sin_port = htons(port);

    int ret = bind(tcp_sock, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(tcp_sock, backlog);
    assert(ret != -1);
}

void Recv(int sock)
{
    char buffer[1024];
    memset(buffer, '\0', 1024);

    int ret = recv(sock, buffer, 1024 - 1, 0);
    assert(ret != -1);
}

void Send(int sock)
{
    const char *msg = "hello world!\n";
    int ret = send(sock, msg, strlen(msg), 0);
    assert(ret != -1);
}

void Accept(int listenfd)
{
    int ret = accept(listenfd, 0, 0);
    assert(ret != -1);
}
/* 




 */
struct pollfd pfds[1024];
int poll_size = 0;
void Poll_add(int sock)
{
    pfds[poll_size].fd = sock;
    pfds[poll_size].events = POLLIN | POLLERR | POLLRDHUP;
    pfds[poll_size].revents = 0;
    setNoneBlocking(sock);
    poll_size++;
}

void Poll()
{
    poll(pfds,poll_size,-1);
}

void exam_poll(){
    for(int i=0;i<poll_size;i++){
        if(pfds[i].revents & POLLIN){
            //recv
        }
        if(pfds[i].revents & POLLERR){
            
        }
        if(pfds[i].revents & POLLRDHUP){
            
        }
    }
}
/* 



 */
int epfd;
void Eoll_create(){
    epfd=epoll_create(5);
}

void Epoll_add(int sock){
    setNoneBlocking(sock);
    epoll_event e;
    e.data.fd=sock;
    e.events=EPOLLIN|EPOLLERR|EPOLLRDHUP;
    epoll_ctl(epfd,EPOLL_CTL_ADD,sock,&e);
}

void Epoll_wait(){
    epoll_event epevt[1024];
    int number=epoll_wait(epfd,epevt,1024,-1);
    for(int i=0;i<number;i++){
        int s=epevt[i].data.fd;
        if(epevt[i].events & EPOLLIN){
            //recv
        }
        //if....{ ..  handle...}
    }
}