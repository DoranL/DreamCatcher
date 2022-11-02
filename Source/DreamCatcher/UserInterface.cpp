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
        PlayerDialogTextBlock->SetText(FText::FromString(InitialMessage)); // ���� ��� �ٷ� ǥ��
        CurrentState = 2;
    }
    else if (CurrentState == 2) // Text completed
    {
        // Get next message
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // ���� npc�� ���� ���
        {
            MessageIndex += 1;
            
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else // npc ��� ����
        {
            PlayerDialogTextBlock->SetText(FText::FromString(""));
            CharacterNameText->SetText(FText::FromString("PALL AND PALLING"));

            if (Dialogue[RowIndex]->MessageOptions.Num() > 0) // �÷��̾� ���� ������
            {
                OnResetOptions();
                //NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num(); // ���� ����
                //SelectedReply = 0; // �÷��̾ ������ ���� �ʱ�ȭ
                for (int i = 0; i < Dialogue[RowIndex]->MessageOptions.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->MessageOptions[i].OptionText); //���� ���̰� ��
                }

                SelectedOption = 0;

                OnHighLightOption(SelectedOption);

                CurrentState = 3; // �÷��̾� ���� ��ٸ�
            }
            else // �÷��̾��� ������ �������� ������
            {
                //
                //RowIndex += 1; // ���� ��� ��

                //if ((RowIndex >= 0) && (RowIndex < Dialogue.Num())) // ���� npc ���
                //{
                //    MessageIndex = 0;
                //    DialogueEvents();
                //}
                //else // �÷��̾� ���� ���� ���� npc ��絵 ������ ��ȭ ����
                //{
                //    bCanStartDialogue = false; // (��������) ��ȭ ���� ����
                //    MainPlayerController->RemoveDialogueUI(); // ��ȭâ ������
                //    CurrentState = 0; // ���� �ʱ�ȭ
                //}

                CurrentState = 0;
                OnAnimationHideMessageUI();
            }
        }
    }
    else if (CurrentState == 3) // �÷��̾� ���� ������ ����
    {
        // �÷��̾� ���信 ���� RowIndex �ٲ�
        RowIndex = Dialogue[RowIndex]->MessageOptions[SelectedOption].AnswerIndex;
        OnResetOptions(); // ���� ����

        if ((RowIndex >= 0) && (RowIndex < Dialogue.Num())) // npc ��� ������
        {
            PlayerDialogTextBlock->SetText(FText::FromString(""));
            MessageIndex = 0;
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else // npc ��� ������ ��ȭ ����
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

		Dialogue.Add(Row); // ���ο� ��� ��� �߰�
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