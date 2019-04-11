#pragma once
#include "CoreGlobals.h"

class LoadBalancer
{
public:
	LoadBalancer();

	void increaseCount();
	bool canExecute(uint64&);

private:
	const double ALLOWED_TIME = 1.0 / 30.0 / 2.0; // 30 fps in seconds but only half the delta time is allowed
	const double AVERAGE_EXECUTE_TIME = ALLOWED_TIME / 2.0; // 0.00008; // Max time for executions
	double executionsPerFrame;
	int executeEvery;
	int count = 0;
};

