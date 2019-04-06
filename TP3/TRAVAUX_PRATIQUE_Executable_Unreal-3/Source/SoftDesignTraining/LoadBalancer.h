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
	bool IsValidId(int);
	bool CanExecute(double, int&);
private:
	const double ALLOWED_TIME = 1.0 / 30.0; // 30 fps in seconds
	int count = 0;
	int order = 1;
	uint64 lastFrame;
	int lastId = 0;
	double timeSpent = 0;
};

