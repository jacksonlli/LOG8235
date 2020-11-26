// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsAgentInGroup.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"


EBTNodeResult::Type UBTTask_IsAgentInGroup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Bool>(aiController->GetIsAgentInGroupBBKeyID()))//aiController->IsAgentInGroup())
        {
            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}