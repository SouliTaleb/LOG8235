// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>
#include "SDTUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "SDTCollectible.h"

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
	
	if (!DetectCollectible(deltaTime)) {
		DetectPlayer(deltaTime);
	}

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
	case State::CatchCollectible:
	{
		if (!DetectCollectible(deltaTime))
			m_state = State::MoveForward;
	}
	break;
	default: break;
	};
}

ASDTAIController::PlayerState ASDTAIController::IsPlayerDetected(FOverlapResult& overlapActor)
{
	TArray<FOverlapResult> foundActors = CollectTargetActorsInFrontOfCharacter(GetPawn());
	ASoftDesignTrainingMainCharacter* playerActor = nullptr;

	for (FOverlapResult overlapResult : foundActors)
	{
		playerActor = dynamic_cast<ASoftDesignTrainingMainCharacter*>(overlapResult.GetActor());
		if (playerActor != nullptr)
		{
			overlapActor = overlapResult;
			bool canReach = CanReachTarget(playerActor, ObjectType::Player);
			if (playerActor->IsPoweredUp() && canReach) {
				return PlayerState::PoweredUp;
			}
			else if (canReach) {
				return PlayerState::Vulnarable;
			}
		}
	}
	return PlayerState::Unseen;
}

bool ASDTAIController::IsPickUpDetected(FOverlapResult& overlapActor)
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

bool ASDTAIController::DetectCollectible(float deltaTime)
{
	TArray<FOverlapResult> foundActors = CollectTargetActorsInFrontOfCharacter(GetPawn());
	ASDTCollectible * ClosestCollectible = nullptr;
	float minDistance = 1000.f;
	for (FOverlapResult overlapResult : foundActors)
	{
		ASDTCollectible* CollectibleActor = Cast<ASDTCollectible>(overlapResult.GetActor());
		if (CollectibleActor != nullptr && !CollectibleActor->IsOnCooldown())
		{
			const FVector targetLocation = CollectibleActor->GetActorLocation();
			const FVector pawnLocation = GetPawn()->GetActorLocation();
			float distance = (targetLocation - pawnLocation).Size();
			FVector directionToTarget = (targetLocation - pawnLocation).GetSafeNormal();
			if (CanCatchCollectible(directionToTarget))
			{
				if (distance < minDistance)
				{
					ClosestCollectible = CollectibleActor;
					minDistance = distance;
				}
			}
		}
	}
	if (ClosestCollectible != nullptr)
	{
		const FVector targetLocation = ClosestCollectible->GetActorLocation();
		const FVector pawnLocation = GetPawn()->GetActorLocation();
		
		FVector directionToTarget = (targetLocation - pawnLocation).GetSafeNormal();
		Move(FVector2D(directionToTarget), m_maxAcceleration, m_maxSpeed, deltaTime);
		m_state = State::CatchCollectible;
		return true;
	}
	
	return false;
}

void ASDTAIController::ReachTarget(float deltaTime, AActor* targetActor)
{
	if (targetActor != nullptr)
	{
		const FVector targetLocation = targetActor->GetActorLocation();
		const FVector pawnLocation = GetPawn()->GetActorLocation();
		const float reachSpeed = 1.f;
		FVector directionToTarget = (targetLocation - pawnLocation);
		if(directionToTarget.Size() > 50.f)
			Move(FVector2D(directionToTarget.GetSafeNormal()), m_maxAcceleration, reachSpeed, deltaTime);
	}
}

void ASDTAIController::Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime)
{
	APawn* pawn = GetPawn();
	m_currentSpeed = FMath::Min(maxSpeed, m_currentSpeed + acceleration * deltaTime);
	FVector const forwardDirection = GetPawn()->GetActorForwardVector();
	
	float AimAtAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(forwardDirection, FVector(direction, 0.f).GetSafeNormal())));
	FVector crossProduct = FVector::CrossProduct(forwardDirection, FVector(direction, 0.f).GetSafeNormal()).GetSafeNormal();
	if (crossProduct.Z == -1.0f)
		AimAtAngle = -AimAtAngle;
	
	//FRotator NewRotation = FRotator(0, AimAtAngle, 0);
	pawn->AddMovementInput(FVector(direction, 0.f), m_currentSpeed);
	//pawn->AddActorWorldRotation(NewRotation);

	pawn->SetActorRotation(FMath::Lerp(pawn->GetActorRotation(), FVector(direction, 0.f).Rotation(), 0.1f));
}

