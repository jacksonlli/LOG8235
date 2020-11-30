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
	bool IsPlayerDetectedByGroup() { return m_isPlayerDetectedbyGroup; }

private:

	TArray<ASDTAIController*> m_registeredAgents;
	int					m_nextAgent = 0;
	bool				m_isPlayerDetectedbyGroup = false;
	int					ShouldPrint = 10;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
