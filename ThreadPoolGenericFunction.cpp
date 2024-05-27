#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <string>
#include <future>
using namespace std;

static class ThreadPoolGeneric
{
public:
	ThreadPoolGeneric(uint8_t numThreads);
	~ThreadPoolGeneric();

	/*
	*	We will return future for given task.
	*/
	template <class F, class... Args>
	auto enqueuTask(F&& func, Args&&... args) -> future<decltype(func(args...))>
	{
		// This will give us the return type for different function.
		using return_type = decltype(func(args...)); 

		//Now we need to packed the task and create the future of that task.
		auto task = make_shared<packaged_task<return_type()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));

		future<return_type> fut = task->get_future();

		unique_lock<std::mutex> lck(m_mtx);
		m_taskQueue.emplace([task]() {
			(*task)();
		});
		lck.unlock();
		cv.notify_one();

		return fut;
	}

	void workerThread()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			cv.wait(lck, [&]() {  return bStop || !m_taskQueue.empty(); });

			if (bStop)
				return;

			if (m_taskQueue.empty())
				continue;

			auto task = std::move(m_taskQueue.front());
			m_taskQueue.pop();
			lck.unlock();
			task();

		}
	}

private:
	uint8_t m_numThread;
	vector<thread> m_threadPool;
	queue<function<void()>> m_taskQueue;
	mutex m_mtx;
	condition_variable cv;
	bool bStop = false;
};

ThreadPoolGeneric::ThreadPoolGeneric(uint8_t numThreads) : m_numThread(numThreads)
{
	bStop = false;
	for (int idx = 0; idx < m_numThread; ++idx)
	{
		m_threadPool.emplace_back(thread(&ThreadPoolGeneric::workerThread, this));
	}
}

ThreadPoolGeneric::~ThreadPoolGeneric()
{
	bStop = true;
	cv.notify_all();
	for (int idx = 0; idx < m_numThread; ++idx)
		m_threadPool.at(idx).join();

	std::unique_lock<std::mutex> lck(m_mtx);
	while (!m_taskQueue.empty())
		m_taskQueue.pop();
}

static int multiplyFunction(int a, int b)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return a * b;
}

int main()
{
	ThreadPoolGeneric threadPool(3);

	auto fut = threadPool.enqueuTask(multiplyFunction, 2, 3);

	cout << "Output of multiplyFunction -> 2*3 = " << fut.get() << "\n";
	std::this_thread::sleep_for(chrono::seconds(2));
	cout << "Done task for main thread\n";
	return 0;
}