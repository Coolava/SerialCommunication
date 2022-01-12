#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <Windows.h>
#include "SafeQueue.h"
class ThreadController
{
public:
	ThreadController();
	~ThreadController();
	void start(std::function<void()> worker);
	void stop();
	void end();

	/*Pause thread*/
	void pause();
	void resume();
	bool isPaused();
	bool isWorking();

	void setInterval(int milliseconds);
	int getInterval();

	void setThreadPriority(unsigned long priority);
private:
	using condition = std::condition_variable;

	/*Thread handle*/
	std::thread threadWorker_;
	//std::atomic<bool> destroyFlag;
	SafeQueue<bool> destroyQueue;


	std::mutex readyMutex;
	condition readyCondition;
	std::atomic<bool> readyStatus;

	/*Call worker in monitor thread*/
	std::function<void()> worker_;

	void setReady();
	bool isReady();
	void waitReady();

	void monitor();

	/*interval milliseconds*/
	int interval_;

	inline void busySleep(std::chrono::system_clock::time_point time)
	{
		SwitchToThread();
		while (1)
		{
			//counter++;
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (std::chrono::system_clock::now() >= time)
			{
				break;
			}
		}
	}
};

