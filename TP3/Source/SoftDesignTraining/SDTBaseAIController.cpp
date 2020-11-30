// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTBaseAIController.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "DrawDebugHelpers.h"
#include "AAI_CPU_Manager.h"

ASDTBaseAIController::ASDTBaseAIController(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    m_ReachedTarget = true;

    m_behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    m_blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ASDTBaseAIController::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> groupCPU;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAAI_CPU_Manager::StaticClass(), groupCPU);

    ASDTAIController* agent = Cast<ASDTAIController>(this);
    AAAI_CPU_Manager* CPU_manager = Cast<AAAI_CPU_Manager>(groupCPU[0]);
    if (CPU_manager)
    {
        CPU_manager->RegisterAIAgent(agent);
    }
}


void ASDTBaseAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    UpdatePlayerInteraction(deltaTime);
	
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

void ASDTBaseAIController::StartBehaviorTree(APawn* pawn)
{
    if (ASDTBaseAIController* aiBaseCharacter = Cast<ASDTBaseAIController>(pawn->GetController()))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_behaviorTreeComponent->StartTree(*aiBaseCharacter->GetBehaviorTree());
        }
    }
}


void ASDTBaseAIController::OnPossess(APawn* pawn)
{
    Super::OnPossess(pawn);

    if (ASDTBaseAIController* aiBaseCharacter = Cast<ASDTBaseAIController>(pawn->GetController()))
    {
        if (aiBaseCharacter->GetBehaviorTree())
        {
            m_blackboardComponent->InitializeBlackboard(*aiBaseCharacter->GetBehaviorTree()->BlackboardAsset);

            m_targetPosBBKeyID = m_blackboardComponent->GetKeyID("PlayerPos");
            m_isPlayerSeenBBKeyID = m_blackboardComponent->GetKeyID("IsPlayerDetected");
            m_isAtJumpBBKeyID = m_blackboardComponent->GetKeyID("IsAtJump");
            m_isPlayerPoweredUpBBKeyID = m_blackboardComponent->GetKeyID("IsPlayerPoweredUp");
			m_isAgentInGroupBBKeyID = m_blackboardComponent->GetKeyID("IsAgentInGroup");
			m_isPlayerDetectedByGroupBBKeyID = m_blackboardComponent->GetKeyID("IsPlayerDetectedByGroup");

            //Set this agent in the BT
            m_blackboardComponent->SetValue<UBlackboardKeyType_Object>(m_blackboardComponent->GetKeyID("SelfActor"), pawn);
        }
    }
}
