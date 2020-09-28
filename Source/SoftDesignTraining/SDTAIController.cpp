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
	if (IsWallOrTrapInTrajectory())//logic for detecting if in collision with wall
	{
		m_state = Stage::avoidObstacleState;
	}
	else if (IsPlayerDetected())//logic for spotting player
	{
		choosen_side = 0;
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
		choosen_side = 0;	// côté duquel le pawn va tourner face à un obstacle
	}
	else
	{
		choosen_side = 0;
		m_state = Stage::moveForwardState;
	}


	//apply state
	switch (m_state)
	{
	case Stage::moveForwardState:
	{
		// Vecteurs unitaires haut/droite/bas/gauche du repère global
		FVector2D WorldVectors[4];
		WorldVectors[0] = FVector2D(0, 1);
		WorldVectors[1] = FVector2D(1, 0);
		WorldVectors[2] = FVector2D(0, -1);
		WorldVectors[3] = FVector2D(-1, 0);

		// Choix du vecteur global le plus proche du vecteur vitesse actuel
		float dotScore = -100000.0f;
		FVector2D bestWorldVector;

		for (int32 i = 0; i < 4; i++)
		{
			if (FVector2D::DotProduct(FVector2D(GetPawn()->GetActorForwardVector().GetSafeNormal()), WorldVectors[i]) > dotScore)
			{
				dotScore = FVector2D::DotProduct(FVector2D(GetPawn()->GetActorForwardVector().GetSafeNormal()), WorldVectors[i]);
				bestWorldVector = WorldVectors[i];
			}
		}
		// Tourner le pawn pour qu'il s'aligne progressivement sur un vecteur global
		MovePawn(GetPawn()->GetActorForwardVector().GetSafeNormal() * 0.9f + FVector(bestWorldVector * 0.1f, 0.0f), deltaTime);
	}
	break;
	case Stage::chaseState:
	{
		// Déplacement orienté vers le joueur
		FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());
		MovePawn(FVector(toPlayer, 0.f), deltaTime);
	}
	break;
	case Stage::fleeState:
	{
		// Déplacement orienté dans la direction opposée au joueur
		FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());
		MovePawn(-FVector(toPlayer, 0.f), deltaTime);
	}
	break;
	case Stage::avoidObstacleState:
	{
		// Si aucun choix n'a déjà été fait, choisir un côté
		if (choosen_side == 0)
			ChooseSide(deltaTime);
		AvoidWall(deltaTime);	// Fonction d'évitement
	}
	break;
	}
}

//Helper Functions

bool ASDTAIController::IsWallOrTrapInTrajectory() // Renvoie True si un mur ou un piège est détecté devant le pawn à une distance de 200 ou moins
{
	UWorld* World = GetWorld();
	APawn* pawn = GetPawn();
	TArray<FHitResult> outHits;

	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);	// ajout des murs aux objets détectés
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);	// ajout des pièges
	objectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel5);	// retrait des Collectibles

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;	// paramètres par défaut
	FCollisionShape collisionShape;
	collisionShape.SetCapsule(pawn->GetSimpleCollisionRadius(), pawn->GetSimpleCollisionHalfHeight());	// Capsule de collision du pawn

	// Debug
	//DrawDebugCapsule(World, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 300.0f, pawn->GetSimpleCollisionHalfHeight(), pawn->GetSimpleCollisionRadius(), FQuat::Identity, FColor::Green, false, -1.0f, 0, 5.0f);

	return World->SweepMultiByObjectType(outHits, pawn->GetActorLocation(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.0f, FQuat::Identity, objectQueryParams, collisionShape, queryParams);
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
		/*if (isPlayerInRange)
		{
			DrawDebugCircle(GetWorld(), GetPawn()->GetActorLocation(), m_visionRadius, 50, FColor::Green, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
			if (wallBetweenPlayerAndAgent)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetTargetLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), 700.f, FColor::Red, false, -1, 0, 10.f);
			}
			else
			{
				DrawDebugDirectionalArrow(GetWorld(), GetPawn()->GetTargetLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), 700.f, FColor::Emerald, false, -1, 0, 10.f);
			}
		}
		else
		{
			DrawDebugCircle(GetWorld(), GetPawn()->GetActorLocation(), m_visionRadius, 50, FColor::Red, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
		}
		*/

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
	float theta = 30.f / 180.f*PI;//angle à chaque côté de la direction forward qui, lors de la détection d'un joueur powered-up dans la zone, tourne l'agent pour fuire
	//debug
	//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) + GetPawn()->GetActorRightVector()*sin(theta))*m_visionRadius, FColor::Orange, false, -1.f, 0, 10.f);
	//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) - GetPawn()->GetActorRightVector()*sin(theta))*m_visionRadius, FColor::Orange, false, -1.f, 0, 10.f);

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
	//voir definitions de m_currentSpeed, m_maxSpeed et m_acceleration dans SDTAIController.h
	m_currentSpeed += m_acceleration * deltaTime;
	GetPawn()->AddMovementInput(direction.GetSafeNormal(), FMath::Min(m_currentSpeed, m_maxSpeed)/100.f);//le 2e parametre de addMovementInput est une constante entre -1 et 1 utilisé pour scale la vitesse CharacterMouvementComponent
	GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}

