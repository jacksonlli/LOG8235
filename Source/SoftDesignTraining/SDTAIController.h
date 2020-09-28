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
	bool IsAgentHeadingTowardsPlayer();
	bool IsBallDetected();
	void MovePawn(FVector direction, float deltaTime);
	virtual void AvoidWall(float deltaTime);
	void ChooseSide(float deltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	//Le maxSpeed de l'agent est un seuil de vitesse mesur� en pourcentage de la vitesse CharacterMovementComponent (%). Toutes valeurs plus grandes que 100, serait trait�es comme 100. Par exemple, avec une valeur de 25, l'agent se d�placerait au plus vite � un quart de la vitesse CharacterMovementComponent.
	float m_maxSpeed = 70.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	//L'acceleration de l'agent est mesur� en pourcentage de la vitesse CharacterMovementComponent par seconde (%/s). Par exemple, avec valeur de 25, un agent immobile atteindra la vitesse CharacterMovementComponent en 4 secondes.
	float m_acceleration = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	//La distance de d�tection des agents est mesur� en unit� de distance de UE4.
	float m_visionRadius = 600.f;

private:
	float m_currentSpeed = 0.f;//mesur� en pourcentage de la vitesse CharacterMovementComponent, valeur entre 0 et 100

	//STATE INDEXES
	enum class Stage
	{
		moveForwardState,
		chaseState,
		fleeState,
		avoidObstacleState
	};
	//DEFAULT CURRENT STATE
	Stage m_state = Stage::moveForwardState;
	// Choix du cot� vers lequel on tourne : 0 - aucun cot� choisi / 1 - Tourne vers la gauche / -1 - Tourne vers la droite
	int choosen_side = 0; 
};