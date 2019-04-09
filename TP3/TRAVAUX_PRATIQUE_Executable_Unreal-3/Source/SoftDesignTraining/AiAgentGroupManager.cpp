// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "AiAgentGroupManager.h"
#include "DrawDebugHelpers.h"
AiAgentGroupManager* AiAgentGroupManager::m_Instance;

AiAgentGroupManager::AiAgentGroupManager()
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
    m_registeredAgents.Add(aiAgent);
}

void AiAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
}


void AiAgentGroupManager::DrawSphereOverHead() {
	for (int i = 0; i < m_registeredAgents.Num(); i++) {
		AAIController* aicontroller = m_registeredAgents[i];
		FVector actorLocation = aicontroller->GetPawn()->GetActorLocation();
		DrawDebugSphere(aicontroller->GetWorld(), actorLocation + FVector(0.f, 0.f, 100.f), 15.0f, 32, FColor::Purple);
	}
	
}

/**TargetLKPInfo AiAgentGroupManager::GetLKPFromGroup(const FString& targetLabel,bool& targetfound)
{
    int agentCount = m_registeredAgents.Num();
    TargetLKPInfo outLKPInfo = TargetLKPInfo();
    
    return outLKPInfo;
}**/