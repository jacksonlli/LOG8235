// Fill out your copyright notice in the Description page of Project Settings.

#include "AiAgentGroupManager.h"
#include "SoftDesignTraining.h"

AiAgentGroupManager* AiAgentGroupManager::m_Instance;

AiAgentGroupManager::AiAgentGroupManager()
{
}

//obtient le singleton du groupe
AiAgentGroupManager* AiAgentGroupManager::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new AiAgentGroupManager();
	}

	return m_Instance;
}

//détruit le groupe
void AiAgentGroupManager::Destroy()
{
	delete m_Instance;
	m_Instance = nullptr;
}

//ajout d'un agent dans le groupe lorsqu'il voit le joueur
void AiAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
	m_registeredAgents.Add(aiAgent);
	aiAgent->setAgentGroupStatus(true);
}

//vider le groupe de tous ses agents lorsque le joueur est vu en mode PoweredUp ou lorsqu'aucun des agents ne voient le joueur
void AiAgentGroupManager::UnregisterAIAgents()
{
	int agentCount = m_registeredAgents.Num();
	for (int i = 0; i < agentCount; ++i)
	{
		ASDTAIController* aiAgent = m_registeredAgents[i];
		if (aiAgent)
		{
			aiAgent->setAgentGroupStatus(false);
		}
	}
	m_registeredAgents.Empty();
}

//mis a jour de la variable m_isPlayerDetected 
void AiAgentGroupManager::UpdatePlayerStatus(UWorld* World)
{
	//si le joueur n'est pas vu pour une durée plus long que m_MaxTimeElapsedSinceSpottedPlayer(1 seconde par defaut), le joueur n'est plus detecté par le groupe
	if (UGameplayStatics::GetRealTimeSeconds(World) > m_lastUpdatedTimeStamp + m_MaxTimeElapsedSinceSpottedPlayer)
	{
		m_isPlayerDetectedbyGroup = false;
	}
	else
	{
		m_isPlayerDetectedbyGroup = true;
	}
}

//mis a jour du timeStamp et du status du joueur quand le joueur est vu
void AiAgentGroupManager::UpdateTimeStamp(UWorld* World)
{
	m_lastUpdatedTimeStamp = UGameplayStatics::GetRealTimeSeconds(World);//update when player is seen
	UpdatePlayerStatus(World);
}