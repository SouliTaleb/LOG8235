// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BTTask_IsPlayerDetected.h"
#include "AiAgentGroupManager.h"

EBTNodeResult::Type UBTTask_IsPlayerDetected::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
	{
        if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>(aiController->GetPlayerSeenKeyID()))
        {
			AiAgentGroupManager::GetInstance()->RegisterAIAgent(aiController);
			return EBTNodeResult::Succeeded;
		}
		AiAgentGroupManager::GetInstance()->UnregisterAIAgent(aiController);
	}
	return EBTNodeResult::Failed;
}


