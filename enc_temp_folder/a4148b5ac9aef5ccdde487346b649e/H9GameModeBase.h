// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "H9GameModeBase.generated.h"

class AH9PlayerController;

UCLASS()
class HOMEWORK9_API AH9GameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void OnPostLogin(AController* NewPlayer) override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	bool IsGuessStartString(const FString& InStartString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	virtual void BeginPlay() override;

	void PrintChatMessageString(AH9PlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void IncreaseGuessCount(AH9PlayerController* InChattingPlayerController);

	void StartGame();

	void ResetGame();

	void JudgeGame(AH9PlayerController* InChattingPlayerController, int InStrikeCount);


protected:
	FString SecretNumberString;

	TArray<TObjectPtr<AH9PlayerController>> AllPlayerControllers;

	TArray<TObjectPtr<AH9PlayerController>> InGamePlayerControllers;

	int32 CurrentTurnIndex;


	
};
