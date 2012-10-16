#pragma once
#include <ace/Containers.h>
#include <Assert.h>

namespace CGSF
{
	template<class T>
	class Queue
	{
	public:
		void Push(T t)
		{
			m_Queue.enqueue_tail(t);
		}

		/////////////////////////////////////////
		// Description:
		//   when the queue is empty, return false
		/////////////////////////////////////////
		bool Pop(T& t)
		{
			return (-1 != m_Queue.dequeue_head(t));
		}

		bool Head(T*& t)
		{
			return (-1 != m_Queue.get(t));
		}

		int Size()
		{
			return m_Queue.size();
		}

	protected:
		ACE_Unbounded_Queue<T>	m_Queue;
	};

#define INVALID_ID  0xffffffff
	template<int MaxIDCount>
	class IDQueue
	{
	public:
		IDQueue(int offset) : m_offset(offset)
		{
			for (int i = 0; i < MaxIDCount; ++i)
				m_idleIdQueue.Push(i + m_offset);
		}

		inline int Pop()
		{
			int id = INVALID_ID;
			if (false == m_idleIdQueue.Pop(id))
			{
				assert(0);
			}

			return id;
		}


		inline void Push(int id)
		{
			if (IsValidId(id))
			{
				m_idleIdQueue.Push(id);
			}
			else
			{
				assert(0);
			}
		}

		inline bool IsValidId(int id) const
		{
			return (INVALID_ID != id) && (id >= m_offset) && (id < (MaxIDCount + m_offset));
		}

	private:
		int				m_offset;
		Queue<int>	m_idleIdQueue;
	};
};
