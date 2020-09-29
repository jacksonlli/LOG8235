// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"
#include "CollisionShape.h"
#include "PhysicsHelpers.h"
#include "SoftDesignTrainingMainCharacter.h"
#include <SoftDesignTraining\SDTUtils.h>
#include <SoftDesignTraining\SDTCollectible.h>

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
			if (IsAgentHeadingTowardsPlayer(deltaTime))//logic to check if agent is walking towards the player
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
	else if (GetBallDirection() != FVector(0,0,0))//logic for spotting power-up balls
	{
		choosen_side = 0;
		m_state = Stage::moveToBall;	// Diriger l'agent vers la direction du pickup.
	}
	else
	{
		choosen_side = 0;
		m_state = Stage::moveForwardState;
	}


	//Actions à appliquer pour les différents states définis
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
		// Déplacement orienté vers le joueur progressivement
		FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());
		MovePawn(GetPawn()->GetActorForwardVector().GetSafeNormal() * 0.9f + FVector(toPlayer, 0.f).GetSafeNormal()*0.1f, deltaTime);
	}
	break;
	case Stage::fleeState:
	{
		// Déplacement orienté dans la direction opposée au joueur progressivement
		FVector2D toPlayer = FVector2D(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation() - GetPawn()->GetActorLocation());

		MovePawn(GetPawn()->GetActorForwardVector().GetSafeNormal() * 0.6f  - FVector(toPlayer, 0.f).GetSafeNormal()*0.4f, deltaTime);
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
	case Stage::moveToBall:
	{
		MovePawn(GetBallDirection().GetSafeNormal(), deltaTime);
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

bool ASDTAIController::IsPlayerDetected()//détecter si l'agent voit le joueur
{
	if (GetWorld()->GetFirstPlayerController()->GetPawn() == nullptr)//détecter si on est en mode simulation
	{
		return false;
	}
	else
	{
		bool isPlayerInRange = FVector2D(GetPawn()->GetActorLocation() - GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()).Size() <= m_visionRadius;//voir si le joueur est à l'intérieur de du cercle de vision de l'agent

		FHitResult outHit;
		bool wallBetweenPlayerAndAgent = GetWorld()->LineTraceSingleByObjectType(outHit, GetPawn()->GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), ECC_WorldStatic);//voir si il y a un mur entre le joueur et l'agent

		//debug
		if (isPlayerInRange)
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
		

		return !wallBetweenPlayerAndAgent && isPlayerInRange;
	}
}

bool ASDTAIController::IsPlayerPoweredUp()//voir si le joueur est powered up ou non, retourne true si oui.
{

	ASoftDesignTrainingMainCharacter* player = dynamic_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetPawn());
	return player->IsPoweredUp();
}
bool ASDTAIController::IsAgentHeadingTowardsPlayer(float deltaTime)//voir si l'agent se dirige vers le joueur avec un cône de vision. 
{
	float theta = 45.f / 180.f*PI;//angle à chaque côté de la direction forward qui, lors de la détection d'un joueur powered-up dans la zone, tourne l'agent pour fuire
	//debug
	//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) + GetPawn()->GetActorRightVector()*sin(theta))*m_visionRadius, FColor::Orange, false, -1.f, 0, 10.f);
	//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + (GetPawn()->GetActorForwardVector()*cos(theta) - GetPawn()->GetActorRightVector()*sin(theta))*m_visionRadius, FColor::Orange, false, -1.f, 0, 10.f);

	FVector2D currentDirection = FVector2D(GetPawn()->GetActorForwardVector());
	FVector agentPosition = GetPawn()->GetActorLocation();

	APawn* player = GetWorld()->GetFirstPlayerController()->GetPawn();
	FVector playerLocation = player->GetActorLocation();

	FVector2D toPlayer = FVector2D( playerLocation - agentPosition ).GetSafeNormal();//vecteur de l'agent à la position présente du joueur

	return FVector2D::DotProduct(currentDirection, toPlayer) > cos(theta);//l'agent se dirige vers le joueur si le joueur est dans le cone de vision
}
bool ASDTAIController::IsBallDetected()
{
	return false;
}

void ASDTAIController::MovePawn(FVector direction, float deltaTime)//bouge l'agent vers la direction choisie avec une acceleration constante jusqu'à une vitesse max
{
	//voir definitions de m_currentSpeed, m_maxSpeed et m_acceleration dans SDTAIController.h
	m_currentSpeed += m_acceleration * deltaTime;
	GetPawn()->AddMovementInput(direction.GetSafeNormal(), FMath::Min(m_currentSpeed, m_maxSpeed)/100.f);//le 2e parametre de addMovementInput est une constante entre -1 et 1 utilisé pour scale la vitesse CharacterMouvementComponent
	GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}

void ASDTAIController::ChooseSide(float deltaTime)//détermine quel côté l'agent tourne en modifiant la variable chosenSide
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

void ASDTAIController::AvoidWall(float deltaTime)//évitement de mur et des pièges
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

FVector ASDTAIController::GetBallDirection()
{
	/*
		Fonction renvoyant la direction dans laquelle la balle la plus proche et visible par l'agent se situe.
		La fonction détecte les balles autour de l'AI, et renvoie le vecteur qui va le diriger vers la balle visible la plus proche
	*/

	APawn* pawn = GetPawn();
	UWorld* world = GetWorld();
	TArray<FHitResult> OutHits;
	FHitResult outHit;

	// Création d'une collision box qui permettra de détecter les balles autour de l'agent
	FCollisionShape CollisionBox;
	float angleForwardandX = std::abs(std::acos(FVector::DotProduct(pawn->GetActorForwardVector().GetSafeNormal(), FVector(1, 0, 0).GetSafeNormal())));
	float angleForwardandY = std::abs(std::acos(FVector::DotProduct(pawn->GetActorForwardVector().GetSafeNormal(), FVector(0, 1, 0).GetSafeNormal())));
	CollisionBox = FCollisionShape::MakeBox(FVector(1000, 1000, 300));

	// Récupération de tous les éléments se trouvant à l'intérieur de la collision box dans la variable OutHits
	FVector SweepStart = pawn->GetActorLocation();
	FVector SweepEnd = pawn->GetActorLocation();
	SweepEnd.Z += 0.001f;

	bool isHit = GetWorld()->SweepMultiByObjectType(OutHits, SweepStart, SweepEnd, FQuat::Identity, COLLISION_COLLECTIBLE, CollisionBox);

	if (isHit)
	{
		for (auto& Hit : OutHits)
		{
			if (GEngine)
			{
				FVector const toTarget = Hit.Actor->GetActorLocation() - pawn->GetActorLocation();
				// vérification de la visibilité de la balle
				// Pour cela, un mur ne doit pas séparer l'agent de la balle et la balle ne doit pas être dans un état de cooldown
				bool canSee = !world->LineTraceSingleByObjectType(outHit, pawn->GetActorLocation(), Hit.Actor->GetActorLocation(), ECC_WorldStatic) &&
					(std::acos(FVector::DotProduct(pawn->GetActorForwardVector().GetSafeNormal(), toTarget.GetSafeNormal()))) < PI / 3.0f;
				ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(Hit.GetActor());

				if (canSee && !collectible->IsOnCooldown())
				{
					return toTarget;
				}

			}
		}
	}
	return FVector(0, 0, 0); // Valeur retournée dans le cas où aucune balle n'est détectée
}