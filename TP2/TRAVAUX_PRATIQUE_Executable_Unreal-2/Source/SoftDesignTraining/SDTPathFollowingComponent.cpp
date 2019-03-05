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
		return;
	
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        //update jump
		//CharacterMoveComp->ReceiveJumpFallPathingRequest();
		m_isJumping = true;
    }
    else
    {
        //update navigation along path
		m_isJumping = false;
		Super::FollowPathSegment(DeltaTime);
    }
}

/*
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	if (!Path.IsValid() || MovementComp == nullptr)
	{
		return;
	}

	const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
	const FVector CurrentTarget = GetCurrentTargetLocation();

	const bool bAccelerationBased = MovementComp->UseAccelerationForPathFollowing();
	if (bAccelerationBased)
	{
		CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();

		if (MoveSegmentStartIndex >= DecelerationSegmentIndex)
		{
			const FVector PathEnd = Path->GetEndLocation();
			const float DistToEndSq = FVector::DistSquared(CurrentLocation, PathEnd);
			const bool bShouldDecelerate = DistToEndSq < FMath::Square(CachedBrakingDistance);
			if (bShouldDecelerate)
			{
				const float SpeedPct = FMath::Clamp(FMath::Sqrt(DistToEndSq) / CachedBrakingDistance, 0.0f, 1.0f);
				CurrentMoveInput *= SpeedPct;
			}
		}

		PostProcessMove.ExecuteIfBound(this, CurrentMoveInput);
		MovementComp->RequestPathMove(CurrentMoveInput);
	}
	else
	{
		FVector MoveVelocity = (CurrentTarget - CurrentLocation) / DeltaTime;

		const int32 LastSegmentStartIndex = Path->GetPathPoints().Num() - 2;
		const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

		PostProcessMove.ExecuteIfBound(this, MoveVelocity);
		MovementComp->RequestDirectMove(MoveVelocity, bNotFollowingLastSegment);
	}
}
*/

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

	if (CharacterMoveComp != NULL)
	{
		const TArray<FNavPathPoint>& points = Path->GetPathPoints();
		const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

		if (SDTUtils::HasJumpFlag(segmentStart) /*&& FNavMeshNodeFlags(segmentStart.Flags).IsNavLink()*/)
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

