// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "CollisionShape.h"
#include "PhysicsHelpers.h"
#include "SoftDesignTrainingMainCharacter.h"

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
		if (IsPlayerPoweredUp())//logic for checking player status
		{
			if (IsAgentHeadingTowardsPlayer())//logic to check if agent is walking towards the player
			{
				m_state = Stage::fleeState;//si l'agent se dirige vers le joeur en power-up, on le tourne
			}
			else
			{
				m_state = Stage::moveForwardState;//si l'agent si dirige déjà pour s'éloigner, on le laisse
			}
		}
		else
		{
			m_state = Stage::chaseState;
		}
		
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
			FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());
			MovePawn(FVector(toPlayer, 0.f), deltaTime);
		}
		break;
		case Stage::fleeState:
		{
			FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());
			MovePawn(-FVector(toPlayer, 0.f), deltaTime);
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
	if (GetWorld()->GetFirstPlayerController()->GetPawn() == nullptr)
	{
		return false;
	}
	else
	{
		bool isPlayerInRange = FVector2D(GetPawn()->GetActorLocation() - GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()).Size() <= m_visionRadius;
		
		FHitResult outHit;
		bool wallBetweenPlayerAndAgent = GetWorld()->LineTraceSingleByObjectType(outHit, GetPawn()->GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), ECC_WorldStatic);
		
		//debug
		if (isPlayerInRange) 
		{ 
			DrawDebugCircle(GetWorld(), GetPawn()->GetActorLocation(), m_visionRadius, 50, FColor::Green, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
			if (wallBetweenPlayerAndAgent)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetTargetLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(),700.f, FColor::Red, false, -1, 0, 10.f);
			}
			else
			{
				DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetTargetLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(),700.f, FColor::Emerald, false, -1, 0, 10.f);
			}
		}
		else 
		{ 
			DrawDebugCircle(GetWorld(), GetPawn()->GetActorLocation(), m_visionRadius, 50, FColor::Red, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		}

		return !wallBetweenPlayerAndAgent && isPlayerInRange;
	}
}

bool ASDTAIController::IsPlayerPoweredUp()
{
	
	ASoftDesignTrainingMainCharacter* player = dynamic_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetPawn());
	return player->IsPoweredUp();
}
bool ASDTAIController::IsAgentHeadingTowardsPlayer()
{
	float theta = 30.f/180.f*PI;//angle à chaque côté de la direction forward qui, lors de la détection d'un joueur powered-up dans la zone, tourne l'agent pour fuire
	//debug
	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) + GetPawn()->GetActorRightVector()*sin(theta))*600.f, FColor::Orange, false, -1.f, 0, 10.f);
	DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) - GetPawn()->GetActorRightVector()*sin(theta))*600.f, FColor::Orange, false, -1.f, 0, 10.f);

	//logic
	FVector2D currentDirection = FVector2D(GetPawn()->GetActorForwardVector());
	FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
	return FVector2D::DotProduct(currentDirection, toPlayer) > cos(theta);
}
bool ASDTAIController::IsBallDetected()
{
	return false;
}

void ASDTAIController::MovePawn(FVector direction, float deltaTime)
{
	m_currentSpeed += m_acceleration*deltaTime;
	float scaleValue = m_currentSpeed / m_maxSpeed;
	GetPawn()->AddMovementInput(direction, FMath::Min(scaleValue, 1.f));//le 2e parametre de addMovementInput est une constante entre -1 et 1 utilisé pour scale la vitesse CharacterMouvementComponent
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