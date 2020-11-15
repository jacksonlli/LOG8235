// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SetPursueState.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UBTTask_SetPursueState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        ASDTAIController::PlayerInteractionBehavior currentBehavior = ASDTAIController::PlayerInteractionBehavior::PlayerInteractionBehavior_Chase;
        aiController->SetBehavior(currentBehavior);
    }

    return EBTNodeResult::Succeeded;
}

