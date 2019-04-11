// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/Navigation/NavigationSystem.h"
#include "BTTask_Loadbalancing.h"
#include "SoftDesignTrainingGameMode.h"

EBTNodeResult::Type UBTTask_Loadbalancing::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		uint64 frame = aiController->GetLastUpdateFrame();
		LoadBalancer* loadBalancer = ((ASoftDesignTrainingGameMode*)GetWorld()->GetAuthGameMode())->loadBalancer;
		bool canExecute = loadBalancer->canExecute(frame);

		if (canExecute) {
			return EBTNodeResult::Succeeded;
		}

	}
	return EBTNodeResult::Failed;
}