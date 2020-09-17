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
	void MovePawn(float deltaTime, FVector direction);

private:
	float m_currentSpeed = 0.f;
	float m_maxSpeed = 1000.f;
	float m_acceleration = 200.f;
};
