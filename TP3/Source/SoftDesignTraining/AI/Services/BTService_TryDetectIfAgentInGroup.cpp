// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryDetectIfAgentInGroup.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SDTUtils.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_TryDetectIfAgentInGroup::UBTService_TryDetectIfAgentInGroup()
{
	bCreateNodeInstance = true;
}

void UBTService_TryDetectIfAgentInGroup::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (ASDTBaseAIController* aiBaseController = Cast<ASDTBaseAIController>(OwnerComp.GetAIOwner()))
	{
		if (ASDTAIController* aiController = Cast<ASDTAIController>(aiBaseController))
		{
			//Trigger from service the detect
			if (aiController->IsAgentInGroup())
			{
				//write to bb that the agent is in group
				OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiBaseController->GetIsAgentInGroupBBKeyID(), true);
			}
			else
			{
				//write to bb that the agent is in group
				OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiBaseController->GetIsAgentInGroupBBKeyID(), false);
			}
		}
	}
}


