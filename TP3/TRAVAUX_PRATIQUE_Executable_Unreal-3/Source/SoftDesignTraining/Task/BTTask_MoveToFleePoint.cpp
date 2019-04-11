// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/Navigation/NavigationSystem.h"
#include "BTTask_MoveToFleePoint.h"
#include "SoftDesignTrainingGameMode.h"

EBTNodeResult::Type UBTTask_MoveToFleePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		if (aiController->GetHasFleeLocation()) {
			aiController->MoveToLocation(aiController->GetFleeLocation(), 0.5f, false, true, false, NULL, false);
			aiController->OnMoveToTarget();
			return EBTNodeResult::Succeeded;
		}
		
	}
	return EBTNodeResult::Failed;
}