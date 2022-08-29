#ifndef __JSOCK__
#define __JSOCK__
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
#include "../structure/jtree.cpp"
#include "../structure/jque.cpp"
#include "../structure/jheap.cpp"
#define LT false
#define ET true
#define BUFFER_SIZE 64
#define SFD 0
#define LFD 1
#define TCPMODE 0
#define UDPMODE 1
using std::cout;
using std::endl;
namespace jean
{
    int sig_handler_fd;
    void make_sig_handler_fd(int f)
    {
        sig_handler_fd = f;
    }
    void sig_handler(int sig)
    {
        cout<<"SIGHANDLE\n"<<endl;
        int old_e = errno;
        int msg = sig;
        int ret=send(sig_handler_fd, (char*)&msg, 1, 0);
        errno = old_e;
    }
    void add_sig(int sig)
    {
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler = sig_handler;
        sa.sa_flags |= SA_RESTART;
        sigfillset(&sa.sa_mask);
        assert(sigaction(sig, &sa, NULL) != -1);
    }
    int setNoneBlocking(int fd)
    {
        int old_option = fcntl(fd, F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(fd, F_SETFL, new_option);
        return old_option;
    }
    class jsock
    {
    private:
        int create_sock(const char *, int, int, bool);
        int create_sock(bool);

    public:
        jsock();
        ~jsock();

        int create_sock_tcp(const char *, int, int);
        int create_sock_udp(const char *, int, int);
        int create_sock_tcp();
        int create_sock_udp();

        int accepted(int);
        void connected(const char *, int, int);
        char *recv(int);
        void send(const char *, int);

        void display_end(int);
        void close_fd(int); //
    };

    char *jsock::recv(int connectfd) // when waiting for msg, park
    {
        char *buffer = (char *)malloc(BUFFER_SIZE);

        memset(buffer, '\0', BUFFER_SIZE);
        int ret = ::recv(connectfd, buffer, BUFFER_SIZE - 1, 0);
        assert(ret != -1);
        return buffer;
    }

    void jsock::send(const char *buffer, int connectfd)
    {
        int ret = ::send(connectfd, buffer, strlen(buffer), 0);
        assert(ret != -1);
    }

    int jsock::create_sock(const char *ip, int port, int backlog, bool mode)
    {
        int __type;
        if (mode == UDPMODE)
        {
            __type = SOCK_DGRAM;
        }
        else
        {
            __type = SOCK_STREAM;
        }
        int sockfd = socket(PF_INET, __type, 0);
        assert(sockfd >= 0);

        sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &address.sin_addr);
        address.sin_port = htons(port);

        int ret = bind(sockfd, (sockaddr *)&address, sizeof(address));
        assert(ret != -1);

        if (mode == TCPMODE)
        {
            std::cout << "tcp sock created" << std::endl
                      << std::endl;
            ret = listen(sockfd, backlog);
            assert(ret != -1);
        }
        else
        {
            std::cout << "udp sock created" << std::endl
                      << std::endl;
        }

        return sockfd;
    }

    int jsock::create_sock_tcp(const char *ip, int port, int backlog)
    {
        return create_sock(ip, port, backlog, TCPMODE);
    }

    int jsock::create_sock_tcp()
    {
        return create_sock(TCPMODE);
    }

    int jsock::create_sock_udp(const char *ip, int port, int backlog)
    {
        return create_sock(ip, port, backlog, UDPMODE);
    }

    int jsock::create_sock_udp()
    {
        return create_sock(UDPMODE);
    }

    void jsock::connected(const char *ip, int port, int sockfd)
    {
        sockaddr_in server_address;
        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        inet_pton(AF_INET, ip, &server_address.sin_addr);
        server_address.sin_port = htons(port);

        assert(sockfd >= 0);
        int cnct = connect(sockfd, (sockaddr *)&server_address, sizeof(server_address));
        assert(cnct >= 0);
    }

    int jsock::create_sock(bool mode)
    {
        int __type;
        if (mode)
        {
            __type = SOCK_DGRAM;
        }
        else
        {
            __type = SOCK_STREAM;
        }
        int sockfd = socket(PF_INET, __type, 0);
        assert(sockfd >= 0);
        return sockfd;
    }

    int jsock::accepted(int listenfd)
    {
        sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int acpt = accept(listenfd, (sockaddr *)&client, &client_len);
        assert(acpt != -1);

        return acpt;
    }

    jsock::jsock()
    {
    }

    jsock::~jsock()
    {
    }

    /* epfd, automaticly close connection that has been closed by the other end */
    class jepoll
    {
    public:
        struct msg;

    private:
        jsock jso;
        int epfd;
        bool mdfg; // mode flag. LT or ET
        jtree_avl<bool> sockfd_collector;

