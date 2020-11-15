// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryAtJump.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


UBTService_TryAtJump::UBTService_TryAtJump()
{
    bCreateNodeInstance = true;
}


void UBTService_TryAtJump::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        //Trigger from service 
        if (aiController->IsAtJump())
        {
            //write to bb that the player is jumping
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetIsAtJumpBBKeyID(), true);
        }
        else
        {
            //write to bb that the player is not seen
            OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiController->GetIsAtJumpBBKeyID(), false);
        }
    }
}