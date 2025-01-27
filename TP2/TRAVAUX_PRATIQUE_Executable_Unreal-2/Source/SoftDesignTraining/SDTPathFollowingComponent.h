// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "SDTPathFollowingComponent.generated.h"

/**
*
*/
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API USDTPathFollowingComponent : public UPathFollowingComponent
{
    GENERATED_UCLASS_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	bool m_isInJumpingState;
	FNavPathPoint segmentStart;
	FNavPathPoint segmentEnd;
	float lastCurveTime;
public:
    virtual void FollowPathSegment(float deltaTime) override;
    virtual void SetMoveSegment(int32 segmentStartIndex) override;
};