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

bool AiAgentGroupManager::IsPlayerSeenByGroup()
{
	int agentCount = m_registeredAgents.Num();
	for (int i = 0; i < agentCount; ++i)
	{
		ASDTAIController* aiAgent = m_registeredAgents[i];
		if (aiAgent)
		{
			if (aiAgent->IsTargetPlayerSeen())
			{
				return true;
			}
		}

	}
	return false;
}
//TargetLKPInfo AiAgentGroupManager::GetLKPFromGroup(const FString& targetLabel, bool& targetfound)
//{
//	int agentCount = m_registeredAgents.Num();
//	TargetLKPInfo outLKPInfo = TargetLKPInfo();
//	targetfound = false;
//
//	for (int i = 0; i < agentCount; ++i)
//	{
//		AAIBase* aiAgent = m_registeredAgents[i];
//		if (aiAgent)
//		{
//			const TargetLKPInfo& targetLKPInfo = aiAgent->GetCurrentTargetLKPInfo();
//
//				if (targetLKPInfo.GetLastUpdatedTimeStamp() > outLKPInfo.GetLastUpdatedTimeStamp())
//				{
//					targetfound = targetLKPInfo.GetLKPState() != TargetLKPInfo::ELKPState::LKPState_Invalid;
//					outLKPInfo = targetLKPInfo;
//				}
//			
//		}
//	}
//
//	return outLKPInfo;
//}