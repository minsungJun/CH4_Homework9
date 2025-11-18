// Fill out your copyright notice in the Description page of Project Settings.


#include "H9PlayerController.h"
#include "H9UserWidget.h"
#include "Homework9.h"
#include "H9GameModeBase.h"
#include "H9PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"



AH9PlayerController::AH9PlayerController()
{
	bReplicates = true;
}

void AH9PlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void AH9PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}


	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UH9UserWidget>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}

	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
}


void AH9PlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		// ServerRPCPrintChatMessageString(InChatMessageString);

		AH9PlayerState* H9PS = GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			//FString CombinedMessageString = H9PS->PlayerNameString + TEXT(": ") + InChatMessageString;
			FString CombinedMessageString = H9PS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}



void AH9PlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);

	FString NetModeString = ChatXFunctionLibrary::GetNetModeString(this);
	FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);
}



void AH9PlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void AH9PlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	// for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
	// {
	// 	ACXPlayerController* CXPlayerController = *It;
	// 	if (IsValid(CXPlayerController) == true)
	// 	{
	// 		CXPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
	// 	}
	// }

	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		AH9GameModeBase* H9GM = Cast<AH9GameModeBase>(GM);
		if (IsValid(H9GM) == true)
		{
			H9GM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}


