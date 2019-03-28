// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SoftDesignAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "AIBase.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AI/Navigation/NavigationSystem.h"
#include "ObjectPartition.h"
#include "DrawDebugHelpers.h"
#include "BTTask_ProcessAvoidance.h"



EBTNodeResult::Type UBTTask_ProcessAvoidance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASoftDesignAIController* aiController = Cast<ASoftDesignAIController>(OwnerComp.GetAIOwner()))
    {
        if (AAIBase* aiBase = Cast<AAIBase>(aiController->GetCharacter()))
        {
            FVector npcPosition = aiBase->GetActorLocation();
            FVector npcHead = npcPosition + FVector::UpVector * 200.0f;
            UWorld* npcWorld = GetWorld();

            ObjectPartition* op = ObjectPartition::GetInstance();
            if (op)
            {
                TArray<AActor*> actors;
                op->GatherObjectsAroundPosition(aiBase, npcPosition, 250.f, actors);
                if (actors.Num() > 0)
                {
                    DrawDebugSphere(npcWorld, npcHead, 20.0f, 32, FColor::Magenta);
                }
            }
        }
    }

   

    return EBTNodeResult::InProgress;
}
