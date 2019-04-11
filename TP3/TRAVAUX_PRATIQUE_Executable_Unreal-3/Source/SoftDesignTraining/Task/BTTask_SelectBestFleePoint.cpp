// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/Navigation/NavigationSystem.h"
#include "BTTask_SelectBestFleePoint.h"
#include "SoftDesignTrainingGameMode.h"

EBTNodeResult::Type UBTTask_SelectBestFleePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		
		if (!aiController->GetHasFleeLocation())
		{
			aiController->SelectBestFleeLocation();
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}