// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "MainPlayerController.h"
#include "Nelia.h"
#include "NpcCharacter.h"

void UUserInterface::BeginPlay()
{
}

//�ش� SetMessage�� SetCharacterName�� ���������� ȣ���� �κ��� ���� 
//SetMessage�� ��ȭâ�� ��ȭ ����� ������� return�� �ϰ� ���޵� ���ڿ��� FText���·� ��ȭ ��Ͽ� ��������
void UUserInterface::SetMessage(const FString& Text)
{
	if (PlayerDialogTextBlock == nullptr)return;
	PlayerDialogTextBlock->SetText(FText::FromString(Text));
}
 
//ĳ���� �̸� ����ĭ�� ����� ��� return �ϰ� ���� �����ϰ� ���޹��� text ���� FTEXT ���·� ��������
void UUserInterface::SetCharacterName(const FString& Text)
{
	if (CharacterNameText == nullptr) return;
	CharacterNameText->SetText(FText::FromString(Text));
}

//���� Ÿ�̸Ӹ� �ʱ�ȭ ���ְ� ù ��ȭ�� ù ��縦 AnimateMessage�� �Ű������� ����
void UUserInterface::OnTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

// CurrentState�� 1 ù ��° ��ȭâ ��� InitialMessage = text ���� �����ִ� Text�� ���� �Է��ϴ� Text�� ���
// OutputMessage�� ����� �޼��� ����
// iLetter�� ���ڸ� �� ���ھ� ����� �� ���� int32������ ���� ���̸� �����ϰ� �ʱ� ���̺��� ª���� ����ؼ� �Է¹޴� ����
// �� ������ ���� �� ���� �Է� �ð��� �����ϴ� DelayBetweenLetters �����ְ� OnAnimationTimerCompleted�� ���� �ԷµǴ� �ִϸ��̼� ���ൿ���ε��� 
void UUserInterface::AnimateMessage(const FString& Text)
{
	CurrentState = 1;

    InitialMessage = Text.Replace(TEXT("\n"), TEXT("\n"));

	OutputMessage = "";

	iLetter = 0;

	PlayerDialogTextBlock->SetText(FText::FromString(""));

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnAnimationTimerCompleted, DelayBetweenLetters, false);
}

