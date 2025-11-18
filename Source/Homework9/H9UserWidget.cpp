// Fill out your copyright notice in the Description page of Project Settings.


#include "H9UserWidget.h"

#include "Components/EditableTextBox.h"
#include "H9PlayerController.h"


void UH9UserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UH9UserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UH9UserWidget::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			AH9PlayerController* OwningH9PlayerController = Cast<AH9PlayerController>(OwningPlayerController);
			if (IsValid(OwningH9PlayerController) == true)
			{
				OwningH9PlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
