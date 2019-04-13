// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "AiAgentGroupManager.h"
#include "DrawDebugHelpers.h"
#include <cmath>
#include "SoftDesignTrainingMainCharacter.h"

AiAgentGroupManager* AiAgentGroupManager::m_Instance;

AiAgentGroupManager::AiAgentGroupManager() : m_numberOfAnchorPoints(0)
{
}

AiAgentGroupManager* AiAgentGroupManager::GetInstance()
{
    if (!m_Instance)
    {
        m_Instance = new AiAgentGroupManager();
    }

    return m_Instance;
}

void AiAgentGroupManager::Destroy()
{
    delete m_Instance;
    m_Instance = nullptr;
}

void AiAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
	m_registeredAgents.AddUnique(aiAgent);
}

void AiAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
}


void AiAgentGroupManager::DrawSphereOverHead() 
{
	for (int i = 0; i < m_registeredAgents.Num(); i++) 
	{
		AAIController* aicontroller = m_registeredAgents[i];
		int c = 0;
		if (aicontroller) 
		{
			FVector actorLocation = aicontroller->GetPawn()->GetActorLocation();
			DrawDebugSphere(aicontroller->GetWorld(), actorLocation + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Purple);
		}
		else 
		{
			c++;
		}
	}
	
}

void AiAgentGroupManager::GenerateAnchorPoints()
{
	if (m_registeredAgents.Num() < 1) return;

	if (lastAnchorPointUpdateFrame == GFrameCounter) return;

	lastAnchorPointUpdateFrame = GFrameCounter;

	m_anchorPoints.Empty();
	float circle = 2.f * PI;
	m_numberOfAnchorPoints = (m_registeredAgents.Num() < 8)? m_registeredAgents.Num() : 8;
	float angle = circle / (float)m_numberOfAnchorPoints;
	AAIController* aicontroller = m_registeredAgents[0];
	ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(aicontroller->GetWorld(), 0);
	FVector playerPosition = playerCharacter->GetActorLocation();
	float rayon = 200.f;
	float currentAngle = 0.f;
	
	for (int i = 0; i < m_numberOfAnchorPoints; i++) // 8 points arround the player are enough tu surround him.
	{
		FVector newPosition = playerPosition;
		newPosition.X += rayon * cos(currentAngle);
		newPosition.Y += rayon * sin(currentAngle);

		m_anchorPoints.Add(newPosition);
		currentAngle += angle;
		DrawDebugSphere(aicontroller->GetWorld(), newPosition + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Red);
	}
	AssignPointsToAI();
}

void AiAgentGroupManager::AssignPointsToAI()
{
	for (int i = 0; i < m_registeredAgents.Num(); i++)
	{
		m_registeredAgents[i]->SetAnchorPoint(m_anchorPoints[i%m_numberOfAnchorPoints]);
	}
}