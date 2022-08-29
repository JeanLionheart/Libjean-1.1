#ifndef __JLCO__
#define __JLCO__
#include <malloc.h>
#include<mutex>
namespace jean
{
    template <typename T>
    class jalloc_origin
    {
    private:
        /* data */
    public:
        jalloc_origin(/* args */);
        ~jalloc_origin();
        void *allocate()
        {
            void*lp=malloc(sizeof(T));
            return lp;
        }
        void deallocate(void *lp)
        {
            free(lp);
        }
    };
    template <typename T>
    jalloc_origin<T>::jalloc_origin(/* args */)
    {
    }
    template <typename T>
    jalloc_origin<T>::~jalloc_origin()
    {
    }
}
#endif