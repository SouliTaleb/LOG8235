// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_IsPlayerPoweredUp.generated.h"

//class UBehaviorTreeComponent;

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTTask_IsPlayerPoweredUp : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override {}
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override {}
};
