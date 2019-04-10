#include "SoftDesignTraining.h"
#include "LoadBalancer.h"

LoadBalancer::LoadBalancer()
{
	executionsPerFrame = ALLOWED_TIME / AVERAGE_EXECUTE_TIME;
}

void LoadBalancer::increaseCount()
{
	count++;

	executeEvery = (int) ceil(count / executionsPerFrame);

	UE_LOG(LogTemp, Display, TEXT("execute every: %i"), executeEvery)
}

bool LoadBalancer::canExecute(uint64& lastFrameExecuted)
{
	uint64 diff = GFrameCounter - lastFrameExecuted;

	UE_LOG(LogTemp, Display, TEXT("diff: %i"), diff)

	bool res = diff >= executeEvery;

	if (res) lastFrameExecuted = GFrameCounter;
	else UE_LOG(LogTemp, Display, TEXT("cannot execute"))

	return res;
}

