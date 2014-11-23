/*
 * HpLRU
 *
 * Copyright (C) <year>  <name of author>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ata E Husain Bohra(ata.husain@hotmail.com)
 */

/*
 * ThreadPool.cpp
 *
 * Implementation is taken from https://github.com/progschj/ThreadPool
 */

#include "ThreadPool.h"
#include "Locks.h"

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <stdexcept>

// forward declaration
class ThreadPool;
using ThreadPoolPtr = std::shared_ptr<ThreadPool>;

class ThreadPool
{
public:
    static ThreadPoolPtr allocate(int numThreads)
    {
        ThreadPoolPtr ptr(new ThreadPool(numThreads));
        return ptr;
    }
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    ThreadPool(size_t);
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers;
    // the task queue
    std::queue<Task> tasks;

    // synchronization
    lock::Mutex mutex_;
    std::condition_variable condition;
    bool stop;
}; //ThreadPool

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    Task t;
                    {
                        std::unique_lock<std::mutex> lock(this->mutex_);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        t = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    t();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        lock::GuardLock lock(mutex_);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace(
          [task]()
          {
            (*task)();
          });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        lock::GuardLock lock(mutex_);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

namespace
{
ThreadPoolPtr _instance;
} // unnamed

namespace threadpool
{
void allocate(int numThreads)
{
    assert(!_instance);
    _instance = ThreadPool::allocate(numThreads);
}
void teardown()
{
    assert(_instance);
    _instance.reset();
}
void enqueue(const Task &t)
{
    assert(_instance);
    _instance->enqueue(t);
}

} //threadpool
