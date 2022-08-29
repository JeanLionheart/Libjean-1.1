#ifndef __JALCTPC__
#define __JALCTPC__
#include <malloc.h>
#include <iostream>
#include <mutex>
#include <atomic>
namespace jean
{
    template <typename T>
    class jallocator_t // typical allocator, learned from JJH
    {
    private:
        /* data */
        struct node_t
        {
            /* data */
            node_t *next;
        };
        int pool_size;
        size_t max(size_t a, size_t b)
        {
            return a > b ? a : b;
        }
        node_t *freeChunk;
        size_t __size;
        std::mutex mu;
        void init()
        {
            node_t *head;
            node_t *lp = head = (node_t *)malloc(__size * pool_size);
            if (!lp)
                return;
            for (int i = 1; i < pool_size; i++)
            {
                lp->next = (node_t *)((char *)lp + __size);
                lp = lp->next;
            }
            pool_size *= 2;
            lp->next = nullptr;
            freeChunk = head;
        }

    public:
        void *allocate()
        {
            mu.lock();
            void *res = freeChunk;
            if (!res)
            {
                init();
                mu.unlock();
                return allocate();
            }
            freeChunk = ((node_t *)res)->next;
            mu.unlock();
            return res;
        }
        void deallocate(void *lp)
        {
            if (!lp)
                return;
            mu.lock();
            ((node_t *)lp)->next = freeChunk;
            freeChunk = (node_t *)lp;
            mu.unlock();
        }
        jallocator_t();
        ~jallocator_t();
    };

    template <typename T>
    jallocator_t<T>::jallocator_t() : freeChunk(nullptr), __size(max(8, sizeof(T))), pool_size(32)
    {
    }

    template <typename T>
    jallocator_t<T>::~jallocator_t()
    {
    }
}
#endif