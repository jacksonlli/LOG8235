// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"

void ASDTAIController::Tick(float deltaTime)
{
	MovePawn(deltaTime, GetPawn()->GetActorRightVector());
}

void ASDTAIController::MovePawn(float deltaTime, FVector direction)
{
	currentSpeed += m_acceleration*deltaTime;
	GetPawn()->AddMovementInput(direction, m_currentSpeed/m_maxSpeed);
}


