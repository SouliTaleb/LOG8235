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
	FVector const actorForwardDirection = GetPawn()->GetActorForwardVector();
	struct FHitResult hitResult;
	FOverlapResult overlapPlayer;
	FOverlapResult overlapPickUp;
	if (IsPickUpDetected(overlapPickUp))
		ReachTarget(deltaTime, overlapPickUp.GetActor());
	else if (IsPlayerDetected(overlapPlayer))
		ReachTarget(deltaTime, overlapPlayer.GetActor());
	else if (ISObstacleDetected())
		AvoidObstacle(deltaTime);
	else
		Move(FVector2D(actorForwardDirection), m_maxAcceleration, m_maxSpeed, deltaTime);
}

bool ASDTAIController::IsPlayerDetected(FOverlapResult& overlapActor)
{
	TArray<FOverlapResult> foundActors = CollectTargetActorsInFrontOfCharacter(GetPawn());
	ASoftDesignTrainingMainCharacter* playerActor = nullptr;

	for (FOverlapResult overlapResult : foundActors)
	{
		playerActor = dynamic_cast<ASoftDesignTrainingMainCharacter*>(overlapResult.GetActor());
		if (playerActor != nullptr)
		{
			overlapActor = overlapResult;
			return CanReachTarget(playerActor, ObjectType::Player);
		}
	}
	return false;
}

bool ASDTAIController::IsPickUpDetected(FOverlapResult& overlapActor)
{

	TArray<FOverlapResult> outResults;
	APawn* pawn = GetPawn();
	SphereOverlap(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 500.0f, 600.0f, outResults, true, true);

	float minDistance = 10000.f;
	bool isPickUpDetected = false;
	for (FOverlapResult overlapResult : outResults)
	{
		ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(overlapResult.GetActor());
		if (collectible != nullptr && !collectible->IsOnCooldown() && IsPickUpInFrontOfAIActor(collectible) && CanReachTarget(collectible, ObjectType::PickUp))
		{
			float newDistance = (collectible->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
			if (newDistance < minDistance)
			{
				minDistance = newDistance;
				overlapActor = overlapResult;
				isPickUpDetected = true;
			}
		}		
	}
	return isPickUpDetected;
}

void ASDTAIController::ReachTarget(float deltaTime, AActor* targetActor)
{
	if (targetActor != nullptr)
	{
		const FVector targetLocation = targetActor->GetActorLocation();
		const FVector pawnLocation = GetPawn()->GetActorLocation();
		const float reachSpeed = 0.7f;
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
	objectQueryParams.AddObjectTypesToQuery(COLLISION_DEATH_OBJECT);
	objectQueryParams.AddObjectTypesToQuery(COLLISION_COLLECTIBLE);
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	objectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

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
		FVector2D const newActorDirection = FVector2D(FVector::CrossProduct(FVector::UpVector, m_hitObject.m_hitInformation.ImpactNormal));
		Move(newActorDirection, m_maxAcceleration, m_maxSpeed / 2.0f, deltaTime);
	}
	return true;
}

ASDTAIController::ObjectType ASDTAIController::GetObjectType() const
{
	FName nameActor = m_hitObject.m_hitInformation.GetActor()->GetFName();
	FName nameComponent = m_hitObject.m_hitInformation.GetComponent()->GetFName();
	if (nameActor.ToString().StartsWith("Wall"))
		return ObjectType::Wall;
	else if (nameActor.ToString().StartsWith("BP_DeathFloor"))
		return ObjectType::DeathFloor;
	else if (dynamic_cast<ASDTCollectible*>(m_hitObject.m_hitInformation.GetActor()) != nullptr)
		return ObjectType::PickUp;
	else 
		return ObjectType::None;
}

bool ASDTAIController::ISObstacleDetected()
{
	const FVector forwardVectorDirection = GetPawn()->GetActorForwardVector();
	FVector floorDirection = forwardVectorDirection;
	floorDirection.Z= -1.f;
	floorDirection.Normalize();
	return ISCloseToObject(floorDirection, 700.f, ObjectType::DeathFloor) ||
		ISCloseToObject(forwardVectorDirection, 300.f, ObjectType::Wall);
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
	const FVector targetLocation = targetActor->GetActorLocation();
	const FVector pawnLocation = GetPawn()->GetActorLocation();

	FVector directionToTarget = (targetLocation - pawnLocation).GetSafeNormal();
	if (RayCast(directionToTarget, FVector::ZeroVector))
	{
		if(objectType == ObjectType::Player)
			return dynamic_cast<ASoftDesignTrainingMainCharacter*>(m_hitObject.m_hitInformation.GetActor()) != nullptr;
		else if (objectType == ObjectType::PickUp)
			return dynamic_cast<ASDTCollectible*>(m_hitObject.m_hitInformation.GetActor()) != nullptr;
	}
	return false;
}

bool ASDTAIController::SphereOverlap(const FVector& pos, float radius, TArray<struct FOverlapResult>& outOverlaps, bool drawDebug, bool isForPickUps)
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
		}
	}

	return outOverlaps.Num() > 0;
}

TArray<FOverlapResult> ASDTAIController::CollectTargetActorsInFrontOfCharacter(APawn const* pawn)
{
	TArray<FOverlapResult> outResults;
	SphereOverlap(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 500.0f, 600.0f, outResults, true);
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

bool ASDTAIController::IsPickUpInFrontOfAIActor(const ASDTCollectible* const pickUpActor) 
{
	if (pickUpActor == nullptr)
		return false;
	FVector const toTarget = pickUpActor->GetActorLocation() - GetPawn()->GetActorLocation();
	FVector const pawnForward = GetPawn()->GetActorForwardVector();
	bool isPickUpInsideCone = std::abs(std::acos(FVector::DotProduct(pawnForward.GetSafeNormal(), toTarget.GetSafeNormal()))) < m_visionAngle;
	return  isPickUpInsideCone;
}