// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"


#define PATH_FOLLOW_DEBUG
#define SHORTCUT_SAMPLE_NUM 4

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{

	if (PathToFollow.Num() > 0)
	{
		FVector2D position2D(GetPawn()->GetActorLocation());
		FVector destination(PathToFollow.Last());
		FVector2D destination2D(destination);
		//GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Repath: %f, %f"), destination2D.X, destination2D.Y));
		if (position2D.Equals(destination2D, 10.f))
		{
			ShouldRePath = true;
		}
		TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
		detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));

		TArray<FHitResult> detectedPickup;
		bool isHits = GetWorld()->SweepMultiByObjectType(detectedPickup, FVector(destination2D, 245.f), FVector(destination2D, 255.f), FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

		if (isHits)
		{
			for (FHitResult Hit : detectedPickup)
			{
				ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(Hit.GetActor());
				if (!collectible->GetStaticMeshComponent()->IsVisible())
				{
					ShouldRePath = true;
					
				}
			}
		}
		
	}
	
    //Move to target depending on current behavior
    switch (m_state)
    {
    case ASDTAIController::AIState::Pursue:
		if (ShouldRePath)
			ComputePath(UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()), deltaTime);
		MoveTowardsDirection(deltaTime);
		break;
    case ASDTAIController::AIState::Flee:
		if (ShouldRePath)
            ComputePath(GetPathToFleeLocation(), deltaTime);
		MoveTowardsDirection(deltaTime);
        break;
    case ASDTAIController::AIState::GoToClosestPickup:
        GoToClosestPickup(deltaTime);
        break;
    default:
		break;
    }
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere
    for (int i = 0; i < PathToFollow.Num() - 1; i++)
    {
        DrawDebugLine(
            GetWorld(),
            PathToFollow[i].Location,
            PathToFollow[i + 1].Location,
            FColor::White,
            false, -1, 0,
            5.f);
        if (SDTUtils::HasJumpFlag(PathToFollow[i]))
        {
            DrawDebugSphere(
                GetWorld(),
                PathToFollow[i].Location,
                20.0f, 16,
                FColor::Green,
                false, -1, 0);
        }
        else
        {
            DrawDebugSphere(
                GetWorld(),
                PathToFollow[i].Location,
                10.0f, 16,
                FColor::Red,
                false, -1, 0);
        }
    }
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    
    if (IsPlayerDetected())// Si on detecte le joueur, choisir de le poursuivre ou le fuir
    {
		ShouldRePath = true; //make sure the agent repaths based off new player coordinates while player is detected
		if (SDTUtils::IsPlayerPoweredUp(GetWorld()))
		{
			m_state = AIState::Flee;

		}
		else {
			m_state = AIState::Pursue;
		}
            
    }
	else
	{
		// Sinon, finir le chemin actuel et aller vers le pickup le plus proche
		m_state = AIState::GoToClosestPickup;
	}
    
}

// If the player is inside the detection capsule, returns the player in outDetectionHit.
void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
	ShouldRePath = true;
}

// Finds the closest pickup and computes the path to go at its location
void ASDTAIController::GoToClosestPickup(float deltaTime)
{
    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;
    if (ShouldRePath)
    {
        // Choix du pickup le plus proche
        float mincost = 9999999999;
        UNavigationPath* chosenPath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), selfPawn->GetActorLocation());
        for (const FVector2D& pickupLocation : listLocation)
        {
			TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
			detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));

			TArray<FHitResult> detectedPickup;
			bool isHits = GetWorld()->SweepMultiByObjectType(detectedPickup, FVector(pickupLocation, 245.f), FVector(pickupLocation, 255.f), FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

			if (isHits)
			{
				for (FHitResult Hit : detectedPickup)
				{
					ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(Hit.GetActor());
					if (collectible->GetStaticMeshComponent()->IsVisible())
					{
						UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), FVector(pickupLocation, selfPawn->GetActorLocation().Z), selfPawn);
						float cost = (pickupLocation - FVector2D(selfPawn->GetActorLocation())).SizeSquared();
						if (cost < mincost)
						{
							mincost = cost;
							chosenPath = path;
						}
					}
				}
				
			}
        }
        ComputePath(chosenPath, deltaTime);
    }
    
        ShowNavigationPath();
        MoveTowardsDirection(deltaTime);
}

