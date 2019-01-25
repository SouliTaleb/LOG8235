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
	enum class State
	{
		MoveForward,
		AvoidObstacle,
		Stop
	};

	struct ObstacleInformation
	{
		void ObstacleDetected(FVector2D obstacleNormal)
		{
			m_obstacleNormal = obstacleNormal;
			m_obstacleAvoided = false;
		}

		bool AvoidObstacle()
		{
			if (!m_obstacleAvoided)
			{
				m_obstacleAvoided = true;
				return true;
			}
			return false;
		}

		bool m_obstacleAvoided = true;
		FVector2D m_obstacleNormal;
	};

	void Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime);

private:
	bool RayCast(struct FHitResult& outHit);
	bool AvoidObstacle(const struct FHitResult&  outHit, const float deltaTime);

private:
	FVector2D m_MovementInput;
	FVector2D m_StartingPosition;
	float m_capsuleRadius;
	float m_currentSpeed = 0.0f;
	float const m_maxSpeed = 500.0f;
	float const m_maxAcceleration = 500.0f;
	State m_state = State::MoveForward;
	ObstacleInformation m_obstacleInformation;
	float const m_visionAngle = PI / 3.0f;
};
