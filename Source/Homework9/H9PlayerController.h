// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "H9PlayerController.generated.h"

class UH9UserWidget;

UCLASS()
class HOMEWORK9_API AH9PlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	AH9PlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UH9UserWidget> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UH9UserWidget> ChatInputWidgetInstance;

	FString ChatMessageString;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TimerTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> TimerTextWidgetInstance;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText TimerText;
};