// Parcourir le chemin et mettre un JumpFlag si on detecte un NavLink
void ASDTAIController::ComputePath(UNavigationPath* path, float deltaTime)
{
    if (path->IsValid())
    {
        PathToFollow.Empty();

        for (FNavPathPoint point : path->GetPath()->GetPathPoints())
        {

            if (SDTUtils::IsNavLink(point))
            {
                uint16 flags = FNavMeshNodeFlags(point.Flags).AreaFlags;
                SDTUtils::SetNavTypeFlag(flags, SDTUtils::NavType::Jump);
                uint32 flag32 = flags;
                point.Flags = flag32 * 2e16;
            }

            PathToFollow.Add(point);
        }
        CurrentDestinationIndex = 0;
    }

    ShouldRePath = false;
}


FVector ASDTAIController::ComputeDestination(float deltaTime)
{
    FVector2D position2D(GetPawn()->GetActorLocation());
    FVector destination(PathToFollow[CurrentDestinationIndex]);
    FVector2D destination2D(destination);

    if (position2D.Equals(destination2D, 10.f))
    {
        CurrentDestinationIndex++;

        if (CurrentDestinationIndex == PathToFollow.Num())
        {
            CurrentDestinationIndex = -1;
            destination = FVector(position2D, 0.f) + Direction;
        }
        else
        {
            destination = PathToFollow[CurrentDestinationIndex];
        }
    }

    if (!position2D.Equals(destination2D, 20.f) && CurrentDestinationIndex > 0)
        UseIntermediaryDestination_Behavior(position2D, destination2D, destination);
        
    return destination;
}

// For smoother movements
void ASDTAIController::UseIntermediaryDestination_Behavior(FVector2D position2D, FVector2D destination2D, FVector& destination)
{
    FVector lineStart = PathToFollow[CurrentDestinationIndex - 1];
    FVector lineEnd = PathToFollow[CurrentDestinationIndex];
    FVector lineDirection = lineEnd - lineStart;
    lineDirection.Normalize();

    destination = (FMath::ClosestPointOnLine(lineStart, lineEnd, GetPawn()->GetActorLocation()) + destination) / 2;
    DrawDebugSphere(GetWorld(), destination, 20.f, 20, FColor::Red);
}


void ASDTAIController::UpdateDirection(float deltaTime, FVector directionGoal)
{
    directionGoal.Z = 0.f;

    if (!FMath::IsNearlyEqual(directionGoal.SizeSquared(), 1))
        directionGoal.Normalize();
    
    // Si proche de la direction desiree, y aller direct
    if (Direction.Equals(directionGoal, 0.01f))
    {
        Direction = directionGoal;
        IsTurning = false;
        if (CurrentDestinationIndex == IndexAfterSlowDown)
            SlowDownTargetSpeed = -1.f;
        return;
    }
    // Sinon y aller progressivement
    IsTurning = true;

    FRotator currentRotation = Direction.ToOrientationRotator();
    FRotator goalRotation = directionGoal.ToOrientationRotator();

    FRotator deltaRotation = goalRotation - currentRotation;

    if (deltaRotation.Yaw > 180.f)
        deltaRotation.Yaw -= 360.f;

    if (deltaRotation.Yaw < -180.f)
        deltaRotation.Yaw += 360.f;

    float RotationThisFrame = RotationRate * deltaTime;

    deltaRotation.Yaw = FMath::Clamp(deltaRotation.Yaw, -RotationThisFrame, RotationThisFrame);

    Direction = deltaRotation.RotateVector(Direction);
}


FVector ASDTAIController::ComputeVelocity(float deltaTime, FVector destination)
{
    FVector velocity = FVector::ZeroVector;
    float speed = MaxSpeed;

    /*if (UseSlowDownForTurns && (IsTurning || (destination - GetActorLocation()).Size() < 200.f) && CurrentDestinationIndex != -1 && CurrentDestinationIndex != PathToFollow.Num() - 1)
        speed = UseSlowDownForTurns_Behavior(destination, DeltaTime);*/

    CurrentSpeed = FMath::Clamp(CurrentSpeed + (Acceleration * deltaTime), 0.f, speed);
    velocity = Direction * CurrentSpeed;

    return velocity;
}


