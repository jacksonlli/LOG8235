// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTBaseAIController.h"
#include "SoftDesignTraining.h"

ASDTBaseAIController::ASDTBaseAIController(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    m_ReachedTarget = true;
}

void ASDTBaseAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    UpdatePlayerInteraction(deltaTime); // to be replaced with BT logic

    if (m_ReachedTarget)
    {
        GoToBestTarget(deltaTime);
    }
    else
    {
        ShowNavigationPath();
    }
}

// AJOUTS

// Called when the game starts or when spawned
void ASDTBaseAIController::BeginPlay()
{
    Super::BeginPlay();

    StartBehaviorTree(GetPawn());
}

void ASDTBaseAIController::StartBehaviorTree(APawn* pawn)
{
    if (ASDTBaseAIController* aiBaseCharacter = Cast<ASDTBaseAIController>(pawn))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_behaviorTreeComponent->StartTree(*aiBaseCharacter->GetBehaviorTree());
        }
    }
}

void ASDTBaseAIController::StopBehaviorTree(APawn* pawn)
{
    if (ASDTBaseAIController* aiBaseCharacter = Cast<ASDTBaseAIController>(pawn))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_behaviorTreeComponent->StopTree();
        }
    }
}


