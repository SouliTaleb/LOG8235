// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include <cmath>

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
	switch (m_state)
	{
		case State::MoveForward:
		{
			FVector const ForwardDirection = GetPawn()->GetActorForwardVector().GetSafeNormal();
			Move(FVector2D(ForwardDirection), m_maxAcceleration, m_maxSpeed, deltaTime);

			struct FHitResult hitResult;
			if (RayCast(hitResult))
			{
				float distanceToImpactPoint = (hitResult.ImpactPoint - GetPawn()->GetActorLocation()).Size();
				if (distanceToImpactPoint <= 100.f)
				{
					m_state = State::AvoidObstacle;
				} 
			}
		}
		break;
		case State::AvoidObstacle:
		{
			struct FHitResult hitResult;
			if (RayCast(hitResult))
			{
				m_obstacleInformation.ObstacleDetected(FVector2D(hitResult.Normal));
				AvoidObstacle(hitResult, deltaTime);
			}
			m_state = State::MoveForward;
		}
		break;
		case State::Stop:
		{

		}
		break;
	};
}

void ASDTAIController::Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime)
{
	APawn* pawn = GetPawn();
	m_currentSpeed = FMath::Min(maxSpeed, m_currentSpeed + acceleration * deltaTime);
	pawn->AddMovementInput(FVector(direction, 0.f), m_currentSpeed);
	FRotator deltaRotation = FVector(direction, 0.f).ToOrientationRotator();
	pawn->SetActorRotation(FVector(direction, 0.f).ToOrientationQuat());
}

bool ASDTAIController::RayCast(struct FHitResult& outHit)
{
	UWorld * world = GetWorld();
	if (world == nullptr)
		return false;

	//Single line trace
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.bReturnPhysicalMaterial = true;

	APawn* pawn = GetPawn();
	FVector const actorForwardDirection = pawn->GetActorForwardVector();
	FVector const startTrace = pawn->GetActorLocation();
	FVector const endTrace = actorForwardDirection * 1000.f + startTrace;

	return world->LineTraceSingleByObjectType(outHit, startTrace, endTrace, objectQueryParams, queryParams) 
		   && (outHit.GetActor() != NULL);
}

bool ASDTAIController::AvoidObstacle(const struct FHitResult& obstacle, const float deltaTime)
{
	float distanceToImpactPoint = 0.f;
	distanceToImpactPoint = (obstacle.ImpactPoint - GetPawn()->GetActorLocation()).Size();
	if(distanceToImpactPoint <= 100.f)
	{
		FVector2D const newActorDirection = FVector2D(FVector::CrossProduct(FVector::UpVector, FVector(m_obstacleInformation.m_obstacleNormal, 0.0f)));
		Move(newActorDirection, m_maxAcceleration, m_maxSpeed, deltaTime);
	}
	return true; 
}