        std::mutex cmu;

        void collect(epoll_event *, int);
        int create_epoll();
        jque<msg, jalloc_conc> msg_que;

    public:
        /* the sockfd in ep_eait() */
        jector<int> clients;
        struct msg
        {
            /* data */
            char *buf;
            int sockfd;
        };

        bool empty()
        {
            return msg_que.empty();
        }
        msg *pop()
        {
            return msg_que.pop_front();
        }
        jepoll();
        ~jepoll();
        void ep_add(int, bool);
        void ep_del(int);
        void ep_wait();
        char *int2str(int);
    };

    jepoll::jepoll() : epfd(create_epoll()), mdfg(LT), msg_que()
    {
    }

    jepoll::~jepoll()
    {
        while (!sockfd_collector.is_empty())
        {
            int k = sockfd_collector.get_top();
            ep_del(k);
        }
        shutdown(epfd, SHUT_RDWR);
    }

    int jepoll::create_epoll()
    {
        int nums = 5;
        int ret = epoll_create(nums);
        assert(ret != -1);
        return ret;
    }

    void jepoll::ep_add(int fd, bool fdfg)
    {
        // epoll_event *evt = (epoll_event *)malloc(sizeof(epoll_event));
        epoll_event evt;
        evt.data.fd = fd;
        evt.events = EPOLLIN;
        if (mdfg)
        {
            cout << "add ET evt" << endl;
            evt.events |= EPOLLET;
        }
        else
        {
            cout << "add LT evt" << endl;
        }
        sockfd_collector.insert(fd, fdfg);
        if (fdfg == SFD)
        {
            cmu.lock();
            clients.push_back(fd);
            cmu.unlock();
        }
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
        cout << fd << " ep_added" << endl
             << endl;
        setNoneBlocking(fd);
    }

    void jepoll::ep_del(int sockfd)
    {
        if (!sockfd_collector.find(sockfd))
        {
            return;
        }
        epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, 0);

        cmu.lock();
        int addr = clients.find(sockfd);
        if (addr != -1)
        {
            clients.del(addr);
        }
        cmu.unlock();

