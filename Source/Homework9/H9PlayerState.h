// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "H9PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HOMEWORK9_API AH9PlayerState : public APlayerState
{
	GENERATED_BODY()
		

public:
	AH9PlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetPlayerInfoString();

public:
	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;
};
