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
        break;
    case ASDTAIController::AIState::GoToLastSeen:
        break;
    case ASDTAIController::AIState::Flee:
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
            PathToFollow[i],
            PathToFollow[i + 1],
            FColor::White,
            false, -1, 0,
            5.f);
        DrawDebugSphere(
            GetWorld(),
            PathToFollow[i + 1],
            10.0f, 16,
            FColor::Red,
            false, -1, 0);
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

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);


    //Set behavior based on hit
    if (allDetectionHits.Num() != 0)
    {
        // Si c'est le joueur, le poursuivre ou le fuir
        UPrimitiveComponent* component = detectionHit.GetComponent();
        if (component->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            // A completer
            m_state = AIState::Pursue;
        }
    }

    // Sinon, aller vers le pickup le plus proche
    m_state = AIState::GoToClosestPickup;
    
    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
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
						UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), FVector(pickupLocation, selfPawn->GetActorLocation().Z));
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

        if (chosenPath->IsValid())
        {
            PathToFollow.Empty();
            for (FNavPathPoint point : chosenPath->GetPath()->GetPathPoints())
                PathToFollow.Add(point.Location);
			CurrentDestinationIndex = 0;
        }

        ShouldRePath = false;
    }
    
        ShowNavigationPath();
        MoveTowardsDirection(deltaTime);
}


FVector ASDTAIController::ComputeDestination(float deltaTime)
{
    FVector2D position2D(GetPawn()->GetActorLocation());
    FVector destination(PathToFollow[CurrentDestinationIndex]);
    FVector2D destination2D(destination);

    if (UseShortcuts)
        UseShortcuts_Behavior(destination2D, destination);

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

    /*if (UseIntermediaryDestinations && !position2D.Equals(destination2D, 20.f) && CurrentDestinationIndex > 0)
        UseIntermediaryDestination_Behavior(position2D, destination2D, destination);
        */
    return destination;
}


// Breaks the path segment into smaller segments
void ASDTAIController::UseShortcuts_Behavior(FVector2D& destination2D, FVector& destination)
{
    if (CurrentDestinationIndex != PathToFollow.Num() - 1)
    {
        TArray<FVector> sampleShortcutPoints;

        sampleShortcutPoints.Empty();
        sampleShortcutPoints.Reserve(SHORTCUT_SAMPLE_NUM);              // prepare space in the array
        FVector startNextPath = PathToFollow[CurrentDestinationIndex];  // path segment
        FVector endNextPath = PathToFollow[CurrentDestinationIndex + 1];

        FVector displacementSamples = (endNextPath - startNextPath) / SHORTCUT_SAMPLE_NUM;

        for (int i = 0; i < SHORTCUT_SAMPLE_NUM; i++)
        {
            FVector sample = startNextPath + ((i + 1) * displacementSamples);   // fraction of the path segment
            sampleShortcutPoints.Push(sample);
#ifdef PATH_FOLLOW_DEBUG
            {
                DrawDebugSphere(
                    GetWorld(),
                    sample,
                    10.f, 20,
                    FColor::Emerald,
                    false, -1, 0
                );
            }
#endif //PATH_FOLLOW_DEBUG
        }

        // Decide what is the farthest sample point reachable and sets it as the new destination
        FVector originBB = FVector::ZeroVector;
        FVector extentsBB;
        GetPawn()->GetActorBounds(true, originBB, extentsBB);
        extentsBB.Z = 0.f;

        for (int i = SHORTCUT_SAMPLE_NUM - 1; i >= 0; i--)
        {
            FVector start = GetPawn()->GetActorLocation();
            start.Z = originBB.Z;
            FVector end = sampleShortcutPoints[i];
            end.Z = originBB.Z;

            FVector sideVectorLeft = FVector::CrossProduct((end - start), GetPawn()->GetActorUpVector());
            sideVectorLeft.Normalize();
            sideVectorLeft *= extentsBB.Size();
            FVector sideVectorRight = -sideVectorLeft;

            FHitResult HitOut;
            if (!SDTUtils::Raycast(GetWorld(), start + sideVectorLeft, end + sideVectorLeft)
                &&
                !SDTUtils::Raycast(GetWorld(), start + sideVectorRight, end + sideVectorRight)
                )
            {
                destination = sampleShortcutPoints[i];
                destination2D = FVector2D(sampleShortcutPoints[i]);
                if (destination.Equals(PathToFollow[CurrentDestinationIndex + 1]))
                    CurrentDestinationIndex++;
                break;
            }
        }
    }
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

            GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Direction : %f, %f, %f"), destination.X, destination.Y, destination.Z));

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
            GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Direction : %f, %f, %f"), Direction.X, Direction.Y, Direction.Z));

            if (velocity.SizeSquared() < 0.1f)
            {
                CurrentSpeed = 0.f;
                CurrentDestinationIndex = -1.f;
                //IsWalking = false;
            }
        }

        ApplyVelocity(deltaTime, velocity);
    }
}