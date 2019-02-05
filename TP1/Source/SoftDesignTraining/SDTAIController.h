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
		MoveAndRotate,
		FollowPlayer,
		Stop
	};

	enum class ObstacleType
	{
		Wall,
		Slab,
		Player,
		None
	};

	struct HitObject
	{
		void ObstacleDetected(FVector obstacleNormal)
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
		FVector m_obstacleNormal;
		struct FHitResult m_hitInformation;
		float m_allowedDistanceToHit = 0.0f;
	};

	void Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime);

private:
	bool RayCast(const FVector direction);
	bool AvoidObstacle(const float deltaTime);
	bool ISObstacleDetected();
	bool ISCloseToObstacle(const FVector direction, const float allowedDistance, const ObstacleType obstacleType);
	ObstacleType GetObstacleType() const;
	bool SphereOverlap(const FVector& pos, float radius, TArray<struct FOverlapResult>& outOverlaps, bool drawdebug);
	bool CanFollowPlayer(const FVector direction);
	void DebugDrawPrimitive(const UPrimitiveComponent& primitive);
	TArray<FOverlapResult> CollectTargetActorsInFrontOfCharacter(APawn const* pawn);
	void SetVisibilityInformation(bool isVisible);
	bool DetectPlayer(float deltaTime);

private:

	// Visible, shootable
	UPROPERTY(EditAnywhere)
		class UMaterial* VisibleMaterial;

	// not visible, not shootable
	UPROPERTY(EditAnywhere)
		class UMaterial* NonVisibleMaterial;

	FVector2D m_MovementInput;
	FVector2D m_StartingPosition;
	float m_capsuleRadius;
	float m_currentSpeed = 0.0f;
	float const m_maxSpeed = 0.4f;
	float const m_maxAcceleration = 500.0f;
	State m_state = State::MoveForward;
	HitObject m_hitObject;
	float const m_visionAngle = PI / 3.0f;
};
