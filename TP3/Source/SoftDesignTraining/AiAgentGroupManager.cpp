// Fill out your copyright notice in the Description page of Project Settings.

#include "AiAgentGroupManager.h"
#include "SoftDesignTraining.h"

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
	m_registeredAgents.AddUnique(aiAgent);
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
	UpdatePlayerStatus(World);
}

FVector AiAgentGroupManager::GetTargetPos(UWorld* World, ASDTAIController* aiAgent)
{
	int32 index;
	m_registeredAgents.Find(aiAgent, index);

	//if (index == 0)
	//	return;

	float circle = 2.f * PI;
	int numberOfPoints = m_registeredAgents.Num() - 1;
	float angleBetweenPoints = circle / (float)numberOfPoints;

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	FVector playerPosition = playerCharacter->GetActorLocation();
	float radius = 250.f;

	FVector targetPosition = playerPosition;
	targetPosition.X += radius * cos(angleBetweenPoints * (index - 1));
	targetPosition.Y += radius * sin(angleBetweenPoints * (index - 1));

	DrawDebugSphere(World, targetPosition, 25.0f, 32, FColor::Red);

	return targetPosition;
}

int AiAgentGroupManager::GetIndex(ASDTAIController* aiAgent)
{
	int32 index;
	m_registeredAgents.Find(aiAgent, index);

	return index;
}