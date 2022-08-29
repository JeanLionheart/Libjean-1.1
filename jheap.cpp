#ifndef __JhEAP__
#define __JhEAP__
#include "jector.cpp"
#include "key_traits.cpp"
#include <iostream>
using namespace std;
namespace jean
{
    template <typename T, class traits_t>
    class jheap
    {
    private:
        /* data */
        struct obj
        {
            /* data */
            T val;
            long key;
            obj(long k, T v) : val(v), key(k) {}
        };
        jector<obj> jecA, jecV;
        traits_t tFunc;
        void float_up_A();
        void float_up_V();
        void sink_down_A();
        void sink_down_V();
        void swap(obj &a, obj &b)
        {
            obj t = a;
            a = b;
            b = t;
        }

    public:
        void push_A(T);
        void push_V(T);
        T *pop_A();
        T *pop_V();
        T*topA();
        T*topV();
        int sizeA(){
            return jecA.size();
        }
        int sizeV(){
            return jecV.size();
        }
        jheap(/* args */);
        ~jheap();
    };
    template <typename T, class traits_t>
    void jheap<T, traits_t>::float_up_A()
    {
        for (int i = (jecA.size() - 2) / 2, j = jecA.size() - 1; jecA[i].key > jecA[j].key && i >= 0; j = i, i = (j - 1) / 2)
        {
            swap(jecA[i], jecA[j]);
        }
    }

    template <typename T, class traits_t>
    void jheap<T, traits_t>::float_up_V()
    {
        for (int i = (jecV.size() - 2) / 2, j = jecV.size() - 1; jecV[i].key < jecV[j].key && i >= 0; j = i, i = (j - 1) / 2)
        {
            swap(jecV[i], jecV[j]);
        }
    }

    template <typename T, class traits_t>
    void jheap<T, traits_t>::sink_down_A()
    {
        for (int i = 0, j = 1; (i <= (jecA.size() - 2) / 2) && (j < jecA.size()); i = j, j = (j * 2) + 1)
        {
            if (jecA.size() > j + 1 && jecA[j].key > jecA[j + 1].key)
                j++;
            if (jecA[i].key > jecA[j].key)
            {
                swap(jecA[i], jecA[j]);
            }
            else
            {
                return;
            }
        }
    }

    template <typename T, class traits_t>
    void jheap<T, traits_t>::sink_down_V()
    {
        for (int i = 0, j = 1; (i <= (jecV.size() - 2) / 2) && (j < jecV.size()); i = j, j = (j * 2) + 1)
        {
            if (jecV.size() > j + 1 && jecV[j].key < jecV[j + 1].key)
                j++;
            if (jecV[i].key < jecV[j].key)
            {
                swap(jecV[i], jecV[j]);
            }
            else
            {
                return;
            }
        }
    }

    template <typename T, class traits_t>
    void jheap<T, traits_t>::push_A(T v)
    {
        obj q(tFunc(v), v);
        jecA.push_back(q);
        float_up_A();
    }

    template <typename T, class traits_t>
    void jheap<T, traits_t>::push_V(T v)
    {
        obj q(tFunc(v), v);
        jecV.push_back(q);
        float_up_V();
    }

    template <typename T, class traits_t>
    T *jheap<T, traits_t>::pop_A()
    {
        if (jecA.is_empty())
        {
            return 0;
        }
        swap(jecA[0], jecA[jecA.size() - 1]);
        obj res = jecV.pop_back();
        T*r=new T(res.val);
        sink_down_A();
        return r;
    }

    template <typename T, class traits_t>
    T *jheap<T, traits_t>::pop_V()
    {
        if (jecV.is_empty())
        {
            return 0;
        }
        swap(jecV[0], jecV[jecV.size() - 1]);
        obj res = jecV.pop_back();
        T*r=new T(res.val);
        sink_down_V();
        return r;
    }

    template <typename T, class traits_t>
    T *jheap<T, traits_t>::topA(){
        if(jecA.size()== 0){
            return nullptr;
        }
        return &(jecA[0]);
    }

    template <typename T, class traits_t>
    T *jheap<T, traits_t>::topV(){
        if(jecV.size()== 0){
            return nullptr;
        }
        return &(jecV[0]);
    }

    template <typename T, class traits_t>
    jheap<T, traits_t>::jheap(/* args */)
    {
    }

    template <typename T, class traits_t>
    jheap<T, traits_t>::~jheap()
    {
    }

} // namespace jean

#endif