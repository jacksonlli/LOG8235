// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
// AJOUTS
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"
// Generated
#include "SDTBaseAIController.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API ASDTBaseAIController : public AAIController
{
	GENERATED_BODY()

public:

    ASDTBaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    virtual void Tick(float deltaTime) override;
	
protected:
    virtual void RotationUpdate(float deltaTime) {};
    virtual void ImpulseToDirection(float deltaTime) {};

    bool m_ReachedTarget;
private:
    virtual void GoToBestTarget(float deltaTime) {};
    virtual void UpdatePlayerInteraction(float deltaTime) {};
    virtual void ShowNavigationPath() {};

// AJOUTS
public:
    UBehaviorTree* GetBehaviorTree() const { return m_aiBehaviorTree; }

    UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return m_behaviorTreeComponent; }
    UBlackboardComponent*   GetBlackBoardComponent() const { return m_blackboardComponent; }

    void                    StartBehaviorTree(APawn* pawn);

    uint8   m_targetPosBBKeyID;
    uint8   m_isPlayerSeenBBKeyID;
    uint8   m_isAtJumpBBKeyID;
    uint8   m_isPlayerPoweredUpBBKeyID;

    uint8           GetTargetPosBBKeyID() const { return m_targetPosBBKeyID; }
    uint8           GetTargetSeenBBKeyID() const { return m_isPlayerSeenBBKeyID; }
    uint8           GetIsAtJumpBBKeyID() const { return m_isAtJumpBBKeyID; }
    uint8           GetIsPlayerPoweredUpBBKeyID() const { return m_isPlayerPoweredUpBBKeyID; }

protected:
    virtual void OnPossess(APawn* pawn) override;

    UPROPERTY(EditAnywhere, category = Behavior)
        UBehaviorTree* m_aiBehaviorTree;

private:
    UPROPERTY(transient)
        UBehaviorTreeComponent* m_behaviorTreeComponent;

    UPROPERTY(transient)
        UBlackboardComponent* m_blackboardComponent;
};