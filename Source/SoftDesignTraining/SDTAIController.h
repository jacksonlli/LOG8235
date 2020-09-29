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
	bool IsWallOrTrapInTrajectory();
	bool IsPlayerDetected();
	bool IsPlayerPoweredUp();
	bool IsAgentHeadingTowardsPlayer(float deltaTime);
	void MovePawn(FVector direction, float deltaTime);
	virtual void AvoidWall(float deltaTime);
	FVector GetBallDirection();
	void ChooseSide(float deltaTime);

	// Ces paramètres sont exposés dans Unreal et sont accessibles en double-cliquant sur le fichier Content\Blueprint\BP_SDTAIController, onglet AI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0"))
		//Le maxSpeed de l'agent est un seuil de vitesse mesuré en pourcentage de la vitesse CharacterMovementComponent (%). Par exemple, avec une valeur de 25, l'agent se déplacerait au plus vite à un quart de la vitesse CharacterMovementComponent.
		float m_maxSpeed = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "200.0", UIMin = "0.0", UIMax = "200.0"))
		//L'acceleration de l'agent est mesuré en pourcentage de la vitesse CharacterMovementComponent par seconde (%/s). Par exemple, avec valeur de 25, un agent immobile atteindra la vitesse CharacterMovementComponent en 4 secondes.
		float m_acceleration = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "200.0", ClampMax = "1500.0", UIMin = "200.0", UIMax = "1500.0"))
		//La distance de détection des agents est mesuré en unité de distance de UE4.
		float m_visionRadius = 600.f;

private:
	float m_currentSpeed = 0.f;//mesuré en pourcentage de la vitesse CharacterMovementComponent, valeur entre 0 et 100

	//STATE INDEXES
	enum class Stage
	{
		moveForwardState,
		chaseState,
		fleeState,
		avoidObstacleState,
		moveToBall
	};
	//DEFAULT CURRENT STATE
	Stage m_state = Stage::moveForwardState;
	// Choix du coté vers lequel on tourne : 0 - aucun coté choisi / 1 - Tourne vers la gauche / -1 - Tourne vers la droite
	int choosen_side = 0; 
};