void ASDTAIController::ChooseSide(float deltaTime)
{
	UWorld* World = GetWorld();
	APawn* pawn = GetPawn();
	TArray<FHitResult> outHitsRight;
	TArray<FHitResult> outHitsLeft;

	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);	// ajout des murs aux objets détectés
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);	// ajout des pièges
	objectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel5);	// retrait des Collectibles
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	FCollisionShape collisionShape;
	collisionShape.SetCapsule(pawn->GetSimpleCollisionRadius(), pawn->GetSimpleCollisionHalfHeight());	// Capsule de collision du pawn
	float const castDist = 300.0f;

	// On regarde des deux côtés du pawn pour savoir de quel côté tourner
	World->LineTraceMultiByObjectType(outHitsRight, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 100.f, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 100.f + pawn->GetActorRightVector() * castDist, objectQueryParams, queryParams);
	World->LineTraceMultiByObjectType(outHitsLeft, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 100.f, pawn->GetActorLocation() + pawn->GetActorForwardVector() * 100.f - pawn->GetActorRightVector() * castDist, objectQueryParams, queryParams);

	// Debug
	/*
	for (int32 i = 0; i < outHitsLeft.Num(); ++i)
		DrawDebugPoint(World, outHitsLeft[i].ImpactPoint, 7.0f, FColor::Yellow, false, 1.0f, 0);
	for (int32 i = 0; i < outHitsRight.Num(); ++i)
		DrawDebugPoint(World, outHitsRight[i].ImpactPoint, 7.0f, FColor::Green, false, 1.0f, 0);
	*/

	bool isObstacleLeft = false;
	bool isObstacleRight = false;

	if (outHitsLeft.Num() > 0)
	{
		isObstacleLeft = true;
	}
		
	if (outHitsRight.Num() > 0)
	{
		isObstacleRight = true;
	}
	
	if (isObstacleLeft && isObstacleRight)
		// On tourne à droite
		choosen_side = -1;
	else if (isObstacleLeft && !isObstacleRight)
		// On tourne à droite
		choosen_side = -1;
	else if (!isObstacleLeft && isObstacleRight)
		// On tourne à gauche
		choosen_side = 1;
	else
	{
		// On choisit un côté au hasard
		choosen_side = (rand() % 2) * 2 - 1;
		
	}

}

void ASDTAIController::AvoidWall(float deltaTime)
{
	UWorld* World = GetWorld();
	APawn* pawn = GetPawn();
	TArray<FHitResult> outHits;

	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);	// ajout des murs aux objets détectés
	objectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);	// ajout des pièges
	objectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel5);	// retrait des Collectibles

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	FCollisionShape collisionShape;
	collisionShape.SetCapsule(pawn->GetSimpleCollisionRadius(), pawn->GetSimpleCollisionHalfHeight());	// Capsule de collision du pawn
	float const castDist = 300.0f;	// distance du balayage devant le pawn

	World->SweepMultiByObjectType(outHits, pawn->GetActorLocation(), pawn->GetActorLocation() + pawn->GetActorForwardVector() * castDist, FQuat::Identity, objectQueryParams, collisionShape, queryParams);

	// Debug
	/*
	for (int32 i = 0; i < outHits.Num(); ++i)
		DrawDebugPoint(World, outHits[i].ImpactPoint, 7.0f, FColor::Red, false, 1.0f, 0);
	*/

	// Calcul du nouveau vecteur de direction (faire un coefficient inv proportionnel a la distance au pawn)
	FHitResult blockingObstacle = outHits[0];
	FVector contactDirection;
	contactDirection = choosen_side * FVector::CrossProduct(FVector::UpVector, blockingObstacle.ImpactNormal.GetSafeNormal());

	float const coeffEvitement = FMath::Max(blockingObstacle.Distance - (pawn->GetSimpleCollisionRadius() + 50.0f), 0.0f) / (castDist - (pawn->GetSimpleCollisionRadius() + 50.0f));
	FVector const displacementDirection = pawn->GetActorForwardVector() * coeffEvitement + contactDirection * (1 - coeffEvitement);

	MovePawn(displacementDirection.GetSafeNormal(), deltaTime);
}