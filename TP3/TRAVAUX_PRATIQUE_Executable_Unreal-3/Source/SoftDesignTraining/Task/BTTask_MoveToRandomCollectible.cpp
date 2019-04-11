// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/Navigation/NavigationSystem.h"
#include "BTTask_MoveToRandomCollectible.h"
#include "SoftDesignTrainingGameMode.h"

EBTNodeResult::Type UBTTask_MoveToRandomCollectible::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		if (aiController->GetHasCollectibleLocation() && aiController->GetReachedTarget())
		{
			aiController->MoveToLocation(aiController->GetCollectibleLocation(), 0.5f, false, true, true, NULL, false);
			aiController->OnMoveToTarget();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}