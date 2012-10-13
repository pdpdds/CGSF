#ifndef Queue_H
#define Queue_H

#include <queue>
#include "Synchronized.h"

using namespace std;

template <class T>
class Queue : public Waiter
{
	queue<T>				data;
	CriticalSectionLock		entireSynch;

public:

	Queue() : Waiter(), data(), entireSynch()
	{
	}

	virtual ~Queue()
	{
		clear();
	}

	void clear()
	{
		Synchronized synch(&entireSynch);
		while(false==data.empty())
		{
			data.pop();
		}
	}

	bool empty()
	{
		Synchronized synch(&entireSynch);
		return data.empty();
	}

	void push(T node)
	{
		Synchronized synch(&entireSynch);
		data.push(node);
		if(1 == data.size()) notify();
	}

	void pop(T& output)
	{
		Synchronized synch(&entireSynch);
		if(data.empty()) return;
		output = data.front();
		data.pop();
	}

	// outs must valid and classic-array or 0-base operator[] overloading
	int pops(T* outs, int max)
	{
		if(0 >= max || 0 == outs) return 0;
		Synchronized synch(&entireSynch);
		int count = 0;

		for(count = 0 ; count < max ; count++)
		{
			if(data.empty()) return count;
			outs[count] = data.front();
			data.pop();
		}
		return count;
	}

	int size()
	{
		return data.size();
	}
	
};

template <class T>
class QueueNoLock : public Waiter
{
	queue<T>				data;
	

public:

	QueueNoLock() : Waiter(), data()
	{
	}

	virtual ~QueueNoLock()
	{
		clear();
	}

	void clear()
	{
		while(false==data.empty())
		{
			data.pop();
		}
	}

	bool empty()
	{
		return data.empty();
	}

	void push(T node)
	{
		data.push(node);
		if(1 == data.size()) notify();
	}

	void pop(T& output)
	{
		if(data.empty()) return;
		output = data.front();
		data.pop();
	}
	
};

#endif