// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "SDTAIController.generated.h"


UENUM(BlueprintType)
enum class EAIBrainMode : uint8
{
    IfElse          UMETA(DisplayName = "IfElse_Logic"),
    BehaviorTree 	UMETA(DisplayName = "BT_Logic")
};

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat* JumpCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool Landing = false;

public:

    enum PlayerInteractionBehavior
    {
        PlayerInteractionBehavior_Collect,
        PlayerInteractionBehavior_Chase,
        PlayerInteractionBehavior_Flee
    };

protected:
    void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
    void UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime);
    PlayerInteractionBehavior GetCurrentPlayerInteractionBehavior(const FHitResult& hit);
    bool HasLoSOnHit(const FHitResult& hit);
    void MoveToRandomCollectible();
    void MoveToPlayer();
    void MoveToBestFleeLocation();
    void PlayerInteractionLoSUpdate();
    void OnPlayerInteractionNoLosDone();
    void OnMoveToTarget();

public:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void RotateTowards(const FVector& targetLocation);
    void SetActorLocation(const FVector& targetLocation);
    void AIStateInterrupted();
	void PrintSelf(float freqUpdate);

private:
    virtual void GoToBestTarget(float deltaTime) override;
    virtual void UpdatePlayerInteraction(float deltaTime) override;
    virtual void ShowNavigationPath() override;


protected:
    FVector m_JumpTarget;
    FRotator m_ObstacleAvoidanceRotation;
    FTimerHandle m_PlayerInteractionNoLosTimer;
    PlayerInteractionBehavior m_PlayerInteractionBehavior;


// AJOUTS
private:
    //Player detection
    FVector m_playerPos;
    bool m_isPlayerDetected;
	bool m_inGroup = false;

    virtual void    ClearTimer();
    virtual void    UpdateTimer();

public:
    FVector         GetTargetPlayerPos() const { return m_playerPos; }
    bool            IsTargetPlayerSeen() const { return m_isPlayerDetected; }
    bool            IsAtJump() const { return AtJumpSegment; }
	bool			IsAgentInGroup() const { return m_inGroup; }
	void			setAgentGroupStatus(bool status) { m_inGroup = status; }
	
	
	bool			IsPlayerDetectedByGroup();
	virtual void    GroupDetectPlayer();
    virtual void    DetectPlayer();
    virtual void    GetDetectionHits(TArray<FHitResult>& allDetectionHits, FHitResult& detectionHit);
    virtual void    SetBehavior(ASDTAIController::PlayerInteractionBehavior currentBehavior);
	virtual void	EmptyGroup();
	virtual void	RegisterAgent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision_Logic")
        EAIBrainMode          m_currentBrainLogic = EAIBrainMode::BehaviorTree;
};
