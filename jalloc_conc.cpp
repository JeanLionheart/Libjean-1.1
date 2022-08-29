#ifndef __JALLOCTYPICCONC__
#define __JALLOCTYPICCONC__
#include <atomic>
#include <malloc.h>
#include <iostream>
#include <mutex>
#include <thread>
#define mfence asm volatile("mfence" :: \
                                : "memory")
using std::cout;
using std::endl;
namespace jean
{
    template <typename T>
    class jalloc_conc // a thread_safe concurrency allocator ,without lock except inition , can not return mem to OS
    {
    private:
        /* data */ 
        unsigned short __size; // obj __size
        unsigned short max(unsigned short a, unsigned short b)
        {
            return a > b ? a : b;
        }

        struct node
        {
            node *next;
        };
        std::atomic<node *> freeChunk;

        std::mutex mu;
        std::atomic_int popingthreads;

        void init()
        {
            node *lp = (node *)malloc(__size * 64);
            node *head = lp;
            for (int i = 1; i < 64; i++)
            {
                lp->next = (node *)((char *)lp + __size);
                lp = lp->next;
            }
            lp->next = freeChunk.load();
            while (!freeChunk.compare_exchange_strong(lp->next, head))
            ;
        }

    public:
        void *allocate();
        void deallocate(void *);
        jalloc_conc();
        ~jalloc_conc();
    };

    template <typename T>
    jalloc_conc<T>::jalloc_conc() : __size(max(8, sizeof(T))), freeChunk(nullptr), mu(), popingthreads(0)
    {
        init();
    }

    template <typename T>
    jalloc_conc<T>::~jalloc_conc()
    {
    }

    template <typename T>
    void *jalloc_conc<T>::allocate() // here is same to stack_pop
    {
        if (!freeChunk.load())
        {
            mu.lock();
            if (!freeChunk.load())
            {
                init();
            }
            mu.unlock();
        }
        popingthreads++;
        node* res = freeChunk.load();
        while (res && !freeChunk.compare_exchange_strong(res, res->next, std::memory_order_seq_cst))
            ;
        popingthreads--;
        while(popingthreads)std::this_thread::yield();
        if (!res)
            return allocate();
        return res;
    }

    template <typename T>
    void jalloc_conc<T>::deallocate(void *lp)
    {
        node *p = (node *)lp;
        p->next = freeChunk.load();        
        while (!freeChunk.compare_exchange_strong(p->next, p, std::memory_order_seq_cst))
            ;
    }
}
#undef mfence
#endif