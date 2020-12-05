// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsPlayerDetectedByGroup.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UBTTask_IsPlayerDetectedByGroup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        //check if the player is detected by the group
        if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>(aiController->GetIsPlayerDetectedByGroupBBKeyID()))
        {
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}