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
		H9PlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server.  \"Type\" /start to Start Game."));

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

	UE_LOG(LogTemp, Log, TEXT("Number is : %s"), *Result);

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
	// 어떻게 칠지 모르니 lower를 사용하여 통일
	FString InStartStringLower = InStartString.ToLower();
	//UE_LOG(LogTemp, Log, TEXT("InStartStringLower"));
	if (InStartStringLower.Contains(TEXT("start")))
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

	isStart = false;
}

void AH9GameModeBase::PrintChatMessageString(AH9PlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 게임 시작 관리
	if (IsGuessStartString(InChatMessageString) == true)
	{
		//UE_LOG(LogTemp, Log, TEXT("IsGuessStartString true"));
		StartGame();
		return;
	}

	FString ChatMessageString = InChatMessageString;
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);



	if (IsGuessNumberString(GuessNumberString) == true)
	{
		UE_LOG(LogTemp, Log, TEXT("IsGuessNumberString true"));
		if (InGamePlayerControllers.Num() != 0)
		{
			int32 PlayerIndex = CurrentTurnIndex % InGamePlayerControllers.Num();
			AH9PlayerController* CurrentTurnPC = InGamePlayerControllers[PlayerIndex];

			// 플레이 가능 컨트롤러 확인 및 채팅 친 플레이어 로그 확인
			UE_LOG(LogTemp, Log, TEXT("CurrentTurnPC Name :: %s"), *CurrentTurnPC->GetName());
			UE_LOG(LogTemp, Log, TEXT("InChattingPlayerController Name :: %s"), *InChattingPlayerController->GetName());

			if (!IsValid(CurrentTurnPC)) return;
			if (CurrentTurnPC != InChattingPlayerController) return;

			UE_LOG(LogTemp, Log, TEXT("InGamePlayerControllers[CurrentTurnIndex true"));

			// 입력 확인 시 타이머 초기화 및 재시작
			ResetTimer();
			StartTimer();
			// 턴 확인 변수 증가
			++CurrentTurnIndex;
			PlayerIndex = CurrentTurnIndex % InGamePlayerControllers.Num();

			//누구의 턴인지 NotificationText를 수정하여 모두에게 확인
			AH9PlayerState* H9PS = InGamePlayerControllers[PlayerIndex]->GetPlayerState<AH9PlayerState>();
			for (const auto& H9PlayerController : InGamePlayerControllers)
			{
				if (IsValid(H9PS) == true)
				{
					FString CombinedMessageString = H9PS->PlayerNameString + TEXT("'s Turn.");
					H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);
				}
			}

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
	// 이미 시작된 게임이라면 시작 불가
	if (isStart) return;
	isStart = true;

	// 방에 들어온 플레이어와 게임 중인 플레이어 구분
	for (const auto& H9PlayerController : AllPlayerControllers)
	{
		InGamePlayerControllers.Add(H9PlayerController);
		UE_LOG(LogTemp, Log, TEXT("H9PlayerController Name :: %s"), *H9PlayerController.GetName());
	}

	// 방에 들어온 플레이어 구분이 끝나면 숫자 생성
	SecretNumberString = GenerateSecretNumber();

	// 게임 시작과 함께 누구의 턴인지 NotificationText으로 알림
	AH9PlayerState* H9PS = InGamePlayerControllers[0]->GetPlayerState<AH9PlayerState>();
	for (const auto& H9PlayerController : InGamePlayerControllers)
	{
		if (IsValid(H9PS) == true)
		{
			FString CombinedMessageString = H9PS->PlayerNameString + TEXT("'s Turn.");
			H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);
		}
	}

	// 타이머 시작
	StartTimer();
}

void AH9GameModeBase::ResetGame()
{
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
	isStart = false;

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
				FString CombinedMessageString = H9PS->PlayerNameString + TEXT(" has won the game.  \"Type\" /start to Start Game.");
				H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);

				ResetTimer();
				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& H9PlayerController : InGamePlayerControllers)
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
				H9PlayerController->NotificationText = FText::FromString(TEXT("Draw...  \"Type\" /start to Start Game."));

				ResetGame();
			}
		}
	}
}

void AH9GameModeBase::StartTimer()
{
	// 타이머 최대 시간을 BP에서 수정 가능하게 MaxTimerCount를 이용
	TimerCount = MaxTimerCount;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, FTimerDelegate::CreateLambda([&]
			{
				// 타이머가 시작되면 게임 중인 플레이어의 타이머 수정
				for (const auto& H9PlayerController : InGamePlayerControllers)
				{
					FString CombinedMessageString = TEXT("Timer : ") + FString::FromInt(TimerCount);
					H9PlayerController->TimerText = FText::FromString(CombinedMessageString);
				}
				--TimerCount;
				EndTimer();
			}
		), 1.0f, true, 0.0f);

}

void AH9GameModeBase::ResetTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void AH9GameModeBase::EndTimer()
{
	if (TimerCount == 0)
	{
		// 타이머가 종료될 시 현재 플레이어의 추측 횟수 증가
		int32 PlayerIndex = CurrentTurnIndex % InGamePlayerControllers.Num();
		IncreaseGuessCount(InGamePlayerControllers[PlayerIndex]);

		// 턴 넘어감
		++CurrentTurnIndex;
		PlayerIndex = CurrentTurnIndex % InGamePlayerControllers.Num();

		// 이후 누구의 턴인지 플레이어들에게 알림
		AH9PlayerState* H9PS = InGamePlayerControllers[PlayerIndex]->GetPlayerState<AH9PlayerState>();
		for (const auto& H9PlayerController : InGamePlayerControllers)
		{
			if (IsValid(H9PS) == true)
			{
				FString CombinedMessageString = TEXT("TIME OVER! ") + H9PS->PlayerNameString + TEXT("'s Turn.");
				H9PlayerController->NotificationText = FText::FromString(CombinedMessageString);
			}
		}
		ResetTimer();
		StartTimer();
	}

}
