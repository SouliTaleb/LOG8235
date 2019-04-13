// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SDTPathFollowingComponent.h"
#include "SDTFleeLocation.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "EngineUtils.h"
#include "AiAgentGroupManager.h"
#include "SoftDesignTrainingGameMode.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
	  , m_isPlayerSeenBBKeyID(0)
	 , m_anchorPoint(FVector::ZeroVector)
{
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
	m_behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	m_blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ASDTAIController::BeginPlay()
{
	Super::BeginPlay();
	
	StartBehaviorTree();
	loadBalancer = ((ASoftDesignTrainingGameMode*) GetWorld()->GetAuthGameMode())->loadBalancer;
	loadBalancer->increaseCount();
}

void ASDTAIController::MoveToRandomCollectible()
{
	if (!loadBalancer->canExecute(lastUpdateFrame)) return;

	SelectRandomCollectible();

	if (m_hasCollectibleLocation)
	{
		MoveToLocation(m_collectibleLocation, 0.5f, false, true, true, NULL, false);
		OnMoveToTarget();
	}
}

void ASDTAIController::MoveToPlayer()
{
    ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;
	
    MoveToLocation(playerCharacter->GetActorLocation(), 0.5f, false, true, true, NULL, false);
    OnMoveToTarget();
}

void ASDTAIController::MoveToAnchorPoint()
{
	AiAgentGroupManager* aiAgentGroupManager = AiAgentGroupManager::GetInstance();
	aiAgentGroupManager->DrawSphereOverHead();
	aiAgentGroupManager->GenerateAnchorPoints();

	MoveToLocation(m_anchorPoint, 0.5f, false, true, true, NULL, false);
	OnMoveToTarget();
}

void ASDTAIController::PlayerInteractionLoSUpdate()
{
    ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    bool hasLosOnPlayer = false;

    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            hasLosOnPlayer = true;
        }
    }

    if (hasLosOnPlayer)
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
        {
            GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
            m_PlayerInteractionNoLosTimer.Invalidate();
            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
        }
    }
    else
    {
        if (!GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
        {
            GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
        }
    }
    
}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);

    if (!AtJumpSegment)
    {
        AIStateInterrupted();
        m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
    }
}

void ASDTAIController::MoveToBestFleeLocation()
{
	if (!loadBalancer->canExecute(lastUpdateFrame)) return;

	SelectBestFleeLocation();

    if (m_hasFleeLocation)
    {
        MoveToLocation(m_fleeLocation, 0.5f, false, true, false, NULL, false);
        OnMoveToTarget();
    }
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}

bool ASDTAIController::HasLoSOnHit(const FHitResult& hit)
{
    if (!hit.GetComponent())
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
    hitDirection.Normalize();

    FHitResult losHit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(hit.GetActor());

    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

    return losHit.GetActor() == nullptr;
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}

ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
{
    if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Collect)
    {
        if (!hit.GetComponent())
            return PlayerInteractionBehavior_Collect;

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))
            return PlayerInteractionBehavior_Collect;

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else
    {
        PlayerInteractionLoSUpdate();

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if(component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime)
{
    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit);

    if (currentBehavior != m_PlayerInteractionBehavior)
    {
        m_PlayerInteractionBehavior = currentBehavior;
        AIStateInterrupted();
    }
}

void ASDTAIController::StartBehaviorTree()
{
	ASoftDesignTrainingCharacter* charater = Cast<ASoftDesignTrainingCharacter>(GetPawn());
	UBehaviorTree* behaviorTree = charater->GetBehaviorTree();
	if (behaviorTree)
	{
		m_behaviorTreeComponent->StartTree(*behaviorTree);
	}
}

void ASDTAIController::Possess(APawn* pawn)
{
	Super::Possess(pawn);

	ASoftDesignTrainingCharacter* charater = Cast<ASoftDesignTrainingCharacter>(pawn);
	if (charater->GetBehaviorTree())
	{
		m_blackboardComponent->InitializeBlackboard(*charater->GetBehaviorTree()->BlackboardAsset);
		m_isPlayerSeenBBKeyID = m_blackboardComponent->GetKeyID("isPlayerSeen");
		//Set this agent in the BT
		m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_blackboardComponent->GetKeyID("SelfActor"), pawn);
	}
}

void ASDTAIController::TryDetectPlayer()
{
	if (!loadBalancer->canExecute(lastUpdateFrame)) return;

	double startTime = FPlatformTime::Seconds();

	m_IsPlayerDetected = false;

	//finish jump before updating AI state
	if (AtJumpSegment)
		return;

	APawn* selfPawn = GetPawn();
	if (!selfPawn)
		return;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
	FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

	TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
	detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

	TArray<FHitResult> allDetectionHits;
	GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

	FHitResult detectionHit;
	GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

	if (detectionHit.GetComponent() && (detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER) /*&& !HasLoSOnHit(detectionHit)*/)
		m_IsPlayerDetected = true;

	double timeTaken = FPlatformTime::Seconds() - startTime;

	// Draw time taken for 5 seconds
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 8.f), "player: " + FString::SanitizeFloat(timeTaken) + "s", GetPawn(), FColor::Orange, .5f, false);

}

void ASDTAIController::SelectBestFleeLocation()
{
	double startTime = FPlatformTime::Seconds();

	m_hasFleeLocation = false;

	float bestLocationScore = 0.f;
	ASDTFleeLocation* bestFleeLocation = nullptr;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!playerCharacter)
		return;

	for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
	{
		ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
		if (fleeLocation)
		{
			float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

			FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
			selfToPlayer.Normalize();

			FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
			selfToFleeLocation.Normalize();

			float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
			float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

			if (locationScore > bestLocationScore)
			{
				bestLocationScore = locationScore;
				bestFleeLocation = fleeLocation;
			}

			DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, .5f, false);

		}
	}

	if (bestFleeLocation != NULL)
	{
		m_hasFleeLocation = true;
		m_fleeLocation = bestFleeLocation->GetActorLocation();
	}

	double timeTaken = FPlatformTime::Seconds() - startTime;

	// Draw time taken for 5 seconds
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 7.f), "flee: " + FString::SanitizeFloat(timeTaken) + "s", GetPawn(), FColor::Purple, .5f, false);

}

void ASDTAIController::SelectRandomCollectible()
{
	double startTime = FPlatformTime::Seconds();

	m_hasCollectibleLocation = false;
	ASDTCollectible* collectible = NULL;
	
	TArray<AActor*> foundCollectibles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

	while (foundCollectibles.Num() != 0 && collectible == NULL)
	{
		int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

		ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
		if (!collectibleActor)
			return;

		if (!collectibleActor->IsOnCooldown())
		{
			collectible = collectibleActor;
		}
		else
		{
			foundCollectibles.RemoveAt(index);
		}
	}

	if (collectible != NULL)
	{
		m_hasCollectibleLocation = true;
		m_collectibleLocation = collectible->GetActorLocation();
	}

	double timeTaken = FPlatformTime::Seconds() - startTime;

	// Draw time taken for 5 seconds
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 6.f), "collectible: " + FString::SanitizeFloat(timeTaken) + "s", GetPawn(), FColor::Green, .5f, false);
}
