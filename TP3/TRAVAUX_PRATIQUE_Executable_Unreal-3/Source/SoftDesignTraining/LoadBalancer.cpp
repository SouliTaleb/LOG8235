#include "LoadBalancer.h"
#include "CoreGlobals.h"


LoadBalancer::LoadBalancer()
{
	executionsPerFrame = ALLOWED_TIME / AVERAGE_EXECUTE_TIME;
}

LoadBalancer* LoadBalancer::GetLoadBalancer()
{
	if (instance == nullptr)
	{
		instance = new LoadBalancer();
	}

	return instance;
}

void LoadBalancer::IncreaseCount()
{
	count++;

	executeEvery = (int) ceil(count / executionsPerFrame);
}

//bool LoadBalancer::IsValidId(int id)
//{
//	return order > 0 ? id <= lastId : id >= lastId;
//}

bool LoadBalancer::CanExecute(uint64& lastFrameExecuted)
{
	return GFrameCounter - lastFrameExecuted >= executeEvery;
}

