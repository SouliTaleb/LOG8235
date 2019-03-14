// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTPathFollowingComponent.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
	:lastCurveTime(0.0f), segmentStart(FNavPathPoint()), segmentEnd(FNavPathPoint())
{
	Initialize();
}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{

	const TArray<FNavPathPoint>& points = Path->GetPathPoints();
	if (!m_isJumping) 
	{
		segmentStart = points[MoveSegmentStartIndex];
		segmentEnd = points[MoveSegmentStartIndex + 1];
	}
	ASDTAIController* controller = dynamic_cast<ASDTAIController*>(GetOwner());

	FVector startPoint = segmentStart.Location;
	FVector endPoint = points[MoveSegmentStartIndex + 1].Location;
	FVector const pawnPosition(controller->GetPawn()->GetActorLocation());
	FVector2D const toTarget = FVector2D(endPoint) - FVector2D(startPoint);
	//FVector2D const displacement = 200.0f * DeltaTime * toTarget.GetSafeNormal();

    if (SDTUtils::HasJumpFlag(segmentStart) || m_isJumping)
    {
		if (!controller->InAir)
		{
			lastCurveTime = 0;
			controller->InAir = true;
		}
	
		m_isJumping = true;
		UCurveFloat* JumpCurve = controller->JumpCurve;
		float value = JumpCurve->GetFloatValue(lastCurveTime + DeltaTime);
		lastCurveTime = lastCurveTime + DeltaTime;
		if (value <= 0.19)
		{
			m_isJumping = false;
			controller->InAir = false;
			FVector currentLocation = FVector(endPoint);
			currentLocation.Z = 216.f;
			controller->GetPawn()->SetActorLocation(currentLocation);
			return;
		}
		FVector pawnPosition = controller->GetPawn()->GetActorLocation();
		pawnPosition.Z = 216.f;
		FVector2D const toTarget = FVector2D(endPoint - pawnPosition);
		FVector2D const displacement = FMath::Min(toTarget.Size(), 200.f * DeltaTime) * toTarget.GetSafeNormal();
		pawnPosition.Z = 216.f + controller->JumpApexHeight * value;
		controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
		//controller->GetPawn()->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());

		//FVector directionToTarget = (endPoint - controller->GetPawn()->GetActorLocation()).GetSafeNormal();
		//FVector currentLocation = FVector(endPoint * lastCurveTime + startPoint * (1.f - lastCurveTime));
		//currentLocation.Z = 200.f + controller->JumpApexHeight * value;
		//controller->GetPawn()->SetActorLocation(currentLocation);

    }
    else
    {
		Super::FollowPathSegment(DeltaTime);
    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
	Super::SetMoveSegment(segmentStartIndex);
	if (MovementComp != NULL)
	{
		const TArray<FNavPathPoint>& points = Path->GetPathPoints();
		const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

		if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
			Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
		else
			Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
	}
}

