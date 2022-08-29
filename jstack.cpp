#ifndef __LKFRSTK__
#define __LKFRSTK__
#include <atomic>
#include <thread>
#include <malloc.h>
#include <mutex>
#include "../jallocator/jallocator_t.cpp"
#include "../jallocator/jalloc_origin.cpp"
#include "../jallocator/jalloc_conc.cpp"
namespace jean
{
    template <class T, template <typename N> class jalloc_t = jalloc_conc>
    class jstack // thread-safe 100%
    {
    private:
        std::atomic_int __size;

        struct node_t
        {
            node_t *next;
            std::mutex mu;
            T*data;
            node_t() : data(nullptr), next(nullptr) {}
            ~node_t()
            {
            }
        };
        jalloc_t<node_t> jalloc;
        jalloc_t<T> data_jalloc;
        std::atomic<node_t *> head;
        std::atomic_int popingThreads;

    public:
        void push(T &);
        void push(T &&);
        T *pop();
        void return_data(void *);
        bool empty()
        {
            return (__size.load() == 0);
        }
        jstack(/* args */);
        ~jstack();
    };

    template <class T, template <typename N> class jalloc_t>
    jstack<T, jalloc_t>::jstack(/* args */) : __size(0), head(nullptr), popingThreads(0), jalloc(), data_jalloc()
    {
    }
    template <class T, template <typename N> class jalloc_t>
    jstack<T, jalloc_t>::~jstack()
    {
    }

    template <class T, template <typename N> class jalloc_t>
    void jstack<T, jalloc_t>::push(T &data)
    {
        node_t *lp = (node_t *)jalloc.allocate(); // lp=allocate(); new(lp)node_t(data);here is the bug
        lp->data=(T*)data_jalloc.allocate();
        new (lp->data) T(data);

        lp->next = head.load();
        while (!head.compare_exchange_strong(lp->next, lp))
            ;
        __size++;
    }

    template <class T, template <typename N> class jalloc_t>
    void jstack<T, jalloc_t>::push(T &&data)
    {
        node_t *lp = jalloc.allocate(); // lp=allocate(); new(lp)node_t(data);here is the bug
        lp->data=(T*)data_jalloc.allocate();
        new (lp->data) T(std::move(data));

        lp->next = head.load();
        while (!head.compare_exchange_strong(lp->next, lp))
            ;
        __size++;
    }

    template <class T, template <typename N> class jalloc_t>
    T *jstack<T, jalloc_t>::pop()
    {
        popingThreads++;
        node_t *old_head = head.load();
        while (old_head && !head.compare_exchange_strong(old_head, old_head->next))
            ;
        popingThreads--;
        while (popingThreads)
            std::this_thread::yield();
        if (!old_head)
            return nullptr;
        __size--;
        T *res = old_head->data;
        jalloc.deallocate(old_head);
        return res;
    }

    template <class T, template <typename N> class jalloc_t>
    void jstack<T, jalloc_t>::return_data(void *lp)
    {
        data_jalloc.deallocate(lp);
    }
}
#endif