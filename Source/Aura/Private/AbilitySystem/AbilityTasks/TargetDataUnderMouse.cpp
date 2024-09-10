// Copyright GH Indie Developments


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// create local object of type UTargetDataUnderMouse so can be instanced from blueprints
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);


	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	// getting the valid data for the target under the cursor
	
	// first check if we are  or on the server (single player) or locally controlled (client in multiplayer)
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();

	if (bIsLocallyControlled)
	{
		// call function below this one to send data up from client to server
		SendMouseCursorData();
	}
	else
	{
		// get spec handle & prediction key so dont have to call functions multiple times here
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

		// get data sent up to the server from the available delegate(L_118)
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicateCallback);
		
		// check if data has already been sent 
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

		// tell server to wait for data if not already sent
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}	
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// this is for sending data to the server in myltiplayer

	// Create a scoped prediction wondow
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	// Get the location under the mouse cursor (get target player controller to get the location data)
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);

	// need to make a target data handle
	FGameplayAbilityTargetDataHandle DataHandle;

	// make an object holding the target data 
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	
	// store or set the data
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	// function also requires a FGameplay Tag application tag
	FGameplayTag ApplicationTag;
	
	AbilitySystemComponent->ServerSetReplicatedTargetData(GetAbilitySpecHandle(), 
		                                                  GetActivationPredictionKey(), 
		                                                  DataHandle, 
		                                                  ApplicationTag, 
		                                                  AbilitySystemComponent->ScopedPredictionKey);
	
	//Broadcast the ValidData delegate after checking ability is still active
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}

}

void UTargetDataUnderMouse::OnTargetDataReplicateCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());

	//Broadcast the ValidData delegate after checking ability is still active
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