bool ASDTAIController::RayCast(const FVector direction, const FVector delta)
{
	UWorld * world = GetWorld();
	if (world == nullptr)
		return false;

	//Single line trace
	FCollisionObjectQueryParams objectQueryParams;

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_COLLECTIBLE);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_DEATH_OBJECT);
	queryParams.AddIgnoredActor(GetPawn());
	queryParams.bReturnPhysicalMaterial = true;

	APawn* pawn = GetPawn();
	FVector const startTrace = pawn->GetActorLocation() + delta;
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
		FVector2D const newActorDirection = FVector2D(FVector::CrossProduct(FVector::UpVector, GetPawn()->GetActorForwardVector()/*m_hitObject.m_obstacleNormal*/));
		Move(newActorDirection, m_maxAcceleration, m_maxSpeed, deltaTime);
	}
	return true;
}

ASDTAIController::ObjectType ASDTAIController::GetObjectType() const
{
	FName nameActor = m_hitObject.m_hitInformation.GetActor()->GetFName();
	FName nameComponent = m_hitObject.m_hitInformation.GetComponent()->GetFName();
	FString test = nameActor.ToString();
	//UE_LOG(LogTemp, Warning, TEXT("UMyClass %s entering FireWeapon()"), nameActor.ToString());
	if (nameActor.ToString().StartsWith("Wall") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Wall;
	else if (nameActor.ToString().StartsWith("Slab") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Slab;
	else if (nameActor.ToString().StartsWith("BP_SDTMainCharacter_C") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Player;
	else if (nameActor.ToString().StartsWith("BP_SDTCollectible") && nameComponent.ToString().StartsWith("StaticMeshComponent"))
		return ObstacleType::Collectible;
	else return ObstacleType::None;
}

bool ASDTAIController::ISObstacleDetected()
{
	const FVector forwardVectorDirection = GetPawn()->GetActorForwardVector();
	FVector floorDirection = forwardVectorDirection;
	floorDirection.Z= -.25f;
	floorDirection.GetSafeNormal();
	return ISCloseToObstacle(floorDirection, 150.f, ObstacleType::Slab) ||
		ISCloseToObstacle(forwardVectorDirection, 150.f, ObstacleType::Wall);
}

bool ASDTAIController::ISCloseToObject(const FVector direction, const float allowedDistance, const ObjectType objectType)
{
	APawn* pawn = GetPawn();

	FVector originBB;
	FVector extentsBB;
	pawn->GetActorBounds(true, originBB, extentsBB);
	extentsBB.Z = 0.f;

	FVector leftSide = FVector::CrossProduct(direction, pawn->GetActorUpVector());
	//leftSide.Z = pawn->GetActorForwardVector().Z;
	leftSide.Normalize();
	leftSide *= extentsBB.Size()* 0.7f;
	FVector rightSide = -leftSide;

	if (RayCast(direction, leftSide) && RayCast(direction, rightSide) && GetObjectType() == objectType)
	{
		float distanceToImpactPoint = (m_hitObject.m_hitInformation.ImpactPoint - GetPawn()->GetActorLocation()).Size();
		m_hitObject.m_allowedDistanceToHit = allowedDistance;
		return (distanceToImpactPoint <= allowedDistance);
	}
	return false;
}

bool ASDTAIController::CanReachTarget(const AActor* const targetActor, ObjectType objectType)
{
	bool result = (RayCast(direction) && Cast<ASoftDesignTrainingMainCharacter>(m_hitObject.m_hitInformation.GetActor()) != nullptr);
	return result;
}

bool ASDTAIController::CanCatchCollectible(const FVector direction)
{
	return (RayCast(direction) && Cast<ASDTCollectible>(m_hitObject.m_hitInformation.GetActor()) != nullptr);
}

bool ASDTAIController::SphereOverlap(const FVector& pos, float radius, TArray<struct FOverlapResult>& outOverlaps, bool drawDebug)
{
	UWorld * world = GetWorld();
	if (world == nullptr)
		return false;

	if (drawDebug)
		DrawDebugSphere(world, pos, radius, 24, FColor::Green);


	FCollisionObjectQueryParams objectQueryParams; // All objects
	
	objectQueryParams.AddObjectTypesToQuery(COLLISION_DEATH_OBJECT);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_COLLECTIBLE);
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

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
			if (Cast<ASDTCollectible>(outOverlaps[i].GetActor()) != nullptr) {
				DebugDrawPrimitiveColor(*(outOverlaps[i].GetComponent()), FColor::Purple);
			}
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

void ASDTAIController::DebugDrawPrimitiveColor(const UPrimitiveComponent& primitive, FColor color)
{
	FVector center = primitive.Bounds.Origin;
	FVector extent = primitive.Bounds.BoxExtent;
	UWorld * world = GetWorld();
	if (world == nullptr)
		return;
	DrawDebugBox(world, center, extent, color);
}