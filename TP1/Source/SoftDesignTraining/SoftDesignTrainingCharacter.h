// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "SoftDesignTrainingCharacter.generated.h"


UCLASS()
class ASoftDesignTrainingCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASoftDesignTrainingCharacter();

    virtual void BeginPlay() override;
    virtual void OnCollectPowerUp() {};


	UPROPERTY(BlueprintReadOnly, Category = AI)
		//Nombre de pickups ramassés par agent
		int nb_pickup = 0;
	UPROPERTY(BlueprintReadOnly, Category = AI)
		//Nombre de morts (par piège) par agent
		int nb_death = 0;

protected:
    UFUNCTION()
    virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    FVector m_StartingPosition;
};

