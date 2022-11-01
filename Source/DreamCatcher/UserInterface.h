// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserInterface.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API UUserInterface : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayeerDialogTextBlock;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnAnimationShowMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnAnimationHideMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnShowMessage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnHideMessage();


public:
	void SetMessage(const FString& Text);

	void SetCharacterName(const FString& Text);
};
