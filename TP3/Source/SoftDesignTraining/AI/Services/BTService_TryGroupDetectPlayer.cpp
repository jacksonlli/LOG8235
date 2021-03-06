// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryGroupDetectPlayer.h"
#include "SoftDesignTraining/SDTBaseAIController.h"
#include "SoftDesignTraining/SDTAIController.h"
#include "SoftDesignTraining/SDTUtils.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_TryGroupDetectPlayer::UBTService_TryGroupDetectPlayer()
{
	bCreateNodeInstance = true;
}

/*
It's more like a service to know if Player has been lost by the group
*/
void UBTService_TryGroupDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (ASDTBaseAIController* aiBaseController = Cast<ASDTBaseAIController>(OwnerComp.GetAIOwner()))
	{
		if (ASDTAIController* aiController = Cast<ASDTAIController>(aiBaseController))
		{
			aiController->GroupDetectPlayer();
			//Trigger from service the detect
			if (aiController->IsPlayerDetectedByGroup())
			{
				//write to bb that the player is seen by the group
				//OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiBaseController->GetIsPlayerDetectedByGroupBBKeyID(), true);
			}
			else
			{
				//write to bb that the player has been lost by all the group
				OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(aiBaseController->GetIsPlayerDetectedByGroupBBKeyID(), false);
			}
		}
	}
}