        sockfd_collector.del(sockfd);
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        printf("the %d closed\n", sockfd);
        cout << endl;
    }

    void jepoll::ep_wait()
    {
        epoll_event events[1024];
        cout << "epoll_wait" << endl;
        int ret = epoll_wait(epfd, events, 1024, -1);
        // assert(ret >= 0);//because -1, must park, ret > 0;
        if (ret < 0)
        {
            cout << "epoll_wait failed" << endl
                 << endl;
            return;
        }
        cout << "events come : " << ret << endl
             << endl;
        collect(events, ret);
    }

    void jepoll::collect(epoll_event *events, int number)
    {
        char *buf = 0;
        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            cout << sockfd << endl;
            bool *p = sockfd_collector.find(sockfd);
            if (*p == LFD)
            {
                cout << "cnct rcv" << endl;
                int acpt = jso.accepted(sockfd);
                assert(acpt != -1);
                ep_add(acpt, SFD);
                msg m;
                m.buf = int2str(acpt);
                m.sockfd = sockfd;
                msg_que.push_back(m);
            }
            else if (events[i].events & EPOLLIN)
            {
                if (mdfg == LT) // LT
                {
                    cout << "LT mode" << endl;
                    buf = (char *)malloc(BUFFER_SIZE);
                    memset(buf, '\0', BUFFER_SIZE);
                    int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                    msg m;
                    m.buf = buf;
                    m.sockfd = sockfd;
                    msg_que.push_back(m);
                    if (ret <= 0)
                    {
                        cout << "no msg from " << sockfd << endl
                             << endl;
                        ep_del(sockfd);
                        continue;
                    }

                    cout << "msg rcv from " << sockfd << endl
                         << endl;
                }
                else // ET
                {
                    cout << "ET mode" << endl
                         << endl;
                    while (1)
                    {
                        buf = (char *)malloc(BUFFER_SIZE);
                        memset(buf, '\0', BUFFER_SIZE);
                        int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                        if (ret < 0)
                        {
                            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                            {
                                break;
                            }
                            break;
                        }
                        else if (ret == 0)
                        {
                            ep_del(sockfd);
                            cout << "no msg from " << sockfd << endl
                                 << endl;
                        }
                        else
                        {
                            msg m;
                            m.buf = buf;
                            m.sockfd = sockfd;
                            msg_que.push_back(m);
                            cout << "msg rcv from " << sockfd << endl
                                 << endl;
                        }
                    }
                }
            }
        }
    }
    char *jepoll::int2str(int n)
    {
        cout << n << endl;
        char *buf = (char *)malloc(BUFFER_SIZE);
        memset(buf, '\0', BUFFER_SIZE);
        int i = 0;
        while (n)
        {
            buf[i] = (n % 10) + '0';
            n /= 10;
            i++;
        }
        for (int j = i - 1, k = 0; k < j; j--, k++)
        {
            char c = buf[j];
            buf[j] = buf[k];
            buf[k] = c;
        }
        return buf;
    }

    /* a simple poll */
    class jpoll
    {
    private:
        /* data */

        pthread_rwlock_t prw;
        jector<pollfd> pfds;
        std::atomic_int __size;

    public:
        pollfd &operator[](int k)
        {
            return pfds[k];
        }
        int size()
        {
            return __size.load();
        }
        void p_add(int);
        void p_del(int);
        void p_wait();
        jpoll();
        ~jpoll();
    };

    jpoll::jpoll() : pfds(), __size(0)
    {
        pthread_rwlock_init(&prw,0);
    }

    jpoll::~jpoll()
    {
    }

    /* add a sockfd to be listened by poll */
    void jpoll::p_add(int sockfd) // listen read event
    {
        pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN | POLLERR | POLLRDHUP;
        pfd.revents = 0;
        setNoneBlocking(sockfd);
        pthread_rwlock_wrlock(&prw);
        pfds.push_back(pfd);
        pthread_rwlock_unlock(&prw);
        __size++;
    }

    /* delete a listened sockfd */
    void jpoll::p_del(int sockfd)
    {
        pthread_rwlock_wrlock(&prw);
        for (int i = 0; i < pfds.size(); i++)
        {
            if (pfds[i].fd == sockfd)
            {
                __size--;
                pfds.del(i);
                break;
            }
        }
        pthread_rwlock_unlock(&prw);
    }

    /* block, wait for POLLIN event*/
    void jpoll::p_wait()
    {
        int r = -1;
        pthread_rwlock_rdlock(&prw);
        if (pfds.is_empty())
        {
            pthread_rwlock_unlock(&prw);
            return;
        }
        cout<<"poll wait\n"<<endl;
        r = poll(&(pfds[0]), pfds.size(), -1);
        pthread_rwlock_unlock(&prw);
        return;
    }

    /* manage invalid connect automaticly */
    class cnct_manager
    {
    private:
        /* data */
        void (*call_back)(int);
        int secs;
        jtree_avl<time_t> reg;
        struct cnctfd_t
        {
            /* data */
            int cnct;
            time_t expire;
        };
        jque<cnctfd_t> jq;

    public:
        void callback(int s)
        {
            call_back(s);
        }
        cnct_manager(int, void (*)(int));
        ~cnct_manager();
        void push(int);
        void invalid(int);
        void adjust(int);
        void tick();
    };

    cnct_manager::cnct_manager(int s, void (*f)(int)) : secs(s), call_back(f)
    {
        secs=max(1,s);
        alarm(secs * 2);
    }

    cnct_manager::~cnct_manager() {}

    void cnct_manager::push(int sockfd)
    {
        printf("manage %d\n", sockfd);
        cout << endl;
        time_t cur = time(0);
        cnctfd_t c;
        c.expire = cur + 2 * secs;
        c.cnct = sockfd;
        jq.push_back(c);
    }

    void cnct_manager::invalid(int sockfd)
    {
        reg.insert(sockfd, -1);
    }

    void cnct_manager::adjust(int sockfd)
    {
        time_t *tt = reg.find(sockfd);
        time_t expire = time(0) + secs * 2;
        if (tt)
        {
            if (*tt == -1)
            {
                return;
            }
            reg.update(sockfd, expire);
        }
        else
        {
            reg.insert(sockfd,expire);
        }
    }

    void cnct_manager::tick()
    {
        while (1)
        {
            cnctfd_t *lp = jq.pop_front();
            time_t cur = time(0);
            if (!lp)
                break;
            time_t *e = reg.find(lp->cnct);
            if (!e)
            {
                if (lp->expire < cur) // out of date
                {
                    call_back(lp->cnct);
                    jq.return_data(lp);
                }
                else
                { // not out of date
                    jq.push_front(*lp);
                    jq.return_data(lp);
                    break;
                }
            }
            else
            {
                if (*e == -1)
                { // invalide
                    reg.del(lp->cnct);
                    jq.return_data(lp);
                }
                else
                { // update
                    lp->expire = *e;
                    if (lp->expire < cur)
                    {
                        call_back(lp->cnct);
                        reg.del(lp->cnct);
                        jq.return_data(lp);
                    }
                    else
                    {
                        jq.push_back(*lp);
                        jq.return_data(lp);
                        break;
                    }
                }
            }
        }
        alarm(secs);
    }

}
#endif