void ASDTAIController::ApplyVelocity(float deltaTime, FVector velocity)
{
    if (!velocity.IsNearlyZero())
    {
        FVector position = GetPawn()->GetActorLocation();

        FVector destination = position + velocity * deltaTime;

        GetPawn()->SetActorLocation(destination);
        GetPawn()->SetActorRotation(Direction.ToOrientationQuat(), ETeleportType::None);
    }
}


void ASDTAIController::MoveTowardsDirection(float deltaTime)
{
    if (PathToFollow.Num() > 0)
    {
        FVector velocity = FVector::ZeroVector;

        // si on n'est pas a une extremite du chemin
        if (CurrentDestinationIndex > -1 && CurrentDestinationIndex < PathToFollow.Num())
        {
            FVector position = GetPawn()->GetActorLocation();
            FVector destination = ComputeDestination(deltaTime);

            //GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Direction : %f, %f, %f"), destination.X, destination.Y, destination.Z));

#ifdef PATH_FOLLOW_DEBUG
            {
                DrawDebugSphere(
                    GetWorld(),
                    destination,
                    20.f, 20,
                    FColor(255, 0, 0),
                    false, -1, 0
                );
            }
#endif //PATH_FOLLOW_DEBUG

            UpdateDirection(deltaTime, destination - position);
            velocity = ComputeVelocity(deltaTime, destination);
        }
        else
        {
            velocity = ComputeVelocity(-deltaTime, GetPawn()->GetActorLocation() + Direction);

            if (velocity.SizeSquared() < 0.1f)
            {
                CurrentSpeed = 0.f;
                CurrentDestinationIndex = -1.f;
                //IsWalking = false;
            }
        }

        // --------------------------- //
                // A modifier en remplaçant le IsNavLink par HasJumpFlag
                // --------------------------- //
        if(CurrentDestinationIndex > 0)
        { 
        if (SDTUtils::HasJumpFlag(PathToFollow[CurrentDestinationIndex-1]))
        {
            APawn* selfPawn = GetPawn();
            ACharacter* character = Cast<ACharacter>(GetPawn());
            ASDTAIController* controller = Cast<ASDTAIController>(character->GetController()); // A garder pour le moment
            auto MovementComponent = character->GetCharacterMovement();

            // Paramètre à modifier au besoin
            MovementComponent->JumpZVelocity = 600.0f;
            MovementComponent->DoJump(false);
            

            GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, FString::Printf(TEXT("Saut en cours ")));
        }
        }

        ApplyVelocity(deltaTime, velocity);
    }
}

bool ASDTAIController::IsPlayerDetected()
{
	APawn* selfPawn = GetPawn();
	FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
	FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

	TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
	detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
	detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

	TArray<FHitResult> allDetectionHits;
	GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

	FHitResult detectionHit;
	GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

	bool playerInDetectionZone = allDetectionHits.Num() != 0 && detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER;
	FHitResult outHit;
	bool wallInBetween = GetWorld()->LineTraceSingleByObjectType(outHit, GetPawn()->GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), ECC_WorldStatic);

	//DEBUG
	 DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);

	return playerInDetectionZone && !wallInBetween;
}

UNavigationPath* ASDTAIController::GetPathToFleeLocation()
{
	TArray<AActor*> foundLocations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), foundLocations);
	FVector directionAwayFromPlayer = (GetPawn()->GetActorLocation() - GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()).GetSafeNormal();
	float maxDirectionScore = -999999;
	FVector bestFleeLocation;
	for (AActor* fleeLocationObject : foundLocations)
	{
		FVector fleeLocationVector = fleeLocationObject->GetActorLocation();
		FVector directionToFleeLocation = (fleeLocationVector - GetPawn()->GetActorLocation()).GetSafeNormal();
		float directionScore = FVector::DotProduct(directionAwayFromPlayer, directionToFleeLocation);
		if (directionScore > maxDirectionScore)
		{
			bestFleeLocation = fleeLocationVector;
			maxDirectionScore = directionScore;
		}
	}
	DrawDebugCircle(GetWorld(), bestFleeLocation, 100, 50, FColor::Blue, false, 10.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	return UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), bestFleeLocation);
}

