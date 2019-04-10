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
    //TargetLKPInfo GetLKPFromGroup(const FString& targetLabel, bool& targetFound);

private:

    //SINGLETON
    AiAgentGroupManager();
	void AssignPointsToAI();

	//~AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;
	TArray<FVector> m_anchorPoints;
    TArray<ASDTAIController*> m_registeredAgents;
	

};
