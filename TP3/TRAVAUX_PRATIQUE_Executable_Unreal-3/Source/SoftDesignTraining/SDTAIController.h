// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h" 
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
	ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleHalfLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float m_DetectionCapsuleForwardStartingOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		UCurveFloat* JumpCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float JumpApexHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float JumpSpeed = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool AtJumpSegment = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool InAir = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		bool Landing = false;

	virtual void BeginPlay() override;
	void StartBehaviorTree();
	FVector GetPlayerPlayerPos() const { return GetPawn()->GetActorLocation(); }
	void TryDetectPlayer();
	uint8 GetPlayerPosBBKeyID() const { return m_playerPosBBKeyID; }
	uint8 GetPlayerSeenKeyID() const { return m_isPlayerSeenBBKeyID; }
	uint8 GetNextPatrolDestinationKeyID() const { return m_nextPatrolDestinationBBKeyID; }
	uint8 GetCurrentPatrolDestinationKeyID() const { return m_currentPatrolDestinationBBKeyID; }
	bool GetReachedTarget() const { return m_ReachedTarget; }

protected:

	enum PlayerInteractionBehavior
	{
		PlayerInteractionBehavior_Collect,
		PlayerInteractionBehavior_Chase,
		PlayerInteractionBehavior_Flee
	};

	void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
	void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime);
	PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit);
	bool HasLoSOnHit(const FHitResult& hit);
	void PlayerInteractionLoSUpdate();
	void OnPlayerInteractionNoLosDone();
	void OnMoveToTarget();

public:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	void RotateTowards(const FVector& targetLocation);
	void SetActorLocation(const FVector& targetLocation);
	void AIStateInterrupted();
	bool IsPlayerPlayerSeen() const { return m_IsPlayerDetected; }
	void MoveToPlayer();
	void MoveToAnchorPoint();
	void MoveToBestFleeLocation();
	void MoveToRandomCollectible();
	PlayerInteractionBehavior GetPlayerInteractionBehavior() const { return m_PlayerInteractionBehavior; }
	void UpdatePlayerInteraction(float deltaTime);
	void SelectBestFleeLocation();
	void SelectRandomCollectible();
	FVector GetAnchorPoint() const { return m_anchorPoint;}
	void SetAnchorPoint(FVector anchorPoint) { m_anchorPoint = anchorPoint; }

private:
    virtual void GoToBestTarget(float deltaTime) override;
    //virtual void UpdatePlayerInteraction(float deltaTime) override;
    virtual void ShowNavigationPath() override;

protected:
	virtual void Possess(APawn* pawn) override;

private:

	UPROPERTY(transient)
	UBehaviorTreeComponent* m_behaviorTreeComponent;

	UPROPERTY(transient)
	UBlackboardComponent* m_blackboardComponent;

	uint8 m_playerPosBBKeyID;
	uint8 m_isPlayerSeenBBKeyID;
	uint8 m_isPlayerPoweredUpBBKeyID;
	uint8 m_nextPatrolDestinationBBKeyID;
	uint8 m_currentPatrolDestinationBBKeyID;

	bool m_IsPlayerDetected;

	bool m_hasFleeLocation;
	FVector m_fleeLocation;

	bool m_hasCollectibleLocation;
	FVector m_collectibleLocation;

	FVector m_anchorPoint;
protected:
    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;
    PlayerInteractionBehavior m_PlayerInteractionBehavior;
};
