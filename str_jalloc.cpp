#ifndef ____str_JALLOC____
#define ____str_JALLOC____
#include "../structure/jtree.cpp"
#include <iostream>
namespace jean
{
   class str_jalloc
   {
   private:
      int size_list[16];
      void init(int);
      struct next_t
      {
         /* data */
         next_t *next;
      };
      int desize(size_t);
      void **ptr;

   public:
      str_jalloc(/* args */);
      ~str_jalloc();
      void *allocate(size_t);
      void deallocate(void *,size_t);
   };

   str_jalloc::str_jalloc(/* args */) : size_list()
   {
      ptr = (void **)malloc(8 * 16);
      for (int i = 1; i <= 16; i++)
      {
         size_list[i - 1] = 16 * i;
      }
   }

   str_jalloc::~str_jalloc()
   {
      free(ptr);
   }

   void str_jalloc::init(int ord){
      int size=size_list[ord];
      ptr[ord]=malloc(size*64);
      next_t*lp=(next_t*)ptr[ord];
      for(int i=1;i<64;i++){
         lp->next=(next_t*)(((char*)lp)+size);
         lp=lp->next;
      }
      lp->next=NULL;
   }

   int str_jalloc::desize(size_t size)
   {
      int k = (size + 15) / 16;
      k--;
      return k;
   }

   void* str_jalloc::allocate(size_t size){
      if(size>256){
         return malloc(size);
      }
      int q=desize(size);
      if(!ptr[q]){
         init(q);
      }
      void*res=ptr[q];
      ptr[q]=(void*)(((next_t*)(ptr[q]))->next);
      return res;
   }

   void str_jalloc::deallocate(void*buf,size_t size){
      if(size>256){
         free(buf);
         return;
      }
      int q=desize(size);
      (((next_t*)(buf))->next)=(next_t*)ptr[q];
      ptr[q]=buf;
      return;
   }

}
#endif