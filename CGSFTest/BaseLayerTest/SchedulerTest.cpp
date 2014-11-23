#include "stdafx.h"
#include "SchedulerTest.h"
#include "SFScheduler.h"
#include <iostream>


void Task2()
{
	std::cout << "OK2 ! now is   " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
}
void Task3()
{
	std::cout << "--3 " << std::endl;
}

void Task1(SFScheduler & sch)
{
	auto now = std::chrono::system_clock::now();
	std::cout << "OK1 ! now is   " << now.time_since_epoch().count() << std::endl;

	sch.ScheduleAt(now + std::chrono::seconds(1), []{ Task2(); });
	sch.ScheduleAt(now + std::chrono::seconds(2), []{ Task2(); });
	sch.ScheduleAt(now + std::chrono::seconds(3), []{ Task2(); });
}

SchedulerTest::SchedulerTest()
{
}


SchedulerTest::~SchedulerTest()
{
}



bool SchedulerTest::Run()
{
	auto now = std::chrono::system_clock::now();

	SFScheduler sch;

	sch.ScheduleAt(now + std::chrono::seconds(15), [&sch]{ Task1(sch); });
	sch.ScheduleAt(now + std::chrono::seconds(20), [&sch]{ Task1(sch); });
	sch.ScheduleAt(now + std::chrono::seconds(25), [&sch]{ Task1(sch); });
	sch.ScheduleAt(now + std::chrono::seconds(2), [&sch]{ Task2();    });

	sch.ScheduleEvery(std::chrono::seconds(1), []{ Task3(); });

	getchar();

	return true;
}