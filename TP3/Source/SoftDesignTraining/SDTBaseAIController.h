// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
// AJOUTS
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
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UBehaviorTree* GetBehaviorTree() const { return m_aiBehaviorTree; }

    UBehaviorTreeComponent* GetBehaviorTreeComponent() const { return m_behaviorTreeComponent; }
    UBlackboardComponent* GetBlackBoardComponent() const { return m_blackboardComponent; }

    void                    StartBehaviorTree(APawn* pawn);
    void                    StopBehaviorTree(APawn* pawn);

protected:
    UPROPERTY(EditAnywhere, category = Behavior)
        UBehaviorTree* m_aiBehaviorTree;

private:
    UPROPERTY(transient)
        UBehaviorTreeComponent* m_behaviorTreeComponent;

    UPROPERTY(transient)
        UBlackboardComponent* m_blackboardComponent;
};
