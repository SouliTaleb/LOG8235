// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
	ASDTAIController();

	virtual void BeginPlay() override;
    virtual void Tick(float deltaTime) override;

protected:
	virtual void Move(float acceleration, float maxSpeed, float deltaTime);

	FVector2D m_MovementInput;
	FVector2D m_StartingPosition;
	float m_capsuleRadius;
	float m_currentSpeed = 0.0f;
	float const m_maxSpeed = 500.0f;
	float const m_maxAcceleration = 500.0f;
	float const m_visionAngle = PI / 3.0f;
};
