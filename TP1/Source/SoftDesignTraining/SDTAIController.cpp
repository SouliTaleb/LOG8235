// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>
#include "SDTUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "SoftDesignTrainingMainCharacter.h"

ASDTAIController::ASDTAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASDTAIController::BeginPlay()
{
	Super::BeginPlay();
	m_capsuleRadius = 0.0f;

	if (UCapsuleComponent const* capsule = GetPawn()->FindComponentByClass<UCapsuleComponent>())
	{
		m_capsuleRadius = capsule->GetScaledCapsuleRadius() + 4.0f;
	}
	m_StartingPosition = FVector2D(GetPawn()->GetActorLocation());
}

void ASDTAIController::Tick(float deltaTime)
{
	DetectPlayer(deltaTime);

	FVector const actorForwardDirection = GetPawn()->GetActorForwardVector();
	struct FHitResult hitResult;
	switch (m_state)
	{
	case State::MoveForward:
	{
		Move(FVector2D(actorForwardDirection), m_maxAcceleration, m_maxSpeed, deltaTime);
		if (ISObstacleDetected())
			m_state = State::AvoidObstacle;
	}
	break;
	case State::AvoidObstacle:
	{
		m_hitObject.m_obstacleNormal = m_hitObject.m_hitInformation.ImpactNormal;
		AvoidObstacle(deltaTime);
		m_state = State::MoveForward;
	}
	break;
	case State::FollowPlayer:
	{
		if (!DetectPlayer(deltaTime))
			m_state = State::MoveForward;
	}
	break;
	default: break;
	};
}


bool ASDTAIController::DetectPlayer(float deltaTime)
{
	TArray<FOverlapResult> foundActors = CollectTargetActorsInFrontOfCharacter(GetPawn());
	for (FOverlapResult overlapResult : foundActors)
	{
		ASoftDesignTrainingMainCharacter* targetActor = Cast<ASoftDesignTrainingMainCharacter>(overlapResult.GetActor());
		if (targetActor != nullptr)
		{
			const FVector targetLocation = targetActor->GetActorLocation();
			const FVector pawnLocation = GetPawn()->GetActorLocation();

			FVector directionToTarget = (targetLocation - pawnLocation).GetSafeNormal();
			if (CanFollowPlayer(directionToTarget))
			{
				Move(FVector2D(directionToTarget), m_maxAcceleration, m_maxSpeed, deltaTime);
				return true;
			}
		}
	}
	return false;
}

void ASDTAIController::Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime)
{
	APawn* pawn = GetPawn();
	m_currentSpeed = FMath::Min(maxSpeed, m_currentSpeed + acceleration * deltaTime);
	FVector const forwardDirection = GetPawn()->GetActorForwardVector();
	pawn->AddMovementInput(FVector(direction, 0.f), m_currentSpeed);
	float AimAtAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(forwardDirection, FVector(direction, 0.f))));
	FRotator NewRotation = FRotator(0, AimAtAngle, 0);
	pawn->AddActorWorldRotation(NewRotation);
}

bool ASDTAIController::RayCast(const FVector direction)
{
	UWorld * world = GetWorld();
	if (world == nullptr)
		return false;

	//Single line trace
	FCollisionObjectQueryParams objectQueryParams;

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.AddIgnoredActor(GetPawn());
	queryParams.bReturnPhysicalMaterial = true;

	APawn* pawn = GetPawn();
	FVector const startTrace = pawn->GetActorLocation();
	FVector const endTrace = direction * 5000.f + startTrace;

	DrawDebugLine(world, startTrace, endTrace, FColor::Red);

	world->LineTraceSingleByObjectType(m_hitObject.m_hitInformation, startTrace, endTrace, objectQueryParams, queryParams);
	return (m_hitObject.m_hitInformation.GetActor() != nullptr);
}

