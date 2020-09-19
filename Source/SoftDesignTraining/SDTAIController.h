// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
	bool IsInCollisionWithWall();
	bool IsTrapInTrajectory();
	bool IsPlayerDetected();
	bool IsPlayerPoweredUp();
	bool IsBallDetected();
	void MovePawn(FVector direction, float deltaTime);

private:
	float m_currentSpeed = 0.f;
	float m_maxSpeed = 1000.f;
	float m_acceleration = 200.f;
	//STATE INDEXES
	const int moveForwardState = 0;
	const int chaseState = 1;
	const int fleeState = 2;
	//DEFAULT CURRENT STATE
	int m_state = moveForwardState;

};
