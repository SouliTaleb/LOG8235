// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AI/Navigation/NavigationSystem.h"
#include "BTTask_MoveToPlayer.h"

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
		ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(aiController->GetWorld(), 0);
		FVector playerPosition = playerCharacter->GetActorLocation();
		if ((aiController->GetPawn()->GetActorLocation() - playerPosition).Size() < 250.f)
		{
			aiController->MoveToPlayer();
		}
		else
		{
			aiController->MoveToAnchorPoint();
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}