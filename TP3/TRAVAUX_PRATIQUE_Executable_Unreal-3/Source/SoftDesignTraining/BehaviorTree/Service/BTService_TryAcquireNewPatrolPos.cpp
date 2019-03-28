// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "AIBase.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BTService_TryAcquireNewPatrolPos.h"

UBTService_TryAcquireNewPatrolPos::UBTService_TryAcquireNewPatrolPos()
{
    bCreateNodeInstance = true;
}

void UBTService_TryAcquireNewPatrolPos::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (AAIBase* aiBase = Cast<AAIBase>(aiController->GetCharacter()))
        {
            FVector nextPatrolDestination = FVector::ZeroVector;

            //We are restarting a patrol
            if (FVector::ZeroVector == aiBase->GetCurrentPatrolDestination())
            {
                //Get the next patrol pos
                nextPatrolDestination = aiBase->GetNextPatrolDestination();
                aiBase->SetCurrentPatrolDestination(nextPatrolDestination);
            }
            else if ((aiBase->GetActorLocation() - aiBase->GetNextPatrolDestination()).Size2D() < 100.f)
            {
                //Process new patrol point
                aiBase->ProcessNextPatrolDestination(nextPatrolDestination);
            }

            //Write to blackboard
            if (FVector::ZeroVector != nextPatrolDestination )
            {
                OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(aiController->GetNextPatrolDestinationKeyID(), nextPatrolDestination);
            }
        }
    }
}



