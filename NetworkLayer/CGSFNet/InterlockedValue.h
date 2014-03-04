#pragma once

class InterlockedValue
{
public:
	InterlockedValue();
	InterlockedValue(long initialValue);
	~InterlockedValue() {};
	bool	Acquire();
	void	Release();

public:
	bool	CompareExchange(long exchange, long comparand);
	void	Exchange(long exchange);
	bool	Compare(long comparand);

private:
	long	_value;
};