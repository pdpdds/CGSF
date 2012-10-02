#ifndef StreamData_H
#define StreamData_H

#include <windows.h>

const size_t StreamDataSize = 64000;

class StreamData
{

protected:

	char	value[StreamDataSize];

	unsigned int	capacity;

	unsigned int	sizeOfReceived;


public:

	StreamData(): capacity(StreamDataSize), sizeOfReceived(0)
	{
		reset();
	}

	virtual ~StreamData()
	{
	}

public:	

	virtual StreamData* clone()
	{
		StreamData * o = new StreamData();
		CopyMemory(o->value, this->value, StreamDataSize);
		o->sizeOfReceived = sizeOfReceived;
		o->capacity = capacity;
		return o;
	}

	char* current() 
	{ 
		return value+sizeOfReceived; 
	}

	void increaseSize(int sz) 
	{
		sizeOfReceived += sz; 
	}

	char* getStream() 
	{ 
		return value;
	}

	unsigned int getSize() 
	{ 
		return sizeOfReceived;
	}

	unsigned int getCapacity() 
	{ 
		return capacity;
	}

	unsigned int availableSize() 
	{
		return capacity-sizeOfReceived;
	}

	void reset()
	{
		sizeOfReceived = 0;
		ZeroMemory(value, sizeof(value));
	}

	void flushTo(unsigned int sz)
	{
		if (sizeOfReceived > sz)
		{
			memmove(value,value+sz,sizeOfReceived-sz);
			sizeOfReceived-=sz;
		}
		else sizeOfReceived=0;
	}

	unsigned int putData(unsigned int size, char* datas)
	{
		if(0 >= size) return 0;

		if(availableSize() < size) return 0;

		CopyMemory(current(), datas, size);
		increaseSize(size);
		return size;
	}	

};




#endif
