// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryDetectPlayer.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SDTUtils.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_TryDetectPlayer::UBTService_TryDetectPlayer()
{
    bCreateNodeInstance = true;
}

void UBTService_TryDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        //Trigger from service the detect
        aiController->DetectPlayer();
        if (aiController->IsTargetPlayerSeen())
        {
            //write to bb that the player is seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetTargetSeenBBKeyID(), true);

            //write to bb the position of the target
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(aiController->GetTargetPosBBKeyID(), aiController->GetTargetPlayerPos());

            if (SDTUtils::IsPlayerPoweredUp(GetWorld()))
            {
                //write to bb that the player is powered up
                OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetIsPlayerPoweredUpBBKeyID(), true);
            }
            else
            {
                //write to bb that the player is not powered up
                OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetIsPlayerPoweredUpBBKeyID(), false);
            }
        }
        else
        {
            //write to bb that the player is not seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetTargetSeenBBKeyID(), false);
        }
    }
}


