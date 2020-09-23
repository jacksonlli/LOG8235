// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "CollisionShape.h"
#include "PhysicsHelpers.h"

void ASDTAIController::Tick(float deltaTime)
{
	//logic to assign state
	if (IsInCollisionWithWall())//logic for detecting if in collision with wall
	{
		m_state = Stage::avoidObstacleState;
	}
	else if (IsTrapInTrajectory())//logic for detecting if a death trap is in the current trajectory
	{

	}
	else if (IsPlayerDetected())//logic for spotting player
	{
		m_state = Stage::chaseState;
	}
	else if (IsBallDetected())//logic for spotting power-up balls
	{

	}
	else 
	{
		m_state = Stage::moveForwardState;
	}


	//apply state
	switch (m_state)
	{
		case Stage::moveForwardState:
		{
			MovePawn(GetPawn()->GetActorForwardVector(), deltaTime);
		}
		break;
		case Stage::chaseState:
		{

		}
		break;
		case Stage::fleeState:
		{

		}
		break;
		case Stage::avoidObstacleState:
		{
			AvoidWall(deltaTime);
		}
		break;
	}
}

//Helper Functions

bool ASDTAIController::IsInCollisionWithWall()
{
	UWorld* World = GetWorld();
	APawn* pawn = GetPawn();
	TArray<FHitResult> outHits;

	// Sans PhysicsHelpers (reproduit selon les besoins)
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);
	objectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel5);

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	FCollisionShape collisionShape;
	collisionShape.SetCapsule(pawn->GetSimpleCollisionRadius(), pawn->GetSimpleCollisionHalfHeight());

	//DrawDebugSphere(World, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, pawn->GetSimpleCollisionRadius(), 25, FColor::Green, false, -1.0f, 0, 5.0f);
	DrawDebugCapsule(World, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, pawn->GetSimpleCollisionHalfHeight(), pawn->GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, false, -1.0f, 0, 5.0f);

	return World->SweepMultiByObjectType(outHits, pawn->GetActorLocation(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, FQuat::Identity, objectQueryParams, collisionShape, queryParams);
	
	// Par PhysicsHelpers
	/*DrawDebugDirectionalArrow(World, pawn->GetActorLocation(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, 500.0f, FColor::Blue, false, -1.0f, 000, 10.0f);
	PhysicsHelpers physicsHelper(World);
	return physicsHelper.SphereCast(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 2.1 * pawn->GetSimpleCollisionRadius(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, pawn->GetSimpleCollisionRadius(), outHits, true);
	*/
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
	GetPawn()->AddMovementInput(direction, FMath::Min(m_currentSpeed, m_maxSpeed));
	GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}

void ASDTAIController::AvoidWall(float deltaTime)
{
	UWorld* World = GetWorld();
	APawn* pawn = GetPawn();
	TArray<FHitResult> outHits;

	// Sans PhysicsHelpers (reproduit selon les besoins)
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);
	objectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel5);
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	FCollisionShape collisionShape;
	collisionShape.SetCapsule(pawn->GetSimpleCollisionRadius(), pawn->GetSimpleCollisionHalfHeight());
	float const castDist = 200.0f;

	World->SweepMultiByObjectType(outHits, pawn->GetActorLocation(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * castDist, FQuat::Identity, objectQueryParams, collisionShape, queryParams);

	// Debug drawing
	for (int32 i = 0; i < outHits.Num(); ++i)
		DrawDebugPoint(World, outHits[i].ImpactPoint, 7.0f, FColor::Red, false, 1.0f, 0);

	// Avec PhysicsHelpers
	/*
	PhysicsHelpers physicsHelper(World);
	physicsHelper.SphereCast(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 2.1 * pawn->GetSimpleCollisionRadius(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, pawn->GetSimpleCollisionRadius(), outHits, true);
	*/

	// Calcul du nouveau vecteur de direction (faire un coefficient inv proportionnel a la distance au pawn)
	FHitResult blockingObstacle = outHits[0];
	FVector contactDirection;
	if (FVector::DotProduct((FVector::CrossProduct(FVector::UpVector, blockingObstacle.ImpactNormal.GetSafeNormal())), pawn->GetActorForwardVector()) > 0)
	{
		contactDirection = FVector::CrossProduct(FVector::UpVector, blockingObstacle.ImpactNormal.GetSafeNormal());
	}
	else
	{
		contactDirection = -FVector::CrossProduct(FVector::UpVector, blockingObstacle.ImpactNormal.GetSafeNormal());
	}

	//float const coeffEvitement = 1.0f / (1.0f + FMath::Max(blockingObstacle.Distance - pawn->GetSimpleCollisionRadius(), 0.0f));
	float const coeffEvitement = FMath::Max(blockingObstacle.Distance - (pawn->GetSimpleCollisionRadius()), 0.0f) / (castDist - (pawn->GetSimpleCollisionRadius()));
	FVector const displacementDirection = pawn->GetActorForwardVector() * coeffEvitement + contactDirection * (1 - coeffEvitement);

	//FVector const contactDirection = pawn->GetActorRightVector();
	MovePawn(displacementDirection.GetSafeNormal(), deltaTime);
}