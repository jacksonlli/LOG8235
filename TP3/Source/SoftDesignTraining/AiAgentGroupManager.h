#pragma once
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
    void UnregisterAIAgents();
	bool IsAIAgentInGroup(ASDTAIController* aiAgent);
	void UpdatePlayerStatus(UWorld* World);
	bool IsPlayerDetectedByGroup() { return m_isPlayerDetectedbyGroup; }
	void UpdateTimeStamp(UWorld* World);

private:

    //SINGLETON
    AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;

    TArray<ASDTAIController*> m_registeredAgents;
	float	  m_MaxTimeElapsedSinceSpottedPlayer = 2;//seconds
	float     m_lastUpdatedTimeStamp = 0;
	bool	  m_isPlayerDetectedbyGroup = false;
};
