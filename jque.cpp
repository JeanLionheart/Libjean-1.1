#ifndef __JQUE__
#define __JQUE__
#include "../jallocator/jalloc_conc.cpp"
#include <mutex>
#include<pthread.h>
#include <iostream>
#include <atomic>
#define mfence asm volatile("mfence" :: \
                                : "memory")
namespace jean
{
    template <typename T, template <typename N> class jalloc_t = jalloc_conc>
    class jque
    { // thread_safe
    private:
        struct node_t
        {
            /* data */
            node_t *next;
            T*val;
        };
        jalloc_t<T> data_jalloc;
        jalloc_t<node_t> jalloc;
        node_t *head, *tail;
        std::mutex muh, mut;
        std::atomic_int _size;
        pthread_spinlock_t psplockh,psplockt;
    public:
        jque();
        ~jque();

    public:
        void push_back(T &);
        void push_back(T &&);
        void push_front(T &);
        void push_front(T &&);
        T *pop_front();
        bool empty()
        {
            return !_size.load();
        }
        void return_data(void *lp)
        {
            data_jalloc.deallocate(lp);
        }
    };

    template <typename T, template <typename N> class jalloc_t>
    jque<T, jalloc_t>::jque() : head(nullptr), tail(nullptr), _size(0), muh(), mut()
    {
        head = (node_t *)jalloc.allocate();
        tail = head;
        head->next = nullptr;
        pthread_spin_init(&psplockh);
        pthread_spin_init(&psplockt);
    }

    template <typename T, template <typename N> class jalloc_t>
    jque<T, jalloc_t>::~jque()
    {
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_back(T &v)
    {
        node_t *new_tail = (node_t *)jalloc.allocate();
        new_tail->next = nullptr;
        T*new_value=(T*)data_jalloc.allocate();
        new(new_value)T(v);
        pthread_spin_lock(&psplockt);
        tail->val=new_value;
        tail->next = new_tail;
        tail = new_tail;
        pthread_spin_unlock(&psplockt);       
        _size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_back(T &&v)
    {
        node_t *new_tail = (node_t *)jalloc.allocate();
        new_tail->next = nullptr;
        T*new_value=(T*)data_jalloc.allocate();
        new(new_value)T(std::move(v));
        pthread_spin_lock(&psplockt);
        tail->val=new_value;
        tail->next = new_tail;
        tail = new_tail;
        pthread_spin_unlock(&psplockt);       
        _size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    T *jque<T, jalloc_t>::pop_front()
    {
        int k = _size--;
        if (k <= 0)
        {
            _size++;
            return nullptr;
        }        
        pthread_spin_lock(&psplockh);
        node_t *r = head;
        head = r->next;
        pthread_spin_unlock(&psplockh);
        T *res=r->val;
        jalloc.deallocate(r);
        return res;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_front(T &v)
    {
        node_t *new_head = (node_t *)jalloc.allocate();
        new_head->val=(T*)data_jalloc.allocate();
        new(new_head->val)T(v);
        pthread_spin_lock(&psplockh);
        new_head->next = head;
        head = new_head;
        pthread_spin_unlock(&psplockh);
        _size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_front(T &&v)
    {
        node_t *new_head = (node_t *)jalloc.allocate();
        new_head->val=(T*)data_jalloc.allocate();
        new(new_head->val)T(std::move(v));
        pthread_spin_lock(&psplockh);
        new_head->next = head;
        head = new_head;
        pthread_spin_unlock(&psplockh);
        _size++;
    }
}
#undef mfence
#endif
