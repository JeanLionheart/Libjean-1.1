#ifndef ____str_JALLOC____
#define ____str_JALLOC____
#include "../structure/jector.cpp"
#include <iostream>
#include <string.h>
using std::cout;
using std::endl;
namespace jean
{
   class str_jalloc
   {
   protected:
      unsigned short size_list[19];
      void init(int);
      struct next_t
      {
         /* data */
         next_t *next;
      };
      int desize(size_t);
      void **ptr;

   public:
      str_jalloc();
      ~str_jalloc();
      void *allocate(size_t);
      void deallocate(void *, size_t);
      void release();
   };

   str_jalloc::str_jalloc() 
   {
      ptr = (void **)malloc(8 * 19);
      for (int i = 1; i <= 16; i++)
      {
         size_list[i - 1] = 16 * i;
         ptr[i - 1] = nullptr;
      }
      for (int i = 16; i <= 18; i++)
      {
         ptr[i] = nullptr;
      }
      size_list[16] = 512;
      size_list[17] = 768;
      size_list[18] = 1024;
   }

   str_jalloc::~str_jalloc()
   {
      free(ptr);
   }

   void str_jalloc::init(int n)
   {
      int size = size_list[n];
      ptr[n] = malloc((size + 2) * 64);

      next_t *lp = (next_t *)((char *)ptr[n] + 2);
      ptr[n] = (void *)lp;
      char *r = ((char *)lp - 2);
      *r = n;
      r += 1;
      *r = 0;
      for (int i = 1; i < 64; i++)
      {
         lp->next = (next_t *)(((char *)lp) + size + 2);
         lp = lp->next;
         r = ((char *)lp - 2);
         *r = n;
         r += 1;
         *r = 0;
      }
      lp->next = NULL;
   }

   int str_jalloc::desize(size_t size)
   {
      int k = (size + 15) / 16;
      k--;
      if (size > 256)
      {
         if (size <= 512)
         {
            k = 16;
         }
         else if (size <= 768)
         {
            k = 17;
         }
         else if (size <= 1024)
         {
            k = 18;
         }
      }
      return k;
   }

   void *str_jalloc::allocate(size_t size)
   {
      if(size == 0){
         return nullptr;
      }
      if (size > 1024)
      {
         return malloc(size);
      }
      int q = desize(size);
      int k = q;
      while (q < 18 && !ptr[q])
         q++;
      if (!ptr[q])
      {
         q = k;
         init(q);
      }
      void *res = ptr[q];
      char *r = (char *)res - 1;
      *r = 1;
      ptr[q] = (void *)(((next_t *)(ptr[q]))->next);
      return res;
   }

   void str_jalloc::deallocate(void *buf, size_t size)
   {
      if(!buf)return;
      if (size > 1024)
      {
         free(buf);
         return;
      }
      char *r = ((char *)buf - 2);
      int q = *r;
      r += 1;
      *r = 0;
      (((next_t *)(buf))->next) = (next_t *)ptr[q];
      ptr[q] = buf;
      return;
   }
/* 















 */
   /* class jalloc:private str_jalloc
   {
   private:
   public:
      jalloc();
      ~jalloc();
   };
   
   jalloc::jalloc()
   {
   }
   
   jalloc::~jalloc()
   {
   } */
   

}
#endif