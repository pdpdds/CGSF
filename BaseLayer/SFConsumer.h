#pragma once

typedef ACE_Atomic_Op<ACE_Thread_Mutex, UINT> SafeInt;

<template T>
class SFConsumer: public ACE_Task_Base
{
	

public:
	SFConsumer(void){}
	virtual ~SFConsumer(void){}

protected:

private:
};
