// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"

void ASDTAIController::Tick(float deltaTime)
{
	//logic to assign state
	if (IsInCollisionWithWall())//logic for detecting if in collision with wall
	{

	}
	else if (IsTrapInTrajectory())//logic for detecting if a death trap is in the current trajectory
	{

	}
	else if (IsPlayerDetected())//logic for spotting player
	{
		m_state = chaseState;
	}
	else if (IsBallDetected())//logic for spotting power-up balls
	{

	}
	else 
	{
		m_state = moveForwardState;
	}


	//apply state
	if (m_state == moveForwardState)
	{
		MovePawn(GetPawn()->GetActorForwardVector(), deltaTime);
	}
	else if (m_state == chaseState)
	{

	}
	else if (m_state == fleeState)
	{

	}
}

//Helper Functions

bool ASDTAIController::IsInCollisionWithWall()
{
	return false;
}

bool ASDTAIController::IsTrapInTrajectory()
{
	return false;
}

bool ASDTAIController::IsPlayerDetected()
{
	return false;
}

bool ASDTAIController::IsPlayerPoweredUp()
{
	return false;
}

bool ASDTAIController::IsBallDetected()
{
	return false;
}

void ASDTAIController::MovePawn(FVector direction, float deltaTime)
{
	m_currentSpeed += m_acceleration*deltaTime;
	GetPawn()->AddMovementInput(direction, m_currentSpeed/m_maxSpeed);
	GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}


