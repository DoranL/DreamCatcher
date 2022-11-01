// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface.h"
#include "Components/TextBlock.h"

void UUserInterface::SetMessage(const FString& Text)
{
	if (PlayeerDialogTextBlock == nullptr)return;
	PlayeerDialogTextBlock->SetText(FText::FromString(Text));
}

void UUserInterface::SetCharacterName(const FString& Text)
{
	if (CharacterNameText == nullptr) return;
	CharacterNameText->SetText(FText::FromString(Text));
}