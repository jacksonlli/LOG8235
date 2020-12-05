// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "AIController.h"

#include "AAI_CPU_Manager.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API AAAI_CPU_Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAAI_CPU_Manager();

	void RegisterAIAgent(ASDTAIController* aiAgent);
	void UnregisterAIAgents();

private:

	TArray<ASDTAIController*> m_registeredAgents;
	int					m_nextAgent = 0; // Next agent to update
	int					ShouldPrint = 0; // Which group of agent will be printed this frame

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
