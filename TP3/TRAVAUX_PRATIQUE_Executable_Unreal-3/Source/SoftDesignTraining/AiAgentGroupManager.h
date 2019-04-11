#pragma once
//#include "TargetLKPInfo.h"
#include "SDTAIController.h"
#include "CoreMinimal.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API AiAgentGroupManager
{
public:   
    static AiAgentGroupManager* GetInstance();
    static void Destroy();

    void RegisterAIAgent(ASDTAIController* aiAgent);
    void UnregisterAIAgent(ASDTAIController* aiAgent);
	void DrawSphereOverHead();
	void GenerateAnchorPoints();

private:

    //SINGLETON
    AiAgentGroupManager();
	void AssignPointsToAI();

    static AiAgentGroupManager* m_Instance;
	TArray<FVector> m_anchorPoints;
    TArray<ASDTAIController*> m_registeredAgents;
};
