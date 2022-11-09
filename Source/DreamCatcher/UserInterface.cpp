// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "MainPlayerController.h"
#include "Nelia.h"

void UUserInterface::BeginPlay()
{
   
}

//해당 SetMessage와 SetCharacterName을 실직적으로 호출한 부분은 없음 
//SetMessage는 대화창의 대화 블록이 비어있음 return을 하고 전달된 문자열을 FText형태로 대화 블록에 지정해줌
void UUserInterface::SetMessage(const FString& Text)
{
	if (PlayerDialogTextBlock == nullptr)return;
	PlayerDialogTextBlock->SetText(FText::FromString(Text));
}
 
//캐릭터 이름 기입칸이 비었을 경우 return 하고 위와 동일하게 전달받은 text 값을 FTEXT 형태로 지정해줌
void UUserInterface::SetCharacterName(const FString& Text)
{
	if (CharacterNameText == nullptr) return;
	CharacterNameText->SetText(FText::FromString(Text));
}

//월드 타이머를 초기화 해주고 대화창 RowIndex 번째 메세지를 보여지게 함?
void UUserInterface::OnTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

// CurrentState는 1 첫 번째 대화창 출력 InitialMessage = text 기존 적혀있던 Text를 새로 입력하는 Text로 덮어씀
// OutputMessage는 뭐 문자를 출력한다 이런 거 같은데 .....................
// iLetter은 글자를 한 글자씩 출력할 때 쓰는 int32형태의 변수 길이를 측정하고 초기 길이보다 짧으면 계속해서 입력받는 형식
// 맨 마지막 줄이 한 글자 입력 시간을 조절하는 DelayBetweenLetters 값을주고 OnAnimationTimerCompleted가 값이 입력되는 애니메이션 실행동안인듯함 
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
    //맵에서 0번째 캐릭터를 가져와서 nelia에 넣어주고 가져온 캐릭터의 컨트롤러를 형변환 후 MainPlayerController에 넣어줌
    Nelia = Cast<ANelia>(UGameplayStatics::GetPlayerCharacter(this, 0));
    MainPlayerController = Cast<AMainPlayerController>(Nelia->GetController());

    //CurrentState 값을 1로 바꿔주는 부분이 블루프린트에 있는 OnResetOption인감
    //타이머를 해제하고 
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
            CharacterNameText->SetText(FText::FromString("Pelia"));

            if (Dialogue[RowIndex]->MessageOptions.Num() > 0) // 플레이어 응답 있으면
            {
                OnResetOptions();
                //NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num(); // 응답 개수
                //SelectedReply = 0; // 플레이어가 선택한 응답 초기화
                for (int i = 0; i < Dialogue[RowIndex]->MessageOptions.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->MessageOptions[i].OptionText); //응답 보이게 함
                    UE_LOG(LogTemp, Warning, TEXT("for"));
                }

                SelectedOption = 0;
                
                
                //마우스나 키보드로 현재 선택중인 옵션 선택하는 대화창에서 선택가능한 사항 있을 경우? 
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
                MainPlayerController->RemoveDialogue();
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
            MainPlayerController->RemoveDialogue();
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

//대화창
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