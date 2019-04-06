#include "LoadBalancer.h"
#include "CoreGlobals.h"


LoadBalancer::LoadBalancer()
{
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
}

bool LoadBalancer::IsValidId(int id)
{
	return order > 0 ? id <= lastId : id >= lastId;
}

bool LoadBalancer::CanExecute(double lastTimeSpent, int& id)
{
	// Reset on each frame (tick)
	if (lastFrame != GFrameCounter)
	{
		timeSpent = 0.0;
		lastFrame = GFrameCounter;
	}

	// If the id has already been updated or the method would take too long
	if (!IsValidId(id) || timeSpent + lastTimeSpent > ALLOWED_TIME)
	{
		return false;
	}

	// The code is allowed to be executed

	id = lastId += order;

	if (lastId == count)
	{
		// Reverse order of execution
		order *= -1;
	}

	// TOOD increase timeSpent here or seperately?

	return true;
}

