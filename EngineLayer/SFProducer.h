#pragma once

typedef ACE_Atomic_Op<ACE_Thread_Mutex, UINT> SafeInt;
enum
	{
		MAX_ITEM_COUNT = 1000,
	};

<template T>
class SFProducer
{
public:
	SFProducer(void){}
	virtual ~SFProducer(void){}
};
