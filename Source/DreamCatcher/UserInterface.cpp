// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UUserInterface::SetMessage(const FString& Text)
{
	if (PlayerDialogTextBlock == nullptr)return;
	PlayerDialogTextBlock->SetText(FText::FromString(Text));
}

void UUserInterface::SetCharacterName(const FString& Text)
{
	if (CharacterNameText == nullptr) return;
	CharacterNameText->SetText(FText::FromString(Text));
}

void UUserInterface::OnTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

void UUserInterface::AnimateMessage(const FString& Text)
{
	CurrentState = 1;

	InitialMessage = Text;

	OutputMessage = "";

	iLetter = 0;

	PlayerDialogTextBlock->SetText(FText::FromString(""));

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnAnimationTimerCompleted, DelayBetweenLetters, false);
}


void UUserInterface::Interact()
{
    if (CurrentState == 1) // The text is being animated, skip
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        PlayerDialogTextBlock->SetText(FText::FromString(InitialMessage)); // 현재 대사 바로 표시
        CurrentState = 2;
    }
    else if (CurrentState == 2) // Text completed
    {
        // Get next message
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // 같은 npc의 다음 대사
        {
            MessageIndex += 1;
            
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else // npc 대사 끝남
        {
            PlayerDialogTextBlock->SetText(FText::FromString(""));
            CharacterNameText->SetText(FText::FromString("PALL AND PALLING"));

            if (Dialogue[RowIndex]->MessageOptions.Num() > 0) // 플레이어 응답 있으면
            {
                OnResetOptions();
                //NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num(); // 응답 개수
                //SelectedReply = 0; // 플레이어가 선택한 응답 초기화
                for (int i = 0; i < Dialogue[RowIndex]->MessageOptions.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->MessageOptions[i].OptionText); //응답 보이게 함
                }

                SelectedOption = 0;

                OnHighLightOption(SelectedOption);

                CurrentState = 3; // 플레이어 응답 기다림
            }
            else // 플레이어의 응답이 존재하지 않으면
            {
                //
                //RowIndex += 1; // 다음 대사 행

                //if ((RowIndex >= 0) && (RowIndex < Dialogue.Num())) // 다음 npc 대사
                //{
                //    MessageIndex = 0;
                //    DialogueEvents();
                //}
                //else // 플레이어 응답 없고 다음 npc 대사도 없으면 대화 종료
                //{
                //    bCanStartDialogue = false; // (수동으로) 대화 시작 못함
                //    MainPlayerController->RemoveDialogueUI(); // 대화창 없어짐
                //    CurrentState = 0; // 상태 초기화
                //}

                CurrentState = 0;
                OnAnimationHideMessageUI();
            }
        }
    }
    else if (CurrentState == 3) // 플레이어 응답 선택한 상태
    {
        // 플레이어 응답에 따라 RowIndex 바뀜
        RowIndex = Dialogue[RowIndex]->MessageOptions[SelectedOption].AnswerIndex;
        OnResetOptions(); // 응답 리셋

        if ((RowIndex >= 0) && (RowIndex < Dialogue.Num())) // npc 대사 있으면
        {
            PlayerDialogTextBlock->SetText(FText::FromString(""));
            MessageIndex = 0;
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else // npc 대사 없으면 대화 종료
        {
            CurrentState = 0;
            OnAnimationHideMessageUI();
        }
    }
}

void UUserInterface::OnAnimationTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	OutputMessage.AppendChar(InitialMessage[iLetter]);

	PlayerDialogTextBlock->SetText(FText::FromString(OutputMessage));

	if (SoundCueMessage != nullptr)
	{
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, SoundCueMessage, 0.5f);
	}

	if ((iLetter + 1) < InitialMessage.Len())
	{
		iLetter = iLetter + 1;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnAnimationTimerCompleted, DelayBetweenLetters, false);
	}
	else
	{
		CurrentState = 2;
	}
}

void UUserInterface::InitializeDialogue(class UDataTable* DialogueTable)
{
	CurrentState = 0;

	CharacterNameText->SetText(FText::FromString(""));
	PlayerDialogTextBlock->SetText(FText::FromString(""));

	OnResetOptions();

	for (auto it : DialogueTable->GetRowMap())
	{
		FNPCDialogue* Row = (FNPCDialogue*)it.Value;

		Dialogue.Add(Row); // 새로운 대사 행들 추가
	}

	if (Dialogue.Num() > 0)
	{
		CurrentState = 0;

		RowIndex = 0;

		CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

		if (Dialogue[RowIndex]->Messages.Num() > 0)
		{
			MessageIndex = 0;

			OnAnimationShowMessageUI();

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnTimerCompleted, 0.5f, false);
		}
	}
}


void UUserInterface::OnSelectUpOption()
{
    if (CurrentState != 3)return;

    if ((SelectedOption - 1) >= 0)
    {
        SelectedOption -= 1;
        OnHighLightOption(SelectedOption);
    }
}

void UUserInterface::OnSelectDownOption()
{
    if (CurrentState != 3)return;

    if ((SelectedOption + 1) < Dialogue[RowIndex]->MessageOptions.Num())
    {
        SelectedOption += 1;
        OnHighLightOption(SelectedOption);
    }
}