
#include "ThreadController.h"
ThreadController::ThreadController()
	:interval_(1000)
{
	readyStatus = true;
}

ThreadController::~ThreadController()
{
	stop();
}

void ThreadController::start(std::function<void()> worker)
{
	if (threadWorker_.get_id() == std::thread::id())
	{
		destroyQueue.clear();
		readyStatus.store(true);
		worker_ = worker;
		threadWorker_ = std::thread(std::bind(&ThreadController::monitor, this));
	}
	else
	{
		/*restart*/
		readyStatus.store(true);
	}
}

void ThreadController::stop()
{
	worker_ = std::function<void()>();
	destroyQueue.enqueue(true);
	readyStatus.store(false);
	readyCondition.notify_one();

	if (threadWorker_.joinable())
	{
		threadWorker_.join();
	}
}

void ThreadController::end()
{
	worker_ = std::function<void()>();
	destroyQueue.enqueue(true);
	readyStatus.store(false, std::memory_order_release);
	readyCondition.notify_one();

}

void ThreadController::pause()
{
	/*대기상태로 진입한다.*/
	readyStatus.store(false);
}

void ThreadController::resume()
{
	setReady();
}

bool ThreadController::isPaused()
{
	return (readyStatus == false) && (destroyQueue.size() == 0);
}

bool ThreadController::isWorking()
{
	return destroyQueue.size() != 0;
}

void ThreadController::setInterval(int milliseconds)
{
	interval_ = milliseconds;
}

int ThreadController::getInterval()
{
	return interval_;
}

void ThreadController::setThreadPriority(unsigned long priority)
{
	SetThreadPriority(threadWorker_.native_handle(), priority);
}

void ThreadController::setReady()
{
	std::unique_lock<std::mutex> lock{ readyMutex };
	readyStatus.store(true);
	readyCondition.notify_one();
}

bool ThreadController::isReady()
{
	bool ready = readyStatus.load(std::memory_order_acquire);

	/*소멸자 호출 시 false*/
	if (destroyQueue.size() > 0)
	{
		return false;
	}
	else if (ready == false)
	{
		/*ready false일 경우 wait상태로 들어간다.*/
		waitReady();
		return false;
	}

	return true;
}

void ThreadController::waitReady()
{
	std::unique_lock<std::mutex> lock{ readyMutex };
	/*readyCondition.wait(lock,  [&] {
		return readyStatus.load(std::memory_order_acquire);
		});*/

	readyCondition.wait_for(lock, std::chrono::milliseconds(interval_), [&] {
		return readyStatus.load(std::memory_order_acquire);
		});

}

void ThreadController::monitor()
{
	//unsigned int count = 1;
	//auto start = std::chrono::system_clock::now();
	while (destroyQueue.size() == 0)
	{
		//count++;
		if (isReady() == false)
		{
			continue;
		}


		auto start = std::chrono::system_clock::now();
		/*to do*/
		worker_();

		std::this_thread::sleep_until(start + std::chrono::milliseconds(interval_) );
	}
}