void UUserInterface::Interact()
{
    //�ʿ��� 0��° ĳ���͸� �����ͼ� nelia�� �־��ְ� ������ ĳ������ ��Ʈ�ѷ��� ����ȯ �� MainPlayerController�� �־���
    Nelia = Cast<ANelia>(UGameplayStatics::GetPlayerCharacter(this, 0));
    MainPlayerController = Cast<AMainPlayerController>(Nelia->GetController());
    
    //PlayerDialogueTextBlock 	UPROPERTY(meta = (BindWidget)) ������ �� bindwidget �ؼ� dialoguewidget�� ������ playerdialoguetextblock�̶� �������ְ�
    //�̺κ��� ��¦ �ָ��ѵ� SetText �̺κ��� �׷� �����̶� �����������ϱ� ���� �׷����� �ִ� settext�� ���ؼ� ���� ��縦 �־���
    if (CurrentState == 1) // npc�� row ���� �ִ� �޽����� �ش��ϴ� ����� �� ���ڸ� ��µǰ� �ִ� ���� �� �ȳ��ϼ��� ��� �� �� �Ǵ� �ȳ� �Ǵ� �ȳ���... (�ȳ��ϼ��� ���� Currentstate ==2)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        PlayerDialogTextBlock->SetText(FText::FromString(InitialMessage)); // ���� ��縦 ��� �� ���콺 ���� Ŭ�� �� ��� ������ ��� ����Ѵ�.
        CurrentState = 2;
    }
    /// <summary>
    /// ó�� ��� ���� currentstate 2�� �ǰ� 
    /// ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) �̺κ��� ���� ������ ���̺� �ִ� �� �ο찪�� �ش��ϴ� Messages �ִ� ���� ���� ������
    /// �̰� �ǹ��ϴ� �� �Ƹ� row �� �� ������ ���̺��� ���� row1 �� �޼��� 2�� row2,3�� 1�� ���ε� �ο쿡 ���Ե� �޼����� ���� ����� ���� �ƴ� ��� 
    /// MessageIndex +!�� ���ְ� AnimateMessage(Dialogue[Rowindex]->Message[MessageIndex].ToString() ù ��° ��ȭ ���� text�� AnimateMessage�Լ��� text�� �־� ������� 
    /// �׷��� ���� interact ù��° �κк��� �ٽ� �����ϴ� �� �׷��� ���� row1�� �ι�° �޼����� ����ϰ� ���� messageindex�� message �ִ� �ε��� ���� Ŀ���ϱ� else�� ����
    /// </summary>
    else if (CurrentState == 2) // npc�� �� row ���� �ִ� �޽����� �ش��ϴ� ��簡 ��� ȭ�鿡 ��µ� ���
    {
        // Get next message ������ ���̺��� Messages�� �� ���� �ִ� ��츦 �ǹ�
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // ���� npc�� ���� ���
        {
            MessageIndex += 1;
            
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        /// <summary>
        /// ��簡 ��� ȭ�鿡 ��µ� ��� �������� �������� PlayerDialogueTextBlock�� ����ְ�
        /// �ش� ���(npc) ���� ���� �÷��̾��� �亯�� �ִ��� Ȯ�� 
        /// </summary>
        else // npc ��� ���� 
        { 
            PlayerDialogTextBlock->SetText(FText::FromString(""));

            if (Dialogue[RowIndex]->MessageOptions.Num() > 0) // �÷��̾� ���� ������ �������� �̸��� ��Ÿ���� â�� �÷��̾� �̸��� �Է�
            {
                CharacterNameText->SetText(FText::FromString("Pelia"));

                OnResetOptions();

                SelectedOption = 0;

                for (int i = 0; i < Dialogue[RowIndex]->MessageOptions.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->MessageOptions[i].OptionText); //���� ���̰� ��
                }
                
                CurrentState = 3; // npc�� ��� ���� ���� �÷��̾��� option ���� â�� ��� ���·� ���� �÷��̾ option�� �������� ���� ����
            }
            // �÷��̾��� ������ �������� ������ CurrentState�� 0���� dialogue â�� ���� ���·� �ϰ� RemoveDialogue() ���� 
            // RemoveDialogue�� bShowMouseCursor = false;, bDialogueVisible = false;, SetCinematicMode(false, true, true); ���� �����ߴ� ���콺 Ŀ���� ������� �ϰ� 
            // �Һ����� false�� setCinematicMode�� ��ȭâ ���� ���߿� ���Ƴ��� ȭ�� ��ȯ �� �Է�Ű�� �ٽ� Ǯ���ִ� ����
            else 
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
                MainPlayerController->RemoveDialogue();
            }
        }
    }
    else if (CurrentState == 3) // �÷��̾� �������� ������ ���� �� ��ȭâ���� npc�� ���� �亯�� ������ �� �ִ� ��� ���� ��Ʈ
    {
        //������ ���̺� �󿡼� ���� MessageOption�� Answer index�� �ִµ� �� ������ �÷��̾ ��� ���� ������ ���¿��� �� ��縦 Ŭ�� �� �ش� ���ڸ� RowIndex�� �־������ν� ���� ��� 2�� ��� 
        //�迭�̱� ������ 3��° ������ row3�� �̵��Ѵ�.
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
            MainPlayerController->RemoveDialogue();
        }
    }
}

/// <summary>
/// �켱 Ÿ�̸Ӹ� �ʱ�ȭ�ϰ� OutputMessage.AppendChar(InitialMessage[iLetter]); AppendChar�� �� ���ھ� �߰�����
/// SoundCueMessage�� null ������� ������ SoundCueMessage�� ���� �ӵ��� 1/2 �ӵ��� play�Ѵ�
/// �׸��� ���� �Է��ؾ��� ���ڿ����� �ʱ�ȭ��(Ȯ���) �޼����� ���̺��� ª���� ����ؼ� ���ڸ� �Է¹ް�
/// OnAnimationTimerCompleted() �Լ��� DelayBetweenLetters�� �������� ������
/// �׷��� ���� �� �Է� �޾Ҵٸ� CurrentState�� 2�� �������� 
/// </summary>
void UUserInterface::OnAnimationTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	/// <summary>
	/// OutputMessage�� �� ���ھ� �߰����� ���� ��� �ȳ��ϼ��並 ����ϰ� ���� �� �Ʒ� ���� ���� �� �� �� ���ڸ� �Էµǰ� ȭ��� ��� x
    /// PlayerDialogTextBlock�� setText �� ���� �� �����ص� �ؽ�Ʈ �ڽ��� ���� ȭ��� ��µ�
	/// </summary>
	OutputMessage.AppendChar(InitialMessage[iLetter]);

	PlayerDialogTextBlock->SetText(FText::FromString(OutputMessage));

	if (SoundCueMessage != nullptr)
	{
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, SoundCueMessage, 0.5f);
	}
    //����ؾ��� �� ���� ���ڼ��� �����ִ� ���
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

