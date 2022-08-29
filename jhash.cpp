#ifndef __JHASH__
#define __JHASH__
#include "jector.cpp"
#include "key_traits.cpp"
#include <iostream>
#include <atomic>
#include <pthread.h>
#include "../jallocator/jalloc_conc.cpp"
using std::cout;
using std::endl;
namespace jean
{

    template <typename T, typename precise_key, class traits_func> //<value_t,key_t,func_traits_key_t>
    class jhash                                                    // unthread-safe hashtable.<value_t,key_t,func_traits_key_t>
    {
    private:
        void operator=(jhash<T, precise_key, traits_func> &);
        jhash(jhash<T, precise_key, traits_func> &);
        /* data */

        long __buc_nums;
        pthread_rwlock_t prw;
        jalloc_conc<T> data_jalloc;
        long get_bucket_key(const long v)
        {
            long res = v % (__buc_nums);
            if (res < 0)
                res += __buc_nums;
            return res;
        }
        long get_bucket_key(const char *v)
        {
            long res = 0;
            for (long i = 0; (v[i] != '\0') && (i < 100); i++)
            {
                res = (res + v[i]) * 5;
                res %= (__buc_nums);
            }
            return res;
        }
        long get_bucket_key(const void *lp)
        {
            char *q = nullptr;
            long k = ((char *)lp - q);
            long res = (k % __buc_nums);
            return res;
        }

        jector<jector<T>> jec;
        void rehash()
        {
            jector<T> stack;
            for (long i = 0; i < jec.size(); i++)
            {
                while (!jec[i].is_empty())
                {
                    stack.push_back(jec[i].pop_back());
                }
            }
            __buc_nums *= 2;
            while (jec.size() < __buc_nums)
            {
                jector<T> Tjec;
                jec.push_back(Tjec);
            }
            while (!stack.is_empty())
            {
                T val = stack.pop_back();
                jec[get_bucket_key(tFunc(val))].push_back(val);
            }
        }

        traits_func tFunc;

    public:
        void push(T &val)
        {
            long key = get_bucket_key(tFunc(val));
            pthread_rwlock_wrlock(&prw);
            jec[key].push_back(val);
            if (jec[key].size() >= __buc_nums / 2)
            {
                rehash();
            }
            pthread_rwlock_unlock(&prw);
        }
        void push(T &&val)
        {
            long key = get_bucket_key(tFunc(val));
            pthread_rwlock_wrlock(&prw);
            jec[key].push_back(std::move(val));
            if (jec[key].size() >= __buc_nums / 2)
            {
                rehash();
            }
            pthread_rwlock_unlock(&prw);
        }

        void del(precise_key inner_key)
        {
            long i = get_bucket_key(inner_key);
            pthread_rwlock_wrlock(&prw);
            for (long w = 0; w < jec[i].size(); w++)
            {
                if (tFunc(jec[i][w]) == inner_key)
                {
                    jec[i].del(w);
                    pthread_rwlock_unlock(&prw);
                    return;
                }
            }
            pthread_rwlock_unlock(&prw);
        }
        T *find(precise_key inner_key)
        {
            long i = get_bucket_key(inner_key);
            T *r = nullptr;
            pthread_rwlock_rdlock(&prw);
            for (long w = 0; w < jec[i].size(); w++)
            {
                if (tFunc(jec[i][w]) == inner_key)
                {
                    r = (T *)data_jalloc.allocate();
                    new (r) T(jec[i][w]);
                    break;
                }
            }
            pthread_rwlock_unlock(&prw);
            return r;
        }
        void return_data(void *lp)
        {
            data_jalloc.deallocate(lp);
        }
        jhash(/* args */);
        ~jhash();
    };

    template <typename T, typename precise_key, class traits_func>
    jhash<T, precise_key, traits_func>::jhash(/* args */) : __buc_nums(50)
    {
        for (long i = 0; i < __buc_nums; i++)
        {
            jector<T> Tjec;
            jec.push_back(Tjec);
        }
        pthread_rwlock_init(prw);
    }

    template <typename T, typename precise_key, class traits_func>
    jhash<T, precise_key, traits_func>::~jhash()
    {
    }
}
#endif