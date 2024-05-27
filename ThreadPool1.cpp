#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <string>
using namespace std;

static class ThreadPool
{
public:
    ThreadPool(uint8_t numThread);
    ~ThreadPool();

    /*
    *   Enqueue the task.
    */
    template <class FUNC>
    void enqueueTask(FUNC&& func) // Using rvalue reference as we don't have any memory object for function.
    {
        unique_lock<std::mutex> lck(m_mtx);
        /*
           We are using perfect forwarding to push the worker function with exacly passed lvalue and rvalue references.
        */
        taskQueue.push(std::forward<FUNC>(func)); 
        lck.unlock();
        cv.notify_one(); // Notify one thread to pick the incoming task.
    }

    void workerThread(int threadNumber)
    {
        while (true)
        {
            unique_lock<std::mutex> lck(m_mtx);
            cv.wait(lck, [&] { return bStop || !taskQueue.empty(); });
            //Now lock aquired.
            
            //Stop single is set. So need to terminate the thread by returning from here.
            if (bStop)
                return;

            if (taskQueue.empty())
                continue;

            // Using move semantics to reduce unnecessary coping of function defination.
            auto task = std::move(taskQueue.front()); 
            taskQueue.pop();
            lck.unlock();
            task(threadNumber);  //Execute the task after tasking enqueue task.

        }
    }

    /*
    *   To demonstrate class member function as a worker function, I have added here.
    */
    void classWorkerFunction(int threadNumber)
    {
        cout << "Executing classWorkerFunction from thread number " << to_string(threadNumber) << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Simulating some task using thread sleep.
    }

private:
    uint8_t m_numThread;
    vector<std::thread> m_threadPool;
    queue<std::function<void(int)>> taskQueue;
    std::mutex m_mtx;
    std::condition_variable cv;
    bool bStop = false;
};

ThreadPool::ThreadPool(uint8_t numThread) : m_numThread(numThread)
{
    /*
    *  Initialize the pool of thread. So no need to create and destroy threads again and again.
    */
    bStop = false;
    for (int threadNumber = 0; threadNumber < m_numThread; ++threadNumber)
    {
        m_threadPool.emplace_back(thread(&ThreadPool::workerThread, this, threadNumber));
    }
}

ThreadPool::~ThreadPool()
{
    bStop = true;
    cv.notify_all();
    /*
    *   Join the thread as thread pool object is being destroying.
    */
    for (int i = 0; i < m_numThread; ++i)
    {
        m_threadPool.at(i).join();
    }
    unique_lock<std::mutex> lck(m_mtx);
    while (taskQueue.size())
        taskQueue.pop();
    lck.unlock();
}

/*
*   To demonstrate global function as a worker function, I have added here.
*/
void globalWorkerFunction(int threadNumber)
{
    cout << "Executing globalWorkerFunction from thread number " << to_string(threadNumber) << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Simulating some task using thread sleep.
}

/*
*   To demonstrate function poinetr as a worker function, I have added here.
*/
void funtionPointerFunction(int threadNumber)
{
    cout << "Executing funtionPointerFunction from thread number " << to_string(threadNumber) << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // Simulating some task using thread sleep.
}

int main()
{
    ThreadPool pool(3);

    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));
    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));
    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));
    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));
    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));
    pool.enqueueTask(std::bind(&ThreadPool::classWorkerFunction, &pool, std::placeholders::_1));

    pool.enqueueTask(std::bind(&globalWorkerFunction, std::placeholders::_1));
    pool.enqueueTask(std::bind(&globalWorkerFunction, std::placeholders::_1));
    pool.enqueueTask(std::bind(&globalWorkerFunction, std::placeholders::_1));
    pool.enqueueTask(std::bind(&globalWorkerFunction, std::placeholders::_1));


    void (*funcPtr)(int) = funtionPointerFunction;
    pool.enqueueTask(std::bind(funcPtr, std::placeholders::_1));
    pool.enqueueTask(std::bind(funcPtr, std::placeholders::_1));
    pool.enqueueTask(std::bind(funcPtr, std::placeholders::_1));

    std::this_thread::sleep_for(std::chrono::seconds(20));
    std::cout << "Done executing...\n";
}

