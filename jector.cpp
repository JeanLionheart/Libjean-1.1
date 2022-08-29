#ifndef __JECT__
#define __JECT__
#include <stdlib.h>
#include <malloc.h>
#include <iostream>
namespace jean
{
    template <typename T>
    class jector // a vector who can free automaticly
    {
    private:
        /* data */
        void operator=(jector<T> &);
        T *jec;
        unsigned long __size;
        unsigned long __capacity;
        void copy(T *&oldlp, T *&newlp, long l, long new_l, long nums, long step = 1, bool dir = 0) // 0 left; 1 right
        {                                                                                       // copy
            long cnt = nums;
            long i, j;
            if (!dir)
            {
                i = l;
                j = new_l;
            }
            else
            {
                i = l + nums - 1;
                j = new_l + nums - 1;
            }
            for (; cnt > 0; i += step, j += step, cnt--)
            {
                new (&newlp[j]) T(oldlp[i]);
            }
        }
        void move_left(long start)
        { // from start(not include start) elements all move to left
            copy(jec, jec, start + 1, start, __size - start - 1, 1, 0);
        }
        void move_right(long start)
        { // from start(include start) elements all move to right
            copy(jec, jec, start, start + 1, __size - start - 1, -1, 1);
        }
        void test_and_realloc()
        {
            if (__size >= __capacity)
            {
                T *old_jec = jec;
                jec = (T *)malloc(__capacity * 2 * sizeof(T));
                /* jec = new T[__capacity * 2]; */
                __capacity *= 2;
                copy(old_jec, jec, 0, 0, __size-1);
                /* delete[] old_jec; */
                free(old_jec);
            }
        }
        void test_and_free()
        {
            if (__size < __capacity / 3)
            {
                T *old_jec = jec;
                jec = (T *)malloc((__capacity / 2>1?__capacity / 2:1) * sizeof(T));
                /* jec = new T[__capacity / 2]; */
                __capacity=(__capacity / 2>1?__capacity / 2:1);
                copy(old_jec, jec, 0, 0, __size);
                /* delete[] old_jec; */
                free(old_jec);
            }
        }

    public:
        jector(jector<T> &);
        jector(jector<T> &&);
        jector(/* args */);
        ~jector();
        T &operator[](const long n)
        {
            return *(jec + n);
        }
        long find(T&val){
            for(long i=0;i<__size;i++){
                if((*this)[i]==val){
                    return i;
                }
            }
            return -1;
        }
        // jector<T> &operator=(jector<T> &cop)
        // {
        //     init();
        //     for (long i = 0; i < cop.size(); i++)
        //     {
        //         (*this)[i]=cop[i]
        //     }
        //     return *this;
        // }
        void push_back(T &val) // copy construct
        {
            long loc=__size;
            __size++;
            test_and_realloc();
            new (jec + loc) T(val);
        }
        void push_back(T &&val) // copy construct
        {
            long loc=__size;
            __size++;
            test_and_realloc();
            new (jec + loc) T(std::move(val));
        }
        T pop_back()
        {
            test_and_free();
            __size--;
            return *(jec + __size);
        }
        void insert_on(long addr, const T &val) // insert val to addr,
        {
            if (addr > __size)
                return;
            __size++;
            test_and_realloc();
            move_right(addr);
            jec[addr] = val;
        }
        void del(long addr) // del the jec[addr]
        {
            if (addr >= capacity())
                return;
            move_left(addr);
            __size--;
            test_and_free();
        }
        unsigned long size()
        {
            return __size;
        }
        long capacity()
        {
            return __capacity;
        }
        bool is_empty()
        {
            return (__size == 0);
        }
    };

    template <typename T>
    jector<T>::jector(/* args */) : jec((T *)malloc(sizeof(T) * 8)) /*(T *)malloc(sizeof(T) * 8)*/ /*new T[8]*/, __size(0), __capacity(8)
    {
    }

    template <typename T> // the higher copy constructor will call its push_back,and push_back will call lower constructor.untill the lowest constructor,such as long
    jector<T>::jector(jector<T> &cop) : jec((T *)malloc(sizeof(T) * 8)) /*new T[8]*/, __size(0), __capacity(8)
    { // copy constructor,deep
        for (long i = 0; i < cop.size(); i++)
        {
            push_back(cop[i]);
        }
    }

    template <typename T> // the higher copy constructor will call its push_back,and push_back will call lower constructor.untill the lowest constructor,such as long
    jector<T>::jector(jector<T> &&cop) : jec((T *)malloc(sizeof(T) * 8)) /*new T[8]*/, __size(0), __capacity(8)
    { // copy constructor,deep
        for (long i = 0; i < cop.size(); i++)
        {
            push_back(cop[i]);
        }
    }

    template <typename T>
    jector<T>::~jector()
    {
        /* delete[] jec; */
        free(jec);
    }
}
#endif
// note: push_back need a r_value reference