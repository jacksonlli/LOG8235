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

	if (PathToFollow.Num() > 0 && AtJumpSegment == false)
	{
		FVector2D position2D(GetPawn()->GetActorLocation());
		FVector destination(PathToFollow.Last());
		FVector2D destination2D(destination);
		//GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Repath: %f, %f"), destination2D.X, destination2D.Y));
		if (position2D.Equals(destination2D, 10.f))
		{
			ShouldRePath = true;
			goToLKP = false;
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
	case ASDTAIController::AIState::Pursue://si on est en mode Poursuite, calculer un chemin vers le joueur et se diriger sur ce chemin
		if (ShouldRePath)//recalculer le chemin si on voit le joueur, si oui on met en jour le chemin. Sinon, on suis le chemin existant qui mene vers le LKP
			ComputePath(UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()), deltaTime);
		goToLKP = true;
		MoveTowardsDirection(deltaTime);
		break;
	case ASDTAIController::AIState::Flee://si on est en mode Fuite, calculer le chemin vers le fleelocation approprié
		if (ShouldRePath)//recalculer le chemin si le joueur apparait dans le champs de vision de l'agent (aka l'agent se dirige vers le joueur powered-up)
			ComputePath(GetPathToFleeLocation(), deltaTime);
		TimeLeftFlee = 3.f;
		MoveTowardsDirection(deltaTime);
		break;
	case ASDTAIController::AIState::GoToClosestPickup:
		GoToClosestPickup(deltaTime);
		TimeLeftFlee = std::max(TimeLeftFlee - deltaTime, 0.f);
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
		ShouldRePath = true; //s'assurez-vous que l'agent repath en fonction des coordonnées du joueur lorsque le joueur est détecté
		if (SDTUtils::IsPlayerPoweredUp(GetWorld()))//si le joueur est-powered-up l<agent entre en mode fuite
		{
			m_state = AIState::Flee;

		}
		else {
			m_state = AIState::Pursue;//sinon l'agent entre en mode pousuite
		}

	}
	else
	{
		// Si on ne voit pas le joeur, finir le chemin actuel et aller vers le pickup le plus proche
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
	ShouldRePath = true;//recalculer le repath si l'agent meure
}

// Finds the closest pickup and computes the path to go at its location
void ASDTAIController::GoToClosestPickup(float deltaTime)
{
	APawn* selfPawn = GetPawn();
	if (!selfPawn)
		return;

	TArray<AActor*> foundLocations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundLocations);
	FVector destination(0.f, 0.f, 0.f);
	if (PathToFollow.Num() > 0) { destination = PathToFollow.Last(); }
	
	FVector2D destination2D(destination);
	FVector2D newDestination(0.f, 0.f);
	float mincost = 9999999999;
	UNavigationPath* chosenPath = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), selfPawn->GetActorLocation());;
	// On regarde si la destination actuelle est toujours le pickup le plus proche
	for (AActor* collectibleObject : foundLocations)
	{
		FVector2D pickupLocation = FVector2D(collectibleObject->GetActorLocation());
		ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(collectibleObject);
		if (collectible->GetStaticMeshComponent()->IsVisible())
		{
			//float cost = (pickupLocation - FVector2D(selfPawn->GetActorLocation())).SizeSquared();
			UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), FVector(pickupLocation, selfPawn->GetActorLocation().Z), selfPawn);
			if (path->IsValid())
			{
				float cost = 0;
				FNavPathPoint currentPoint;
				FNavPathPoint oldPoint;
				for (FNavPathPoint point : path->GetPath()->GetPathPoints())
				{
					if (cost == 0)
					{
						currentPoint = point;
						cost = cost + 1.f;
					}
					else
					{
						oldPoint = currentPoint;
						currentPoint = point;
						cost = cost + (FVector2D(currentPoint) - FVector2D(oldPoint)).SizeSquared();
					}
				}
				if (cost < mincost)
				{
					mincost = cost;
					chosenPath = path;
					newDestination = pickupLocation;
				}
			}
		}
	}
	
	// Si on est pas dans un saut et qu'on doit recalculer un chemin ou que la destination a changé
	// Et qu'on est pas dans un état d'intéraction avec le joueur (fuite ou recherche de la dernière position
	if ((ShouldRePath || (destination2D - newDestination).SizeSquared() > 10.f) && AtJumpSegment == false && goToLKP == false  && TimeLeftFlee == 0.f)
	{
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
		//FNavPathPoint precPoint;

		for (FNavPathPoint point : path->GetPath()->GetPathPoints())
		{

			if (SDTUtils::IsNavLink(point)) //&& !SDTUtils::IsNavLink(precPoint))
			{
				uint16 flags = FNavMeshNodeFlags(point.Flags).AreaFlags;
				SDTUtils::SetNavTypeFlag(flags, SDTUtils::NavType::Jump);
				uint32 flag32 = flags;
				point.Flags = flag32 * 2e16;
			}

			PathToFollow.Add(point);
			//precPoint = point;
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

	if (position2D.Equals(destination2D, 10.f) || CurrentDestinationIndex == 0)
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

	if (IsTurning && CurrentDestinationIndex != -1 && CurrentDestinationIndex != PathToFollow.Num() - 1)
		speed = ComputeTargetSpeedForTurn(); // UseSlowDownForTurns_Behavior(destination, deltaTime);

	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, FString::Printf(TEXT("Velocity : %f"), speed));

	CurrentSpeed = FMath::Clamp(CurrentSpeed + (Acceleration * deltaTime), 0.f, speed);
	velocity = Direction * CurrentSpeed;

	return velocity;
}


