// Fill out your copyright notice in the Description page of Project Settings.

#include "AiAgentGroupManager.h"
#include "SoftDesignTraining.h"

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
	aiAgent->setAgentGroupStatus(true);
}

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

bool AiAgentGroupManager::IsAIAgentInGroup(ASDTAIController* aiAgent)//ne plus utiliser -> utiliser IsAgentInGroup() de SDTAIcontroller de l'agent a la place
{
	return m_registeredAgents.Contains(aiAgent);
}

void AiAgentGroupManager::UpdatePlayerStatus(UWorld* World)
{
	//if grace period elasped without seeing player, return player not detected
	if (UGameplayStatics::GetRealTimeSeconds(World) > m_lastUpdatedTimeStamp + m_MaxTimeElapsedSinceSpottedPlayer)
	{
		m_isPlayerDetectedbyGroup = false;
	}
	else
	{
		m_isPlayerDetectedbyGroup = true;
	}
}

void AiAgentGroupManager::UpdateTimeStamp(UWorld* World)
{
	m_lastUpdatedTimeStamp = UGameplayStatics::GetRealTimeSeconds(World);//update when player is seen
}