/// <summary>
/// currentstate =0, ��ȭâ�� �� �ִ� ĳ���� �̸��� �ش� ĳ������ ��ȭ������ "" �ʱ�ȭ �߰������� OnResetOption()�Լ��� �ʱ�ȭ
/// </summary>
/// <param name="DialogueTable"></param>
void UUserInterface::InitializeDialogue(class UDataTable* DialogueTable)
{
	CurrentState = 0;

	CharacterNameText->SetText(FText::FromString(""));
	PlayerDialogTextBlock->SetText(FText::FromString(""));

	OnResetOptions();

	for (auto it : DialogueTable->GetRowMap())
	{
        //FNPCDDialogue* Row = FNPCDialogue* it.value ������ ���̺� �� 1�� �� ��ü�� FNCDialogue ������ ��ȯ�� �� Row���� �־���
		FNPCDialogue* Row = (FNPCDialogue*)it.Value;

		Dialogue.Add(Row); // ���ο� ��� ��� �߰�
	}

	/// <summary>
	/// ������ Dialogue�� �߰�����µ� ���࿡ ��ȭ�� ������ 0���״ϱ� ��ȭ�� ���� ������
    /// �Ǵ� 0���� ũ�ٸ� ������ ��ȭ�� �߰��� ������ �Ǵ� ������ ��ȭ�� �ִ�
    /// �׷� ��� CurrentState = 0; RowIndex = 0; CharacterNameText�� 0��° ���� �ʱ�ȭ �� ���� �����ϵ��� ����
	/// </summary>
	/// <param name="DialogueTable"></param>
	if (Dialogue.Num() > 0)
	{
		//CurrentState = 0;

		RowIndex = 0;

		CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

		/// <summary>
		/// �߰��� ��ȭ�� Messages.Num �� ���� ��簡 ������ 
        /// Message.Num�� 0���� ũ�� �ٽ� ���� ��ȭ ù ��ȭ���� ������ �� �ֵ��� 0���� �ʱ�ȭ ���� ��ȭâ�� 
        /// �� �� �ֵ��� OnAnimationShowMessageUi();
		/// </summary>
		/// <param name="DialogueTable"></param>
		if (Dialogue[RowIndex]->Messages.Num() > 0)
		{
			MessageIndex = 0;

			OnAnimationShowMessageUI();

			/// <summary>
			/// �۾��� �� ���ھ� ����ϱ� ���� Ÿ�̸ӷ� ���ֱ�
            /// 0.5�� �ڿ� OnTimerCompleted ����
			/// </summary>
			/// <param name="DialogueTable"></param>
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnTimerCompleted, 0.5f, false);
		}
	}
}

/// <summary>
/// ����Ű w,s�� ���� ��ȭ�� ���Ʒ��� ���� ���� 
/// </summary>
//void UUserInterface::OnSelectUpOption()
//{
//    if (CurrentState != 3)return;
//
//    if ((SelectedOption - 1) >= 0)
//    {
//        SelectedOption -= 1;
//        OnHighLightOption(SelectedOption);
//    }
//}
//
//void UUserInterface::OnSelectDownOption()
//{
//    if (CurrentState != 3)return;
//
//    if ((SelectedOption + 1) < Dialogue[RowIndex]->MessageOptions.Num())
//    {
//        SelectedOption += 1;
//        OnHighLightOption(SelectedOption);
//    }
//}