float ASDTAIController::ComputeTargetSpeedForTurn()
{
	if (CurrentDestinationIndex == PathToFollow.Num() - 1)
		return -1.f;

	FVector nextDirection = PathToFollow[CurrentDestinationIndex + 1].Location - PathToFollow[CurrentDestinationIndex].Location;
	nextDirection.Normalize();

	float speedFactor = 1.f;

	if (CurrentDestinationIndex != 0)
	{
		//FVector currentDirection = PathToFollow[CurrentDestinationIndex].Location - PathToFollow[CurrentDestinationIndex - 1].Location;
		//currentDirection.Normalize();

		FVector currentDirectionStart = GetPawn()->GetActorLocation();
		FVector currentDirectionEnd = PathToFollow[CurrentDestinationIndex];

		FVector currentDirection = currentDirectionEnd - currentDirectionStart;
		currentDirection.Normalize();

		speedFactor = 0.5f + FVector::DotProduct(currentDirection, nextDirection) / 2;
	}

	//speedFactor = (FVector::DotProduct(nextDirection, currentDirection) + 1.f) / 2.f;

	return MaxSpeed * speedFactor;
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
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Blue, FString::Printf(TEXT("Index : %i"), CurrentDestinationIndex));
	if (PathToFollow.Num() > 0)
	{
		FVector velocity = FVector::ZeroVector;
		FVector position = GetPawn()->GetActorLocation();

		// si on n'est pas a une extremite du chemin
		if (CurrentDestinationIndex > -1 && CurrentDestinationIndex < PathToFollow.Num())
		{
			FVector destination = ComputeDestination(deltaTime);

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

		if (CurrentDestinationIndex > 0)
		{
			if (SDTUtils::HasJumpFlag(PathToFollow[CurrentDestinationIndex - 1]))
			{
				if (!AtJumpSegment)
				{
					// Handle starting jump
					BaseHeight = GetPawn()->GetActorLocation().Z;
					IsWalking = false;
					AtJumpSegment = true;
					StartJump(deltaTime);
					velocity = FVector::OneVector * MaxSpeed;
				}
				else
				{
					// Update jump
					FVector jumpStart = PathToFollow[CurrentDestinationIndex - 1].Location;
					float sizeJump = FVector2D(PathToFollow[CurrentDestinationIndex].Location - jumpStart).Size();
					float distFromJumpStart = FVector2D(GetPawn()->GetActorLocation() - jumpStart).Size();

					//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("distFromJumpStart : %f"), distFromJumpStart/sizeJump));
					UpdateJump(distFromJumpStart / sizeJump, deltaTime);
				}
			}
			else
			{
				// Handle normal segments
				if (Landing)
					GetPawn()->SetActorLocation(FVector(FVector2D(position), BaseHeight));
				InAir = false;
				Landing = false;
				AtJumpSegment = false;
				IsWalking = true;
			}

			GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Blue, FString::Printf(TEXT("Pawn Height : %f"), GetPawn()->GetActorLocation().Z));
			GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Blue, FString::Printf(TEXT("Index : %i"), CurrentDestinationIndex));
			ApplyVelocity(deltaTime, velocity);
		}
	}
}


void ASDTAIController::StartJump(float deltaTime)
{
	/*APawn* selfPawn = GetPawn();
	ACharacter* character = Cast<ACharacter>(GetPawn());
	ASDTAIController* controller = Cast<ASDTAIController>(character->GetController()); // A garder pour le moment
	auto MovementComponent = character->GetCharacterMovement();

	// Paramètre à modifier au besoin
	MovementComponent->JumpZVelocity = 600.0f;
	MovementComponent->DoJump(false);*/

	InAir = true;
}

void ASDTAIController::UpdateJump(float curveTime, float deltaTime)
{
	float updateHeight = JumpCurve->GetFloatValue(curveTime) * JumpApexHeight;
	FVector2D position2D(GetPawn()->GetActorLocation());

	GetPawn()->SetActorLocation(FVector(position2D, BaseHeight + updateHeight));
	//GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Blue, FString::Printf(TEXT("Pawn Height : %f"), GetPawn()->GetActorLocation().Z));

	if (curveTime > 0.8)
	{
		InAir = false;
		Landing = true;
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

UNavigationPath* ASDTAIController::GetPathToFleeLocation()//permet de trouver le chemin vers le fleelocation approprié
{
	TArray<AActor*> foundLocations;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), foundLocations);//trouve tous les fleelocations
	FVector directionAwayFromPlayer = (GetPawn()->GetActorLocation() - GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation()).GetSafeNormal();//trouve le vecteur de direction opposé au joueur
	float maxDirectionScore = -999999;
	FVector bestFleeLocation;
	for (AActor* fleeLocationObject : foundLocations)//itéré à travers tous les fleelocations pour trouver le meilleur chemin
	{
		FVector fleeLocationVector = fleeLocationObject->GetActorLocation();
		FVector directionToFleeLocation = (fleeLocationVector - GetPawn()->GetActorLocation()).GetSafeNormal();
		float directionScore = FVector::DotProduct(directionAwayFromPlayer, directionToFleeLocation);//le meilleur chemin est celui ou l'angle entre le vecteur de direction vers le fleelocation et le vecteur direction opposé au joueur est la plus petite
		if (directionScore > maxDirectionScore)
		{
			bestFleeLocation = fleeLocationVector;
			maxDirectionScore = directionScore;
		}
	}
	//DrawDebugCircle(GetWorld(), bestFleeLocation, 100, 50, FColor::Blue, false, 10.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
	return UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), bestFleeLocation);
}