// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "H9GameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK9_API AH9GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));
	
};
