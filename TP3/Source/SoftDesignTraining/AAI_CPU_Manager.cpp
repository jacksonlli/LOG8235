// Fill out your copyright notice in the Description page of Project Settings.


#include "AAI_CPU_Manager.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTBaseAIController.h"

// Sets default values
AAAI_CPU_Manager::AAAI_CPU_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

}

// Called when the game starts or when spawned
void AAAI_CPU_Manager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAAI_CPU_Manager::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	double start = FPlatformTime::Seconds();

	int agentCount = m_registeredAgents.Num();
	// DEBUG
	int nb_agents_maj = 0;

	if (agentCount > 0)
	{
		if (ShouldPrint == 7)
		{
			ShouldPrint = 0;
		}
		for (int i = 8*ShouldPrint; i < 8*ShouldPrint+7; ++i)
		{
			if (i < m_registeredAgents.Num())
			{
				ASDTAIController* agent = m_registeredAgents[i];
				agent->PrintSelf(7 * deltaTime);
			}
		}
		ShouldPrint = ShouldPrint + 1;
		while (FPlatformTime::Seconds() - start < 0.01 && nb_agents_maj < m_registeredAgents.Num())
		{
			if (m_nextAgent < m_registeredAgents.Num())
			{
				ASDTAIController* nextAgent = m_registeredAgents[m_nextAgent];
				m_nextAgent = m_nextAgent + 1;
				// DEBUG
				nb_agents_maj = nb_agents_maj + 1;

				if (m_nextAgent > m_registeredAgents.Num()-1)
				{
					m_nextAgent = m_nextAgent - m_registeredAgents.Num();
				}
				nextAgent->Tick(deltaTime);
			}
		}
		// DEBUG
		GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Green, FString::Printf(TEXT("Nb agents mis à jour sur la frame %i"), nb_agents_maj));
	}
}

void AAAI_CPU_Manager::RegisterAIAgent(ASDTAIController* aiAgent)
{
	m_registeredAgents.Add(aiAgent);
}

void AAAI_CPU_Manager::UnregisterAIAgents()
{
	int agentCount = m_registeredAgents.Num();
	for (int i = 0; i < agentCount; ++i)
	{
		ASDTAIController* aiAgent = m_registeredAgents[i];
	}
	m_registeredAgents.Empty();
}
