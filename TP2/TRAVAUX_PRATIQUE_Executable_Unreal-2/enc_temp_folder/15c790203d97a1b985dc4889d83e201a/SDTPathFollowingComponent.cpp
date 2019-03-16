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
	if (!m_isInJumpingState)
	{
		segmentStart = points[MoveSegmentStartIndex];
		if(points.Num() > MoveSegmentStartIndex + 1)
			segmentEnd = points[MoveSegmentStartIndex + 1];
		lastCurveTime = 0;
	}
	ASDTAIController* controller = dynamic_cast<ASDTAIController*>(GetOwner());
	FVector startPoint = segmentStart.Location;
	FVector endPoint = segmentEnd.Location;
	
    if (SDTUtils::HasJumpFlag(segmentStart) || m_isInJumpingState)
    {	
		controller->AtJumpSegment = true;

		FVector pawnPosition = controller->GetPawn()->GetActorLocation();
		FVector2D const toTarget = FVector2D(endPoint - pawnPosition);
		if (!SDTUtils::IsInTargetDirection(controller->GetPawn(), endPoint) && !controller->isStartJumping)
		{
			controller->GetPawn()->SetActorRotation(FMath::Lerp(controller->GetPawn()->GetActorRotation(), FVector(toTarget.GetSafeNormal(), 0.f).Rotation(), 0.1f));			
			return;
		}
		controller->Landing = false;
		m_isInJumpingState = true;
		controller->isStartJumping = true;
		UCurveFloat* JumpCurve = controller->JumpCurve;
		float value = JumpCurve->GetFloatValue(lastCurveTime + 3*DeltaTime);
		lastCurveTime = lastCurveTime + 3*DeltaTime;

		pawnPosition.Z = 216.f + controller->JumpApexHeight * value;
		endPoint.Z = 216.f + controller->JumpApexHeight * value;
		UE_LOG(LogTemp, Warning, TEXT("pawn position : %f"), pawnPosition.Z);

		if (pawnPosition.Z >= 260)
			controller->InAir = true;

		if (pawnPosition.Z <= 265)
			controller->Landing = true;

		FVector2D const displacement =  DeltaTime * toTarget.GetSafeNormal();

		controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
		controller->GetPawn()->AddMovementInput(FVector(toTarget.GetSafeNormal(), 0.f), 100.f);
		if ((pawnPosition - endPoint).Size() <= 30.f)
		{
			endPoint.Z = 216.f;
			controller->GetPawn()->SetActorLocation(endPoint, true);
			m_isInJumpingState = false;
			controller->Landing = true;
			controller->AtJumpSegment = false;
			controller->InAir = false;
			controller->isStartJumping = false;
			UE_LOG(LogTemp, Warning, TEXT("pawn position : %f"), controller->GetPawn()->GetActorLocation().Z);

			UE_LOG(LogTemp, Warning, TEXT("--- --- --- --- ---"));
		}
    }
    else
    {
		Super::FollowPathSegment(DeltaTime);
		controller->Landing = false;
    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
	Super::SetMoveSegment(segmentStartIndex);
	if (MovementComp != NULL)
	{
		const TArray<FNavPathPoint>& points = Path->GetPathPoints();
		const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

		if (m_isInJumpingState || SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
			Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
		else
			Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
	}
}

