// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTPathFollowingComponent.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{
	Initialize();
}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	if (!Path.IsValid() || MovementComp == nullptr)
	{
		return;
	}

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
	//ASDTAIController agent = dynamic_cast<ASDTAIController*>(GetWorld()-);
    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        //update jump
		if (CharacterMoveComp != NULL)
		{
			//CharacterMoveComp->jumpReceiveJumpFallPathingRequest();
		}
    }
    else
    {
        //update navigation along path
		Super::FollowPathSegment(DeltaTime);
    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

	if (CharacterMoveComp != NULL)
	{
		const TArray<FNavPathPoint>& points = Path->GetPathPoints();
		const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

		if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
		{
		    //Handle starting jump
			CharacterMoveComp->SetMovementMode(MOVE_Flying);
		}
		else
		{
		    //Handle normal segments
			CharacterMoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}
