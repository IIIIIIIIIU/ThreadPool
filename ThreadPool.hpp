//
// Created by mayin on 2022/3/16.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <condition_variable>

namespace tp{

    class ThreadPool{
        using Function=std::function<void()>;

        int count;

        std::atomic<bool> ret;

        std::mutex taskMutex;
        std::queue<Function> tasks;
        std::vector<std::thread> workers;
        std::condition_variable condition;

    public:

        template<class Func,class... FuncArgs>
        void enqueue(Func&& func,FuncArgs&&... funcArgs);

        explicit ThreadPool(size_t poolSize);
        ~ThreadPool();
    };

    ThreadPool::ThreadPool(size_t poolSize):
            count(0),
            ret(false){
        for(size_t i=0;i<poolSize;i++){
            workers.emplace_back(
                    [this]{
                        for(;;){
                            Function task;
                            {
                                std::unique_lock<std::mutex> lock(this->taskMutex);

                                if(this->ret.load()){
                                    return;
                                }

                                if(this->count<=0){
                                    --(this->count);
                                    this->condition.wait(lock);
                                }
                                else{
                                    --(this->count);
                                }

                                task=std::move(this->tasks.front());
                                this->tasks.pop();

                                lock.unlock();
                            }
                            task();
                        }
                    }
            );
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock;
            ret.store(true);
            lock.unlock();

            condition.notify_all();

            for(auto &worker:workers){
                worker.join();
            }
        }
    }

    template<class Func,class... FuncArgs>
    void ThreadPool::enqueue(Func&& func,FuncArgs&&... funcArgs){

        auto task=std::make_shared<std::packaged_task<void()> >(
                std::bind(std::forward<Func>(func),std::forward<FuncArgs>(funcArgs)...)
        );

        {
            std::unique_lock<std::mutex> lock(taskMutex);

            tasks.emplace([task](){
                (*task)();
            });

            if(count<0){
                count++;
                condition.notify_one();
            }
            else{
                ++count;
            }

            lock.unlock();
        }
    }
}

#endif
