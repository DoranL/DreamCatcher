// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"

#include "UserInterface.generated.h"




USTRUCT(BlueprintType)
struct FMessageOptions // 플레이어 응답 구조체
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OptionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AnswerIndex;

};

USTRUCT(BlueprintType)
struct FNPCDialogue : public FTableRowBase // 대화 데이터테이블 만들 때 행 구조 이거 선택해야함
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Messages; // npc 대사

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMessageOptions> MessageOptions;
};

UCLASS()
class DREAMCATCHER_API UUserInterface : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() ;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerDialogTextBlock;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterNameText;

	UPROPERTY(EditAnywhere, Category = "UI Message Settings")
	float DelayBetweenLetters = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Message Settings")
	class USoundBase* SoundCueMessage;
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnAnimationShowMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnAnimationHideMessageUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnShowMessage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnHideMessage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnResetOptions();

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation Evenets")
	void OnSetOption(int32 Option, const FText& OptionText);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class ANelia* Nelia;

	UPROPERTY(BlueprintReadWrite)
	int32 SelectedOption;

public:
	void SetMessage(const FString& Text);

	void SetCharacterName(const FString& Text);

	void InitializeDialogue(class UDataTable* DialogueTable);

	void AnimateMessage(const FString& Text);

	void OnSelectUpOption();

	void OnSelectDownOption();

	bool bTextCompleted;
	bool bAnimating;

	UFUNCTION(BlueprintCallable)
	void Interact();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue Num")
	int32 CurrentState = 0;
private:
	FTimerHandle TimerHandle;

	UFUNCTION()
	void OnTimerCompleted();

	UFUNCTION()
	void OnAnimationTimerCompleted();

	FString InitialMessage;

	FString OutputMessage;

	int32 iLetter;

	TArray<FNPCDialogue*> Dialogue;

	int32 MessageIndex;

	int32 RowIndex;
};