bool ASDTAIController::AvoidObstacle(const float deltaTime)
{
	float distanceToImpactPoint = (m_hitObject.m_hitInformation.ImpactPoint - GetPawn()->GetActorLocation()).Size();
	if (distanceToImpactPoint <= m_hitObject.m_allowedDistanceToHit)
	{
		FVector2D const newActorDirection = FVector2D(FVector::CrossProduct(FVector::UpVector, m_hitObject.m_obstacleNormal));
		Move(newActorDirection, m_maxAcceleration, m_maxSpeed, deltaTime);
	}
	return true;
}

ASDTAIController::ObstacleType ASDTAIController::GetObstacleType() const
{
	FName nameActor = m_hitObject.m_hitInformation.GetActor()->GetFName();
	FName nameComponent = m_hitObject.m_hitInformation.GetComponent()->GetFName();
	if (nameActor.ToString().StartsWith("Wall") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Wall;
	else if (nameActor.ToString().StartsWith("Slab") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Slab;
	else if (nameActor.ToString().StartsWith("BP_SDTMainCharacter_C") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Player;
	else return ObstacleType::None;
}

bool ASDTAIController::ISObstacleDetected()
{
	const FVector forwardVectorDirection = GetPawn()->GetActorForwardVector();
	FVector floorDirection = forwardVectorDirection;
	floorDirection.Z = -1.0f;
	floorDirection.GetSafeNormal();
	return ISCloseToObstacle(floorDirection, 350.f, ObstacleType::Slab); //||
		//ISCloseToObstacle(forwardVectorDirection, 150.f, ObstacleType::Wall);
}

bool ASDTAIController::ISCloseToObstacle(const FVector direction, const float allowedDistance, const ObstacleType obstacleType)
{
	if (RayCast(direction) && GetObstacleType() == obstacleType)
	{
		float distanceToImpactPoint = (m_hitObject.m_hitInformation.ImpactPoint - GetPawn()->GetActorLocation()).Size();
		m_hitObject.m_allowedDistanceToHit = allowedDistance;
		return (distanceToImpactPoint <= allowedDistance);
	}
	return false;
}

bool ASDTAIController::CanFollowPlayer(const FVector direction)
{
	return (RayCast(direction) && Cast<ASoftDesignTrainingMainCharacter>(m_hitObject.m_hitInformation.GetActor()) != nullptr);
}

bool ASDTAIController::SphereOverlap(const FVector& pos, float radius, TArray<struct FOverlapResult>& outOverlaps, bool drawDebug)
{
	UWorld * world = GetWorld();
	if (world == nullptr)
		return false;

	if (drawDebug)
		DrawDebugSphere(world, pos, radius, 24, FColor::Green);


	FCollisionObjectQueryParams objectQueryParams; // All objects
	FCollisionShape collisionShape;
	collisionShape.SetSphere(radius);
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.bReturnPhysicalMaterial = true;

	world->OverlapMultiByObjectType(outOverlaps, pos, FQuat::Identity, objectQueryParams, collisionShape, queryParams);

	//Draw overlap results
	if (drawDebug)
	{
		for (int32 i = 0; i < outOverlaps.Num(); ++i)
		{
			if (outOverlaps[i].GetComponent())
				DebugDrawPrimitive(*(outOverlaps[i].GetComponent()));
		}
	}

	return outOverlaps.Num() > 0;
}

TArray<FOverlapResult> ASDTAIController::CollectTargetActorsInFrontOfCharacter(APawn const* pawn)
{
	TArray<FOverlapResult> outResults;
	SphereOverlap(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 750.0f, 1000.0f, outResults, true);
	return outResults;
}

void ASDTAIController::DebugDrawPrimitive(const UPrimitiveComponent& primitive)
{
	FVector center = primitive.Bounds.Origin;
	FVector extent = primitive.Bounds.BoxExtent;
	UWorld * world = GetWorld();
	if (world == nullptr)
		return;
	DrawDebugBox(world, center, extent, FColor::Red);
}
