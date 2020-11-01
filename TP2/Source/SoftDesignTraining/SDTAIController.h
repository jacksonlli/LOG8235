// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "SDTAIController.generated.h"

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
		float CurrentSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float RotationRate = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float MaxSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		float Acceleration = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
		bool IsWalking = true;

public:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	void AIStateInterrupted();

protected:
	void OnMoveToTarget();
	void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
	void UpdatePlayerInteraction(float deltaTime);

	enum class AIState
	{
		Pursue,
		Flee,
		GoToClosestPickup
	};
	AIState m_state = AIState::GoToClosestPickup;
	void GoToClosestPickup(float deltaTime);

	TArray<FNavPathPoint> PathToFollow;
	int32           CurrentDestinationIndex = 0;

	FVector ComputeDestination(float DeltaTime);
	FVector ComputeVelocity(float DeltaTime, FVector destination);
	float   ComputeTargetSpeedForTurn();

	FVector Direction = FVector(0.f, 1.f, 0.f);
	
	bool    IsTurning = false;
	bool    ShouldRePath = true;
	float   SlowDownTargetSpeed = -1.f;
	FVector SlowDownStartPos = FVector::ZeroVector;
	int32   IndexAfterSlowDown = -1;
	float   BaseHeight = 0.f;

	void    UpdateDirection(float deltaTime, FVector directionGoal);
	void    ApplyVelocity(float DeltaTime, FVector velocity);
	void    MoveTowardsDirection(float deltaTime);
	void    ComputePath(UNavigationPath* path, float deltaTime);
	bool	IsPlayerDetected();
	UNavigationPath * GetPathToFleeLocation();
	void    UseIntermediaryDestination_Behavior(FVector2D position2D, FVector2D destination2D, FVector& destination);
	void    StartJump(float deltaTime);
	void    UpdateJump(float curveTime, float deltaTime);

private:
	virtual void GoToBestTarget(float deltaTime) override;
	virtual void ChooseBehavior(float deltaTime) override;
	virtual void ShowNavigationPath() override;

};
