// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"

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
	Move(m_maxAcceleration, m_maxSpeed, deltaTime);
}

void ASDTAIController::Move(float acceleration, float maxSpeed, float deltaTime)
{
	APawn* pawn = GetPawn();
	FVector const ForwardDirection = pawn->GetActorForwardVector().GetSafeNormal();
	m_currentSpeed = FMath::Min(maxSpeed, m_currentSpeed + acceleration * deltaTime);
	pawn->AddMovementInput(ForwardDirection, m_currentSpeed);
}