// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTPathFollowingComponent.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
	:lastCurveTime(0.0f)
{
	Initialize();
}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	if (!Path.IsValid() || MovementComp == nullptr)
		return;
	
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
	ASDTAIController* controller = dynamic_cast<ASDTAIController*>(GetOwner());

	FVector startPoint = segmentStart.Location;
	FVector endPoint = points[MoveSegmentStartIndex + 2].Location;
	FVector const pawnPosition(controller->GetPawn()->GetActorLocation());
	FVector2D const toTarget = FVector2D(endPoint) - FVector2D(pawnPosition);
	FVector2D const displacement = 200.0f * DeltaTime * toTarget.GetSafeNormal();

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
		m_isJumping = true;
		controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f) + FVector(0, 0, 100.f), true);
		controller->GetPawn()->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());
		
		/*controller->AtJumpSegment = true;
		controller->InAir = true;

		UCurveFloat* JumpCurve = controller->JumpCurve;

		//if (JumpCurve)
		//{
		float value = JumpCurve->GetFloatValue(lastCurveTime + DeltaTime);
		if (value == 0.67f)
			controller->Landing = true;


		//FVector2D const displacement = FMath::Min(toTarget.Size(), 200.0f * DeltaTime) * toTarget.GetSafeNormal();

	//	pawn->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
	//	pawn->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());

		if (!controller->Landing)
		{
			//controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f) + FVector(0, 0, value*10), true);
			controller->GetPawn()->AddMovementInput(FVector(toTarget.GetSafeNormal(), 0.f), 200.f);
		}
		else
		{
			controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f) + FVector(0, 0, -value), true);
		}
		controller->GetPawn()->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());
		lastCurveTime = lastCurveTime + DeltaTime;


		//FVector2D const contactDirection = FVector2D(FVector::CrossProduct(FVector::UpVector, FVector(m_hitInformation.m_hitNormal, 0.0f)));

		//FVector2D const displacement = 200.0f * DeltaTime * FVector2D(endPoint - controller->GetPawn()->GetActorLocation()).GetSafeNormal();
		//controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
		//controller->GetPawn()->SetActorRotation(FVector(endPoint - controller->GetPawn()->GetActorLocation(), 0.f).ToOrientationQuat());



		//	controller->GetPawn()->SetActorLocation(controller->GetPawn()->GetActorLocation() + FVector(JumpCurve->GetFloatValue(lastCurveTime + DeltaTime) * 3000.f, 0, 3000.f * JumpCurve->GetFloatValue(lastCurveTime + DeltaTime))*1000);
			
			
		//}*/
    }
    else
    {
		if (m_isJumping)
			controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f) + FVector(0, 0, 100.f), true);
		else 
			controller->GetPawn()->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);

		controller->GetPawn()->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());



        //update navigation along path
		//m_isJumping = false;
		//Super::FollowPathSegment(DeltaTime);
		//lastCurveTime = 0.f;
		//controller->InAir = false;
		//controller->AtJumpSegment = false;
		//controller->Landing = false;
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

