#pragma once
class LoadBalancer
{
// Singleton stuff
public:
	static LoadBalancer* GetLoadBalancer();
private:
	LoadBalancer();
	static LoadBalancer* instance;
// Load balancing stuff
public:
	void IncreaseCount();
	//bool IsValidId(int);
	bool CanExecute(uint64&);
private:
	const double ALLOWED_TIME = 1.0 / 30.0; // 30 fps in seconds
	const double AVERAGE_EXECUTE_TIME = 0.00002; // TODO
	double executionsPerFrame;
	int executeEvery;
	int count = 0;
	/*int order = 1;
	uint64 lastFrame;
	int lastId = 0;
	double timeSpent = 0;*/
};

