// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "SoftDesignTrainingMainCharacter.h"


ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent"))),
	m_collectible(nullptr),
	m_player_pos(FVector::ZeroVector),
	m_flee_point_pos(FVector::ZeroVector),
	m_currentAgentState(AgentState::None)
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
	ACharacter* character =  dynamic_cast<ACharacter*>(GetPawn());
    //Move to target depending on current behavior
	if (m_currentAgentState == AgentState::ReachPlayerPosition)
	{		
		m_runSpeed = 600;
		character->GetCharacterMovement()->MaxWalkSpeed = m_runSpeed;
		MoveToLocation(m_player_pos);
		if ((GetPawn()->GetActorLocation() - m_player_pos).Size() <= 60.f)
		{
			m_player_pos = FVector::ZeroVector;
			m_collectible = nullptr;
		}
	}
	else if (m_currentAgentState == AgentState::ReachFleePoint)
	{
		m_runSpeed = 600;
		character->GetCharacterMovement()->MaxWalkSpeed = m_runSpeed;
		MoveToLocation(m_flee_point_pos);
		if ((GetPawn()->GetActorLocation() - m_flee_point_pos).Size() <= 200.f)
		{
			m_player_pos = FVector::ZeroVector;
			m_collectible = nullptr;
		}
	}
	else if (m_currentAgentState == AgentState::ReachCollectible)
	{
		m_runSpeed = 200;
		character->GetCharacterMovement()->MaxWalkSpeed = m_runSpeed;
		MoveToLocation(m_collectible->GetActorLocation());
	}
	if(GetPathFollowingComponent()->GetPath().IsValid())
		ShowNavigationPath();
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere
	const TArray<FNavPathPoint>& navPoints = GetPathFollowingComponent()->GetPath()->GetPathPoints();
	FNavPathPoint precedentNavPathPoint;
	for (FNavPathPoint navPathPoint : navPoints)
	{
		DrawDebugSphere(GetWorld(), navPathPoint.Location, 50.f, 24, FColor::Purple);
		if (precedentNavPathPoint.HasNodeRef() && sizeof(navPoints) > 1)
			DrawDebugLine(GetWorld(), precedentNavPathPoint.Location, navPathPoint.Location, FColor::Purple);
		precedentNavPathPoint = navPathPoint;
	}
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    //Set behavior based on hit
	ASoftDesignTrainingMainCharacter* playerActor = dynamic_cast<ASoftDesignTrainingMainCharacter*>(detectionHit.GetActor());
	if (playerActor != nullptr)
	{
		m_player_pos = detectionHit.GetActor()->GetActorLocation();
		if(!playerActor->IsPoweredUp())
			m_currentAgentState = AgentState::ReachPlayerPosition;
		else
		{
			TArray<AActor*> fleePoints;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), fleePoints);
			ASDTFleeLocation* pertinentFleeLocation = nullptr;
			for (AActor* fleeActor : fleePoints)
			{
				if ((pertinentFleeLocation == nullptr) || ((m_player_pos - fleeActor->GetActorLocation()).Size() > (m_player_pos - pertinentFleeLocation->GetActorLocation()).Size()))
				{
					pertinentFleeLocation = dynamic_cast<ASDTFleeLocation*>(fleeActor);
				}
			}
			if (pertinentFleeLocation != nullptr)
			{
				m_flee_point_pos = pertinentFleeLocation->GetActorLocation();
				m_currentAgentState = AgentState::ReachFleePoint;
			}				
		}
	}
	else
	{
		TArray<AActor*> collectibles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), collectibles);
		int randomIndex = 0;
		ASDTCollectible* collectible = nullptr;
		do
		{
			randomIndex = FMath::RandRange(0, collectibles.Num() - 1);
			collectible = dynamic_cast<ASDTCollectible*>(collectibles[randomIndex]);
		} while (collectible->IsOnCooldown());

		if ((m_player_pos == FVector::ZeroVector) && ((m_collectible == nullptr) || m_collectible->IsOnCooldown() || ((GetPawn()->GetActorLocation() - m_collectible->GetActorLocation()).Size() < 50.f)))
		{
			m_currentAgentState = AgentState::ReachCollectible;
			m_collectible = collectible;
		}
	}
    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
				ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(hit.GetActor());
				if(!collectible->IsOnCooldown())
					outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}