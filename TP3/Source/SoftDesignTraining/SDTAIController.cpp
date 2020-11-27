// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

// AJOUTS
#include "GameFramework/Character.h"
#include "SDTBaseAIController.h"
#include "AiAgentGroupManager.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
    m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
	m_inGroup = false;
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{

    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Collect:

        MoveToRandomCollectible();

        break;

    case PlayerInteractionBehavior_Chase:
	{

		MoveToPlayer();
	}
        break;
	
    case PlayerInteractionBehavior_Flee:
        MoveToBestFleeLocation();

        break;
    }
}

void ASDTAIController::MoveToRandomCollectible()
{
	double start = FPlatformTime::Seconds();
    float closestSqrCollectibleDistance = 18446744073709551610.f;
    ASDTCollectible* closestCollectible = nullptr;

    TArray<AActor*> foundCollectibles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), foundCollectibles);

    while (foundCollectibles.Num() != 0)
    {
        int index = FMath::RandRange(0, foundCollectibles.Num() - 1);

        ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(foundCollectibles[index]);
        if (!collectibleActor)
            return;

        if (!collectibleActor->IsOnCooldown())
        {
            MoveToLocation(foundCollectibles[index]->GetActorLocation(), 0.5f, false, true, true, NULL, false);
            OnMoveToTarget();
            return;
        }
        else
        {
            foundCollectibles.RemoveAt(index);
        }
    }
	double stop = FPlatformTime::Seconds();
	timeCPU_Collect = 1000*(stop - start);
}

void ASDTAIController::MoveToPlayer()
{

    ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    MoveToActor(playerCharacter, 0.5f, false, true, true, NULL, false);
    OnMoveToTarget();
}

void ASDTAIController::PlayerInteractionLoSUpdate()
{
    ACharacter * playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    FHitResult losHit;
    GetWorld()->LineTraceSingleByObjectType(losHit, GetPawn()->GetActorLocation(), playerCharacter->GetActorLocation(), TraceObjectTypes);

    bool hasLosOnPlayer = false;

    if (losHit.GetComponent())
    {
        if (losHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            hasLosOnPlayer = true;
        }
    }

    if (hasLosOnPlayer)
    {
        ClearTimer();
    }
    else
    {
        UpdateTimer();
    }
    
}

void ASDTAIController::OnPlayerInteractionNoLosDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "TIMER DONE", GetPawn(), FColor::Red, 5.f, false);

    if (!AtJumpSegment)
    {
        AIStateInterrupted();
        m_PlayerInteractionBehavior = PlayerInteractionBehavior_Collect;
    }
}

void ASDTAIController::MoveToBestFleeLocation()
{
	double start = FPlatformTime::Seconds();
    float bestLocationScore = 0.f;
    ASDTFleeLocation* bestFleeLocation = nullptr;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    for (TActorIterator<ASDTFleeLocation> actorIterator(GetWorld(), ASDTFleeLocation::StaticClass()); actorIterator; ++actorIterator)
    {
        ASDTFleeLocation* fleeLocation = Cast<ASDTFleeLocation>(*actorIterator);
        if (fleeLocation)
        {
            float distToFleeLocation = FVector::Dist(fleeLocation->GetActorLocation(), playerCharacter->GetActorLocation());

            FVector selfToPlayer = playerCharacter->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToPlayer.Normalize();

            FVector selfToFleeLocation = fleeLocation->GetActorLocation() - GetPawn()->GetActorLocation();
            selfToFleeLocation.Normalize();

            float fleeLocationToPlayerAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(selfToPlayer, selfToFleeLocation)));
            float locationScore = distToFleeLocation + fleeLocationToPlayerAngle * 100.f;

            if (locationScore > bestLocationScore)
            {
                bestLocationScore = locationScore;
                bestFleeLocation = fleeLocation;
            }

            DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), FString::SanitizeFloat(locationScore), fleeLocation, FColor::Red, 5.f, false);
        }
    }

    if (bestFleeLocation)
    {
        MoveToLocation(bestFleeLocation->GetActorLocation(), 0.5f, false, true, false, NULL, false);
        OnMoveToTarget();
    }
	double stop = FPlatformTime::Seconds();
	timeCPU_Flee = 1000 * (stop - start);
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::RotateTowards(const FVector& targetLocation)
{
    if (!targetLocation.IsZero())
    {
        FVector direction = targetLocation - GetPawn()->GetActorLocation();
        FRotator targetRotation = direction.Rotation();

        targetRotation.Yaw = FRotator::ClampAxis(targetRotation.Yaw);

        SetControlRotation(targetRotation);
    }
}

