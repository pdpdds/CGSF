#pragma once

#include <functional>
#include <chrono>
#include <future>
#include <queue>
#include <thread>
#include <memory>

struct function_timer
{
	std::function<void()> func;
	std::chrono::system_clock::time_point time;

	function_timer()
	{ }

	function_timer(std::function<void()>&& f, std::chrono::system_clock::time_point& t)
		: func(f),
		time(t)
	{ }

	//Note: we want our priority_queue to be ordered in terms of
	//smallest time to largest, hence the > in operator<. This isn't good
	//practice - it should be a separate struct -  but I've done this for brevity.
	bool operator<(const function_timer& rhs) const
	{
		return time > rhs.time;
	}

	void get()
	{
		func();
	}
};

class SFScheduler
{
private:
	std::priority_queue<function_timer> m_tasks;
	std::unique_ptr<std::thread> m_pThread;
	bool m_goOn;

	SFScheduler& operator=(const SFScheduler& rhs) = delete;
	SFScheduler(const SFScheduler& rhs) = delete;

public:

	SFScheduler()		
	{ 
		m_goOn = true;
		std::unique_ptr<std::thread> pThread(new std::thread([this]() { ThreadLoop(); }));
		m_pThread = move(pThread);
	}

	~SFScheduler()
	{
		m_goOn = false;
		m_pThread->join();
	}

	void ThreadLoop()
	{
		while (m_goOn)
		{
			auto now = std::chrono::system_clock::now();
			while (!m_tasks.empty() && m_tasks.top().time <= now) {
				function_timer& f = m_tasks.top();
				f.get();
				m_tasks.pop();
			}

			if (m_tasks.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else {
				std::this_thread::sleep_for(m_tasks.top().time - std::chrono::system_clock::now());
			}
		}
	}

	void ScheduleAt(std::chrono::system_clock::time_point& time, std::function<void()>&& func)
	{
		m_tasks.push(function_timer(std::move(func), time));
	}

	void ScheduleEvery(std::chrono::system_clock::duration interval, std::function<void()> func)
	{
		std::function<void()> waitFunc = [this, interval, func]()
		{
			func();
			this->ScheduleEvery(interval, func);
		};
		ScheduleAt(std::chrono::system_clock::now() + interval, std::move(waitFunc));
	}
};

