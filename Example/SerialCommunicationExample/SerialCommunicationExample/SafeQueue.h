#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class Safedeque : public std::deque<T>
{
private:
    mutable std::mutex m;
    std::condition_variable c;
    bool mIsLoop = true;
    size_t limit = 4096;

public:
    void setLimit(size_t _limit)
    {
        limit = _limit;
    }
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m);
        push_back(t);

        /*for circular*/
        if ((this->size() > limit)
            && (limit > 0))
        {
           // this->front();
            this->pop_front();
        }

        c.notify_one();

    }

    void enqueue(T* t, size_t n)
    {
        std::lock_guard<std::mutex> lock(m);
        
        for (size_t i = 0; i < n; i++)
        {
            push_back(t[i]);
        }

        /*for circular*/
        if ((this->size() > limit)
            && (limit > 0))
        {
             //this->front();
             this->pop_front();
        }

        c.notify_one();

    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        while ((mIsLoop) && (this->empty()))
        {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }
        T val = this->front();
        this->pop_front();

        return val;
    }

    void destory() {
        mIsLoop = false;
        c.notify_one();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m);
        
        this->clear();
        //std::queue<T> emptyQueue;
        //std::swap(q, emptyQueue);
    }

    bool isAvailable()
    {
        return mIsLoop;
    }
};

template<class T>
class SafeQueue
{

private:
    //std::queue<T> q;
    std::queue<T> q;
    std::mutex m;
    std::condition_variable c;
    bool mIsLoop = true;
	bool bEscape;
    size_t limit = 50000;

public:
	SafeQueue()
		: q()
		, m()
		, c()
		, mIsLoop(true)
		, bEscape(false)
    {}
    ~SafeQueue(void) {};

    SafeQueue(SafeQueue<T>&& other)
        :q(std::move(other.q))
	{
	}

    auto contatainer() { return q; }
    /*Default : 4096*/
    void setLimit(size_t _limit)
    {
        limit = _limit;
    }

	void enqueue(T&& t)
	{

		std::unique_lock<std::mutex> lock(m);
		q.push(std::move(t));

		/*for circular*/
		if ((q.size() > limit)
			&& (limit > 0))
		{
			//q.front();
			q.pop();
		}

		c.notify_one();
	}


	void enqueue(const T& t)
	{

		std::unique_lock<std::mutex> lock(m);
		q.push(t);

		/*for circular*/
		if ((q.size() > limit)
			&& (limit > 0))
		{
			//q.front();
			q.pop();
		}

		c.notify_one();
	}



    void enqueue(T* t,size_t n)
    {
        std::unique_lock<std::mutex> lock(m);

        for (size_t i = 0; i < n; i++)
        {
            q.push(t[i]);
        }

        /*for circular*/
        if ((q.size() > limit)
            && (limit > 0))
        {
            //q.front();
            q.pop();
        }

        c.notify_one();

    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
		T val = T();
        std::unique_lock<std::mutex> lock(m);
        while ((mIsLoop) && (q.empty()) && 
			bEscape == false)
        {
            // release lock as long as the wait and reaquire it afterwards.
			// c.wait_for(lock,std::chrono::milliseconds(1));
			c.wait(lock);
        }
		if (bEscape == false && mIsLoop == true)
		{
			val = q.front();
			q.pop();
		}
		else
		{
			bEscape = false;
		}

        return val;
    }

    T dequeue_timeout(const std::chrono::milliseconds time, std::cv_status& status)
    {
        T val = T();
        std::unique_lock<std::mutex> lock(m);
        while ((mIsLoop) && (q.empty()) &&
            bEscape == false)
        {
            // release lock as long as the wait and reaquire it afterwards.
            // c.wait_for(lock,std::chrono::milliseconds(1));
            status = c.wait_for(lock, time);

            if(status == std::cv_status::timeout)
            {
                return T();
            }
        }
        if (bEscape == false && mIsLoop == true)
        {
            val = q.front();
            q.pop();
        }
        else
        {
            bEscape = false;
        }

        return val;
    }

    size_t size()
    {
        return q.size();
    }

	void swap(std::queue<T> Queue) {
		q.swap(Queue);

	}

    void destory() {
        mIsLoop = false;
        c.notify_one();
    }

    void clear() {
        std::unique_lock<std::mutex> lock(m);
        std::queue<T> emptyQueue;
        std::swap(q, emptyQueue);
    }

	/*Queue를 파괴하지 않고 루프를 탈출 할 때 사용한다.*/
	void escape() {
		bEscape = true;
		c.notify_one();
	}

    bool isAvailable()
    {
        return mIsLoop;
    }
};

