#ifndef __JTHREAD__
#define __JTHREAD__
#include<future>
#include<functional>
#include<atomic>
#include<vector>
#include<thread>
#include"../structure/jque.cpp"
namespace jean
{
    class join_thread{
        join_thread(join_thread&){}
        join_thread(join_thread&&){}
        std::vector<std::thread> threads;
    public:
        void push(std::thread&&t){
            threads.push_back(std::move(t));
        }
        void join_all(){
            for(int i=0;i<threads.size();i++){
                threads[i].join();
            }
        }
        join_thread():threads(){}
        ~join_thread(){
            join_all();
        }
    };
    class jthread
    {
    private:
        /* data */
        std::atomic_bool done;
        jque<std::function<void()>> work_que;
        join_thread guard;

        void worker_thread(){
            while (!done)
            {
                std::function<void()>*task;
                task=work_que.pop_front();
                if(task){
                    (*task)();
                    delete task;
                }else{
                    std::this_thread::yield();
                }
            }
            
        }
    public:
        jthread(/* args */);
        ~jthread();
        void submit(std::function<void()> f){
            work_que.push_back(f);
        }
    };
    
    jthread::jthread(/* args */):done(0),work_que(),guard()
    {
        int s=std::thread::hardware_concurrency();
        for(int i=0;i<s;i++){
            std::thread t(&jthread::worker_thread,this);
            guard.push(std::move(t));
        }
    }
    
    jthread::~jthread()
    {
        done=1;
    }
    
} // namespace jean

#endif