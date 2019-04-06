// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_TryDetectPlayer.generated.h"

//class UBehaviorTreeComponent;
//class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTService_TryDetectPlayer : public UBTService
{
	GENERATED_BODY()
	
public:
    UBTService_TryDetectPlayer();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override {}
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override {}
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override {}
};
