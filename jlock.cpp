#ifndef __JLOCKRW__
#define __JLOCKRW__
#include <mutex>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <iostream>
#include<pthread.h>
namespace jean
{

    /* read write lock */
    class jlock_rw
    {
    private:
        /* data */
        std::mutex __mu;
        std::atomic_int r_threads, w_threads;

    public:
        jlock_rw(/* args */);
        ~jlock_rw();
        jlock_rw(jlock_rw &j);
        jlock_rw(jlock_rw &&j);
        void write_lock()
        {
            /* std::cout<<"wl"<<std::endl; */
            for (;;)
            {
                w_threads.fetch_add(1);
                if (0 == r_threads)
                {
                    __mu.lock(); // sleep , wait for write threads
                    return;
                }
                else // spin , wait for read threads. read is fast, so spin but not sleep
                {
                    w_threads.fetch_sub(1);
                    std::this_thread::yield();
                    // sleep until r_threads==0
                }
            }
        }
        void write_unlock()
        {
            __mu.unlock();
            w_threads.fetch_sub(1);
        }
        void read_lock()
        {
            /* std::cout<<"rl"<<std::endl; */
            r_threads.fetch_add(1);
            int times = 0;
            while (w_threads > 0)
            { // spin and wait for write threads
                times++;
                std::this_thread::yield();
                /* if (times > 5000)
                {
                    __mu.lock();
                    __mu.unlock();
                } */
            }
        }
        void read_unlock()
        {
            r_threads.fetch_sub(1);
        }
    };

    jlock_rw::jlock_rw(/* args */) :  __mu(), r_threads(0), w_threads(0)
    {
    }

    jlock_rw::jlock_rw(jlock_rw &j) :  __mu(), r_threads(0), w_threads(0)
    {
    }

    jlock_rw::jlock_rw(jlock_rw &&j) :  __mu(), r_threads(0), w_threads(0)
    {
    }

    jlock_rw::~jlock_rw()
    {
    }
   














    /* spin lock */
    class jlock_spin
    {
    private:
        /* data */
        std::atomic_bool _f;

    public:
        jlock_spin(/* args */);
        ~jlock_spin();
        jlock_spin(jlock_spin &);
        jlock_spin(jlock_spin &&);
        void lock()
        {
            bool fal = 0;
            while (!_f.compare_exchange_strong(fal, 1, std::memory_order_relaxed))
            {
                fal = 0;
            }
        }
        void unlock()
        {
            _f.store(0, std::memory_order_relaxed);
        }
    };

    jlock_spin::jlock_spin(/* args */) :  _f(0)
    {
    }

    jlock_spin::~jlock_spin()
    {
    }
    jlock_spin::jlock_spin(jlock_spin &j) :  _f(0)
    {
    }
    jlock_spin::jlock_spin(jlock_spin &&j) :  _f(0)
    {
    }
    /* 
    
    
    
    
    
    
    
    
    
    
     */
    /* Fitting the situation ,when read_op is a long-time action,
    and write is short  */
    class plock_rw
    {
    private:
        /* data */
        pthread_rwlock_t __plock;
    public:
        plock_rw(/* args */);
        ~plock_rw();
        void read_lock(){
            pthread_rwlock_rdlock(&__plock);
        }
        void write_lock(){
            pthread_rwlock_wrlock(&__plock);
        }
        void unlock(){
            pthread_rwlock_unlock(&__plock);
        }
    };
    
    plock_rw::plock_rw(/* args */)
    {
        pthread_rwlock_init(&__plock,0);
    }
    
    plock_rw::~plock_rw()
    {
        pthread_rwlock_destroy(&__plock);
    }
    


}
 // namespace jean
#endif