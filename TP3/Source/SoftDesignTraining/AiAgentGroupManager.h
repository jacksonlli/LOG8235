#pragma once
#include "SDTAIController.h"
#include "CoreMinimal.h"

#include "SoftDesignTraining.h"

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
	void UpdatePlayerStatus(UWorld* World);
	bool IsPlayerDetectedByGroup() { return m_isPlayerDetectedbyGroup; }
	void UpdateTimeStamp(UWorld* World);

    void GetTargetPos(UWorld* World);

    //void GetTargetPos(UWorld* World, ASDTAIController* aiAgent);
    FVector GetTargetPos(UWorld* World, ASDTAIController* aiAgent);

    int GetIndex(ASDTAIController* aiAgent);

private:

    //SINGLETON
    AiAgentGroupManager();
    static AiAgentGroupManager* m_Instance;

    TArray<ASDTAIController*> m_registeredAgents;
	float	  m_MaxTimeElapsedSinceSpottedPlayer = 1;//seconds
	float     m_lastUpdatedTimeStamp = 0;
	bool	  m_isPlayerDetectedbyGroup = false;
};
