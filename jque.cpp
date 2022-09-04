#ifndef __JQUE__
#define __JQUE__
#include "../jallocator/jalloc_conc.cpp"
#include <mutex>
#include <pthread.h>
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
            T *val;
        };
        jalloc_t<T> data_jalloc;
        jalloc_t<node_t> jalloc;
        node_t *head, *tail;
        std::mutex muh, mut;
        std::atomic_int _size;

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
        T *new_value = (T *)data_jalloc.allocate();
        new (new_value) T(v);
        mut.lock();
        tail->val = new_value;
        tail->next = new_tail;
        tail = new_tail;
        mut.unlock();
        _size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_back(T &&v)
    {
        node_t *new_tail = (node_t *)jalloc.allocate();
        new_tail->next = nullptr;
        T *new_value = (T *)data_jalloc.allocate();
        new (new_value) T(std::move(v));
        mut.lock();
        tail->val = new_value;
        tail->next = new_tail;
        tail = new_tail;
        mut.unlock();
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
        muh.lock();
        node_t *r = head;
        head = r->next;
        muh.unlock();
        T *res = r->val;
        jalloc.deallocate(r);
        return res;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_front(T &v)
    {
        node_t *new_head = (node_t *)jalloc.allocate();
        new_head->val = (T *)data_jalloc.allocate();
        new (new_head->val) T(v);
        muh.lock();
        new_head->next = head;
        head = new_head;
        muh.unlock();
        _size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jque<T, jalloc_t>::push_front(T &&v)
    {
        node_t *new_head = (node_t *)jalloc.allocate();
        new_head->val = (T *)data_jalloc.allocate();
        new (new_head->val) T(std::move(v));
        muh.lock();
        new_head->next = head;
        head = new_head;
        muh.unlock();
        _size++;
    }
    /*
















     */
    template <typename T, template <typename N> class jalloc_t = jalloc_conc>
    class jqueue
    {
    private:
        /* data */
        struct node_t
        {
            /* data */
            T *val;
            std::atomic<node_t *> next;
        };
        jalloc_t<T> data_jalloc;
        jalloc_t<node_t> jalloc;
        std::atomic<node_t *> head, tail;
        std::atomic_int size, head_thread, tail_thread;

    public:
        void push_head(T &);
        void push_tail(T &);
        T *pop_head();
        int get_size()
        {
            return size;
        }
        jqueue(/* args */);
        ~jqueue();
    };

    template <typename T, template <typename N> class jalloc_t>
    jqueue<T, jalloc_t>::jqueue(/* args */) : size(0)
    {
        node_t *node = (node_t *)jalloc.allocate();
        node->next = nullptr;
        node->val = (T *)data_jalloc.allocate();
        head.store(node);
        tail.store(head.load());
    }

    template <typename T, template <typename N> class jalloc_t>
    jqueue<T, jalloc_t>::~jqueue()
    {
    }

    template <typename T, template <typename N> class jalloc_t>
    void jqueue<T, jalloc_t>::push_head(T &v) // only read old_head.no danger
    {
        node_t *new_node = (node_t *)jalloc.allocate();
        new_node->val = (T *)data_jalloc.allocate();
        new (new_node->val) T(v);
        node_t *new_next = head.load();
        bool flag=0;
        while (!flag)
        {
            new_node->next.store(new_next);
            flag=head.compare_exchange_strong(new_next, new_node);
        }

        size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jqueue<T, jalloc_t>::push_tail(T &v)
    {
        node_t *new_node = (node_t *)jalloc.allocate();
        new_node->next = NULL;
        new_node->val = (T *)data_jalloc.allocate();
        node_t *old_tail = tail.load();
        while (!tail.compare_exchange_strong(old_tail, new_node))
            ;
        new (old_tail->val) T(v); // write old_tail
        old_tail->next = new_node;
        size++;
    }

    template <typename T, template <typename N> class jalloc_t>
    T *jqueue<T, jalloc_t>::pop_head()
    {
        int k = size--;
        if (k <= 0)
        {
            size++;
            return nullptr;
        }
        head_thread++;
        node_t *old_head = head.load();
        bool flag = 0;
        while (!flag)
        {
            if (old_head->next) // when true, the write old_tail is done
            {
                flag = head.compare_exchange_strong(old_head, old_head->next.load());
            }
        }
        T *r = old_head->val;
        head_thread--;
        while (head_thread)
            ;
        jalloc.deallocate(old_head);
        return r;
    }
}
#undef mfence
#endif
