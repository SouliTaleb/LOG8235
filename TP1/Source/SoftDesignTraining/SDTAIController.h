// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTCollectible.h"
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

	enum class ObjectType
	{
		Wall,
		DeathFloor,
		Player,
		PickUp,
		None
	};

	struct HitObject
	{
		struct FHitResult m_hitInformation;
		float m_allowedDistanceToHit = 0.0f;
	};

	void Move(const FVector2D& direction, float acceleration, float maxSpeed, float deltaTime);

private:
	bool RayCast(const FVector direction, const FVector delta);
	bool AvoidObstacle(const float deltaTime);
	bool ISObstacleDetected();
	bool ISCloseToObject(const FVector direction, const float allowedDistance, const ObjectType objectType);
	ObjectType GetObjectType() const;
	bool SphereOverlap(const FVector& pos, float radius, TArray<struct FOverlapResult>& outOverlaps, bool drawdebug, bool isForPickUps = false);
	bool CanReachTarget(const AActor* const targetActor, ObjectType objectType);
	void DebugDrawPrimitive(const UPrimitiveComponent& primitive);
	TArray<FOverlapResult> CollectTargetActorsInFrontOfCharacter(APawn const* pawn);
	bool IsPlayerDetected(FOverlapResult& overlapActor);
	bool IsPickUpDetected(FOverlapResult& overlapActor);
	bool IsPickUpInFrontOfAIActor(const ASDTCollectible* const pickUpActor);
	void ReachTarget(float deltaTime, AActor* targetActor);

private:
	FVector2D m_MovementInput;
	FVector2D m_StartingPosition;
	float m_capsuleRadius;
	float m_currentSpeed = 0.0f;
	float const m_maxSpeed = 0.4f;
	float const m_maxAcceleration = 500.0f;
	HitObject m_hitObject;
	float const m_visionAngle = PI / 2.0f;
};