void ASDTAIController::SetActorLocation(const FVector& targetLocation)
{
    GetPawn()->SetActorLocation(targetLocation);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    if (UPathFollowingComponent* pathFollowingComponent = GetPathFollowingComponent())
    {
        if (pathFollowingComponent->HasValidPath())
        {
            const FNavPathSharedPtr path = pathFollowingComponent->GetPath();
            TArray<FNavPathPoint> pathPoints = path->GetPathPoints();

            for (int i = 0; i < pathPoints.Num(); ++i)
            {
                DrawDebugSphere(GetWorld(), pathPoints[i].Location, 10.f, 8, FColor::Yellow);

                if (i != 0)
                {
                    DrawDebugLine(GetWorld(), pathPoints[i].Location, pathPoints[i - 1].Location, FColor::Yellow);
                }
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    if (m_currentBrainLogic == EAIBrainMode::IfElse)
    {
        //finish jump before updating AI state
        if (AtJumpSegment)
            return;

        TArray<FHitResult> allDetectionHits;
        FHitResult detectionHit;

        GetDetectionHits(allDetectionHits, detectionHit);

        UpdatePlayerInteractionBehavior(detectionHit, deltaTime);
    }
    else if (m_currentBrainLogic == EAIBrainMode::BehaviorTree)
    {
        StartBehaviorTree(GetPawn());
    }

    if (GetMoveStatus() == EPathFollowingStatus::Idle)
    {
        m_ReachedTarget = true;
    }

    FString debugString = "";
    FColor  stringColor;
	if (m_inGroup)
	{
		DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 100.f), 15.0f, 32, FColor::Black);
	}
	/*if (AiAgentGroupManager::GetInstance()->IsAIAgentInGroup(this)) {
		DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 50.f), 15.0f, 32, FColor::Purple);
	}*/
    switch (m_PlayerInteractionBehavior)
    {
    case PlayerInteractionBehavior_Chase:
		debugString = "Chase";
        stringColor = FColor::Orange;
		break;
    case PlayerInteractionBehavior_Flee:
        debugString = "Flee";
        stringColor = FColor::Purple;
        break;
    case PlayerInteractionBehavior_Collect:
        debugString = "Collect";
        stringColor = FColor::Green;
        break;
    }

    //DrawDebugSphere(GetWorld(), GetPawn()->GetActorLocation() + FVector(0.f, 0.f, 150.f), 15.0f, 32, stringColor);
    DrawDebugString(GetWorld(), FVector(0.f, 0.f, 5.f), debugString, GetPawn(), stringColor, 0.f, false);
}

bool ASDTAIController::HasLoSOnHit(const FHitResult& hit)
{
    if (!hit.GetComponent())
        return false;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

    FVector hitDirection = hit.ImpactPoint - hit.TraceStart;
    hitDirection.Normalize();

    FHitResult losHit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(hit.GetActor());

    GetWorld()->LineTraceSingleByObjectType(losHit, hit.TraceStart, hit.ImpactPoint + hitDirection, TraceObjectTypes, queryParams);

    return losHit.GetActor() == nullptr;
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}


// To be replaced with BT logic (called line 358)
ASDTAIController::PlayerInteractionBehavior ASDTAIController::GetCurrentPlayerInteractionBehavior(const FHitResult& hit)
{
    if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Collect)
    {
        if (!hit.GetComponent())    // si on ne voit rien
            return PlayerInteractionBehavior_Collect;

        if (hit.GetComponent()->GetCollisionObjectType() != COLLISION_PLAYER)   // si ce qu'on voit c'est pas le player
            return PlayerInteractionBehavior_Collect;

        if (!HasLoSOnHit(hit))      // si ce qui nous interesse n'est pas visible
            return PlayerInteractionBehavior_Collect;

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
    else
    {
        PlayerInteractionLoSUpdate();

        return SDTUtils::IsPlayerPoweredUp(GetWorld()) ? PlayerInteractionBehavior_Flee : PlayerInteractionBehavior_Chase;
    }
}

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
            else if(component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::UpdatePlayerInteractionBehavior(const FHitResult& detectionHit, float deltaTime)
{
    PlayerInteractionBehavior currentBehavior = GetCurrentPlayerInteractionBehavior(detectionHit); // replace with BT logic

    if (currentBehavior != m_PlayerInteractionBehavior)
    {
        m_PlayerInteractionBehavior = currentBehavior;
        AIStateInterrupted();
    }
}

// AJOUTS

/*
Sets m_IsPlayerDetected = true if player is in sight and in range
*/
void ASDTAIController::DetectPlayer()
{
	double start = FPlatformTime::Seconds();
    m_isPlayerDetected = false;

    TArray<FHitResult> allDetectionHits;
    FHitResult detectionHit;

    GetDetectionHits(allDetectionHits, detectionHit);
    if (UPrimitiveComponent* component = detectionHit.GetComponent())
    {
        if (component->GetCollisionObjectType() == COLLISION_PLAYER)
        {
            bool canSeePlayer = false;
            FVector npcPosition = GetPawn()->GetActorLocation();
            m_playerPos = detectionHit.GetActor()->GetActorLocation();

            FVector distToTarget = npcPosition - m_playerPos;
            if (distToTarget.Size() < 15000.0f)
            {
                FVector npcHead = npcPosition + FVector::UpVector * 200.0f;
                FVector playerHead = m_playerPos + FVector::UpVector * 200.0f;
                FVector playerTorso = m_playerPos + FVector::UpVector * 100.0f;
                FVector playerFeet = m_playerPos + FVector::UpVector * 25.0f;

                UWorld* npcWorld = GetWorld();
                int bodyPartSeen = 0;

                if (!SDTUtils::Raycast(npcWorld, npcHead, playerHead))
                {
                    //DrawDebugLine(npcWorld, npcHead, playerHead, FColor::Blue, false, 0.066f);
                    ++bodyPartSeen;
                }
                if (!SDTUtils::Raycast(npcWorld, npcHead, playerTorso))
                {
                    //DrawDebugLine(npcWorld, npcHead, playerTorso, FColor::Blue, false, 0.066f);
                    ++bodyPartSeen;
                }
                if (!SDTUtils::Raycast(npcWorld, npcHead, playerFeet))
                {
                    //DrawDebugLine(npcWorld, npcHead, playerFeet, FColor::Blue, false, 0.066f);
                    ++bodyPartSeen;
                }

                if (bodyPartSeen > 1)
                {
                    m_isPlayerDetected = true;
					AiAgentGroupManager::GetInstance()->UpdateTimeStamp(GetWorld());
                }
            }
        }
    }
	double stop = FPlatformTime::Seconds();
	timeCPU_Detection = 1000 * (stop - start);
	
}

/*
Gets all hits in range and gets highest priority hit among them
*/
void ASDTAIController::GetDetectionHits(TArray<FHitResult>& allDetectionHits, FHitResult& detectionHit)
{
    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

/*
Sets m_PlayerInteractionBehavior to selected behavior
*/
void ASDTAIController::SetBehavior(ASDTAIController::PlayerInteractionBehavior currentBehavior)
{
    if (m_PlayerInteractionBehavior == PlayerInteractionBehavior_Flee)
    {
        UpdateTimer();
        return;
    }

    if (currentBehavior == PlayerInteractionBehavior_Chase)
    {
        ClearTimer();
    }

    if (m_PlayerInteractionBehavior != currentBehavior)
    {
        m_PlayerInteractionBehavior = currentBehavior;
        AIStateInterrupted();
    }
}


/*
Updates if player is seen by group
*/
void ASDTAIController::GroupDetectPlayer()
{
	AiAgentGroupManager::GetInstance()->UpdatePlayerStatus(GetWorld());
}

/*
Returns if player is seen by group
*/
bool ASDTAIController::IsPlayerDetectedByGroup()
{
	return AiAgentGroupManager::GetInstance()->IsPlayerDetectedByGroup();
}

/*
Unregisters all agents of the entire group
*/
void ASDTAIController::EmptyGroup()
{
	AiAgentGroupManager::GetInstance()->UnregisterAIAgents();
}

/*
Registers agent into chase group
*/
void ASDTAIController::RegisterAgent()
{
	AiAgentGroupManager::GetInstance()->RegisterAIAgent(this);
}

/*
Clears the timer (used for IfElse Flee/Chase logic and for BT flee timer
*/
void ASDTAIController::ClearTimer()
{
    if (GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(m_PlayerInteractionNoLosTimer);
        m_PlayerInteractionNoLosTimer.Invalidate();
        DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Got LoS", GetPawn(), FColor::Red, 5.f, false);
    }
}


/*
Updates the timer and handles timer overflow
*/
void ASDTAIController::UpdateTimer()
{
    if (!GetWorld()->GetTimerManager().IsTimerActive(m_PlayerInteractionNoLosTimer))
    {
        GetWorld()->GetTimerManager().SetTimer(m_PlayerInteractionNoLosTimer, this, &ASDTAIController::OnPlayerInteractionNoLosDone, 3.f, false);
        //DrawDebugString(GetWorld(), FVector(0.f, 0.f, 10.f), "Lost LoS", GetPawn(), FColor::Red, 5.f, false);
    }
}