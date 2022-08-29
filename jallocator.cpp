#ifndef __ALKTSG__
#define __ALKTSG__
#include <malloc.h>
#include <iostream>
#include "../structure/jector.cpp"
using std::cout;
using std::endl;
namespace jean
{
    template <class T>
    class jalloc
    {
    private:
        /* data */
        struct node_t
        {
            /* data */
            node_t *next;
        };
        int __size;
        struct map_t
        {
            /* data */
            void *head;
            void *tail;
            node_t *freeptr;
            int rest;
            map_t() : head(0), tail(0), freeptr(0), rest(0) {}
            map_t(map_t &m) : head(m.head), tail(m.tail), freeptr(m.freeptr), rest(m.rest) {}
        };
        jector<map_t> map;
        bool include(int, void *);
        void init();

    public:
        void*allocate();
        void deallocate(void*);
        jalloc(/* args */);
        ~jalloc();
    };

    template <class T>
    jalloc<T>::jalloc(/* args */):__size(sizeof(T)>8?sizeof(T):8)
    {
    }

    template <class T>
    jalloc<T>::~jalloc()
    {
    }

    template <class T>
    bool jalloc<T>::include(int q, void *lp)
    {
        if((map[q].head <= lp)&&(map[q].tail >= lp)){
            return 1;
        }
        return 0;
    }

    template <class T>
    void jalloc<T>::init()
    {
        int q = map.size();
        map_t m;
        void *lp = malloc(__size * 64);
        m.head = lp;
        m.tail = ((char *)lp + 63 * __size);
        m.freeptr = (node_t *)lp;
        m.rest=64;
        node_t *needle = m.freeptr;
        for (int i = 1; i < 64; i++)
        {
            needle->next=(node_t*)((char*)needle+__size);
            needle=needle->next;
        }
        needle->next=0;
        map.push_back(m);
    }

    template <class T>
    void* jalloc<T>::allocate(){
        node_t*q=nullptr;
        for(int i=0;i<map.size();i++){
            if(map[i].rest>0){
                map[i].rest-=1;
                q=map[i].freeptr;
                map[i].freeptr=(map[i].freeptr)->next;
            }
        }
        if(!q){
            init();
            return allocate();
        }
        return q;
    }

    template <class T>
    void jalloc<T>::deallocate(void*lp){
        for(int i=0;i<map.size();i++){
            if(include(i,lp)){
                map[i].rest+=1;
                ((node_t*)lp)->next=map[i].freeptr;
                map[i].freeptr=(node_t*)lp;
                if(map[i].rest == 64){
                    free(map[i].head);
                    map.del(i);
                }
                return;
            }
        }
    }
}
//////////////////

#endif
