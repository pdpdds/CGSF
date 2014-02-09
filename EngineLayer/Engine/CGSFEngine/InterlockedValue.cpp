#include "InterlockedValue.h"
#include <Windows.h>

InterlockedValue::InterlockedValue()
{
}

InterlockedValue::InterlockedValue( long initialValue ) : _value(initialValue)
{
}

bool InterlockedValue::Acquire()
{
	return CompareExchange(1, 0);
}

void InterlockedValue::Release()
{
	Exchange(0);
}

bool InterlockedValue::CompareExchange(long exchange, long comparand)
{
	return (comparand == InterlockedCompareExchange(&_value, exchange, comparand));
}

void InterlockedValue::Exchange(long exchange)
{
	InterlockedExchange(&_value, exchange);
}

bool InterlockedValue::Compare(long comparand)
{
	return (_value == comparand);
}