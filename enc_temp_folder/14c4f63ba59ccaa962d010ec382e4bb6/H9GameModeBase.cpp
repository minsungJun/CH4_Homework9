// Fill out your copyright notice in the Description page of Project Settings.


#include "H9GameModeBase.h"
#include "H9GameStateBase.h"
#include "H9PlayerController.h"
#include "EngineUtils.h"
#include "H9PlayerState.h"

void AH9GameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	AH9PlayerController* H9PlayerController = Cast<AH9PlayerController>(NewPlayer);
	if (IsValid(H9PlayerController) == true)
	{
		H9PlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(H9PlayerController);

		AH9PlayerState* CXPS = H9PlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		AH9GameStateBase* H9GameStateBase = GetGameState<AH9GameStateBase>();
		if (IsValid(H9GameStateBase) == true)
		{
			H9GameStateBase->MulticastRPCBroadcastLoginMessage(CXPS->PlayerNameString);
		}
	}
}

FString AH9GameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool AH9GameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}

			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

bool AH9GameModeBase::IsGuessStartString(const FString& InStartString)
{
	// start 감지 
	// 어떻게 칠지 모르니 lower를 사용하여 통일
	FString InStartStringLower = InStartString.ToLower();

	if (InStartStringLower == "/start")
	{
		return true;
	}

	return false;
}

FString AH9GameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void AH9GameModeBase::BeginPlay()
{
	Super::BeginPlay();


}

void AH9GameModeBase::PrintChatMessageString(AH9PlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	FString ChatMessageString = InChatMessageString;
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true && InGamePlayerControllers[CurrentTurnIndex % InGamePlayerControllers.Num()] == InChattingPlayerController)
	{
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);
		for (TActorIterator<AH9PlayerController> It(GetWorld()); It; ++It)
		{
			AH9PlayerController* H9PlayerController = *It;
			if (IsValid(H9PlayerController) == true)
			{
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				H9PlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				JudgeGame(InChattingPlayerController, StrikeCount);
			}
		}
	}
	// 게임 시작 관리
	else if (IsGuessStartString(GuessNumberString))
	{
		StartGame();
	}



	else
	{
		for (TActorIterator<AH9PlayerController> It(GetWorld()); It; ++It)
		{
			AH9PlayerController* H9PlayerController = *It;
			if (IsValid(H9PlayerController) == true)
			{
				H9PlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}


void AH9GameModeBase::IncreaseGuessCount(AH9PlayerController* InChattingPlayerController)
{
	AH9PlayerState* CXPS = InChattingPlayerController->GetPlayerState<AH9PlayerState>();
	if (IsValid(CXPS) == true)
	{
		CXPS->CurrentGuessCount++;
	}
}

void AH9GameModeBase::StartGame()
{
	//인게임 유저들 관리
	for (const auto& H9PlayerController : AllPlayerControllers)
	{
		InGamePlayerControllers.Add(H9PlayerController);
	}


	SecretNumberString = GenerateSecretNumber();
}

void AH9GameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const auto& H9PlayerController : AllPlayerControllers)
	{
		AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
		if (IsValid(H9PS) == true)
		{
			H9PS->CurrentGuessCount = 0;
		}
	}

	//인게임 관련 변수 초기화
	InGamePlayerControllers.Empty();
	CurrentTurnIndex = 0;

}

void AH9GameModeBase::JudgeGame(AH9PlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AH9PlayerState* H9PS = InChattingPlayerController->GetPlayerState<AH9PlayerState>();
		for (const auto& H9PlayerController : AllPlayerControllers)
		{
			if (IsValid(H9PS) == true)
			{
				FString CombinedMessageString = H9PS->PlayerNameString + TEXT(" has won the game.");
				H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);

				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& H9PlayerController : AllPlayerControllers)
		{
			AH9PlayerState* H9PS = H9PlayerController->GetPlayerState<AH9PlayerState>();
			if (IsValid(H9PS) == true)
			{
				if (H9PS->CurrentGuessCount < H9PS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& H9PlayerController : AllPlayerControllers)
			{
				H9PlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				ResetGame();
			}
		}
	}
}
