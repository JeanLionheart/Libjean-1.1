#ifndef __JECT__
#define __JECT__
#include <stdlib.h>
#include <malloc.h>
#include <iostream>
using std::cout;
using std::endl;
namespace jean
{
    template <typename T>
    class jector // a vector who can free automaticly
    {
    private:
        /* data */
        void operator=(jector<T> &);
        T *jec;
        int __size;
        int __capacity;

        void refill();

    public:
        jector(jector<T> &);
        jector(jector<T> &&);
        jector(/* args */);
        ~jector();
        T &operator[](const int n)
        {
            return *(jec + n);
        }
        void push_back(T &);
        void push_back(T &&);
        T pop_back();
        void del(int);
        void insert(int);
        void release();
        int size();
        int capacity();
        bool empty();
    };

    template <typename T>
    jector<T>::jector(/* args */) : jec((T *)malloc(sizeof(T) * 8)) /*(T *)malloc(sizeof(T) * 8)*/ /*new T[8]*/, __size(0), __capacity(8)
    {
    }

    template <typename T> // the higher copy constructor will call its push_back,and push_back will call lower constructor.untill the lowest constructor,such as int
    jector<T>::jector(jector<T> &cop) : jec((T *)malloc(sizeof(T) * 8)) /*new T[8]*/, __size(0), __capacity(8)
    { // copy constructor,deep
        for (int i = 0; i < cop.size(); i++)
        {
            push_back(cop[i]);
        }
    }

    template <typename T> // the higher copy constructor will call its push_back,and push_back will call lower constructor.untill the lowest constructor,such as int
    jector<T>::jector(jector<T> &&cop) : jec((T *)malloc(sizeof(T) * 8)) /*new T[8]*/, __size(0), __capacity(8)
    { // copy constructor,deep
        for (int i = 0; i < cop.size(); i++)
        {
            push_back(cop[i]);
        }
    }

    template <typename T>
    jector<T>::~jector()
    {
        /* delete[] jec; */
        if(jec)free(jec);
    }

    template <typename T>
    void jector<T>::refill()
    {
        __capacity *= 2;
        T *new_jec = (T *)malloc(sizeof(T) * __capacity);
        int len = sizeof(T) * __capacity / 2;
        long *oldl = (long *)jec;
        long *newl = (long *)new_jec;
        int i;
        for (i = 8; i <= len; i += 8)
        {
            *newl = *oldl;
            newl++;
            oldl++;
        }
        if (i != len)
        {
            i -= 8;
            if (len > 8)
            {
                newl--;
                oldl--;
            }
            char *nc = (char *)newl;
            char *oc = (char *)oldl;
            for (int j = i; j <= len; j++)
            {
                *nc = *oc;
                nc++;
                oc++;
            }
        }
        free(jec);
        jec = new_jec;
    }

    template <typename T>
    void jector<T>::push_back(T &v)
    {
        __size++;
        if (__size > __capacity)
        {
            refill();
        }
        int len = sizeof(T);
        new (jec + __size - 1) T(v);
    }

    template <typename T>
    void jector<T>::push_back(T &&v)
    {
        __size++;
        if (__size > __capacity)
        {
            refill();
        }
        int len = sizeof(T);
        new (jec + __size - 1) T(std::move(v));
    }

    template <typename T>
    T jector<T>::pop_back()
    {
        T r((*this)[__size - 1]);
        __size--;
        return r;
    }

    template <typename T>
    void jector<T>::del(int seq)
    {
        for (int i = seq; i < __size; i++)
        {
            (*this)[i] = (*this)[i + 1];
        }
        __size--;
    }

    template <typename T>
    void jector<T>::insert(int seq)
    {
        __size++;
        if (__size > __capacity)
        {
            refill();
        }
        for (int i = __size-1; i>seq ; i--)
        {
            (*this)[i] = (*this)[i - 1];
        }
    }

    template <typename T>
    void jector<T>::release()
    {
        if (__size == 0)
        {
            free(jec);
            jec=NULL;
            return;
        }
        __capacity = (__size + 1) / 2;
        refill();
    }

    template <typename T>
    int jector<T>::size(){
        return __size;
    }

    template <typename T>
    int jector<T>::capacity(){
        return __capacity;
    }
}
#endif
// note: push_back need a r_value reference
