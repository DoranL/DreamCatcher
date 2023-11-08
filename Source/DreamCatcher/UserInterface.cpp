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

//월드 타이머를 초기화 해주고 첫 대화의 첫 대사를 AnimateMessage에 매개변수로 전달
void UUserInterface::OnTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

// CurrentState는 1 첫 번째 대화창 출력 InitialMessage = text 기존 적혀있던 Text를 새로 입력하는 Text로 덮어씀
// OutputMessage는 출력할 메세지 변수
// iLetter은 글자를 한 글자씩 출력할 때 쓰는 int32형태의 변수 길이를 측정하고 초기 길이보다 짧으면 계속해서 입력받는 형식
// 맨 마지막 줄이 한 글자 입력 시간을 조절하는 DelayBetweenLetters 값을주고 OnAnimationTimerCompleted가 값이 입력되는 애니메이션 실행동안인듯함 
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
    //맵에서 0번째 캐릭터를 가져와서 nelia에 넣어주고 가져온 캐릭터의 컨트롤러를 형변환 후 MainPlayerController에 넣어줌
    Nelia = Cast<ANelia>(UGameplayStatics::GetPlayerCharacter(this, 0));
    MainPlayerController = Cast<AMainPlayerController>(Nelia->GetController());
    
    //PlayerDialogueTextBlock 	UPROPERTY(meta = (BindWidget)) 생성할 때 bindwidget 해서 dialoguewidget에 생성한 playerdialoguetextblock이랑 연결해주고
    //이부분이 살짝 애매한데 SetText 이부분을 그럼 위젯이랑 연결해줬으니까 위젯 그래프에 있는 settext를 통해서 현재 대사를 넣어줌
    if (CurrentState == 1) // npc의 row 내에 있는 메시지의 해당하는 대사의 한 글자만 출력되고 있는 상태 즉 안녕하세요 출력 시 안 또는 안녕 또는 안녕하... (안녕하세요 경우는 Currentstate ==2)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        PlayerDialogTextBlock->SetText(FText::FromString(InitialMessage)); // 현재 대사를 출력 중 마우스 왼쪽 클릭 시 모든 내용을 즉시 출력한다.
        CurrentState = 2;
    }
    /// <summary>
    /// 처음 대사 이후 currentstate 2가 되고 
    /// ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) 이부분이 뭔가 데이터 테이블에 있는 각 로우값에 해당하는 Messages 최대 개수 보다 적으면
    /// 이게 의미하는 게 아마 row 당 내 데이터 테이블을 봐도 row1 은 메세지 2개 row2,3를 1개 씩인데 로우에 포함된 메세지를 전부 출력한 것이 아닌 경우 
    /// MessageIndex +!을 해주고 AnimateMessage(Dialogue[Rowindex]->Message[MessageIndex].ToString() 첫 번째 대화 이후 text를 AnimateMessage함수에 text에 넣어 출력해줌 
    /// 그러고 나서 interact 첫번째 부분부터 다시 수행하는 듯 그러고 나서 row1에 두번째 메세지를 출력하고 나서 messageindex가 message 최대 인덱스 보다 커지니까 else문 수행
    /// </summary>
    else if (CurrentState == 2) // npc의 한 row 내에 있는 메시지의 해당하는 대사가 모두 화면에 출력된 경우
    {
        // Get next message 데이터 테이블에서 Messages가 더 남아 있는 경우를 의미
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // 같은 npc의 다음 대사
        {
            MessageIndex += 1;
            
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        /// <summary>
        /// 대사가 모두 화면에 출력된 경우 위젯에서 설정해준 PlayerDialogueTextBlock을 비워주고
        /// 해당 대사(npc) 행의 대한 플레이어의 답변이 있는지 확인 
        /// </summary>
        else // npc 대사 끝남 
        { 
            PlayerDialogTextBlock->SetText(FText::FromString(""));

            if (Dialogue[RowIndex]->MessageOptions.Num() > 0) // 플레이어 응답 있으면 위젯에서 이름을 나타내는 창에 플레이어 이름을 입력
            {
                CharacterNameText->SetText(FText::FromString("Pelia"));

                OnResetOptions();

                SelectedOption = 0;

                for (int i = 0; i < Dialogue[RowIndex]->MessageOptions.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->MessageOptions[i].OptionText); //응답 보이게 함
                }
                
                CurrentState = 3; // npc의 대사 종료 이후 플레이어의 option 선택 창을 띄운 상태로 아직 플레이어가 option을 선택하지 않은 상태
            }
            // 플레이어의 응답이 존재하지 않으면 CurrentState를 0으로 dialogue 창을 숨김 상태로 하고 RemoveDialogue() 수행 
            // RemoveDialogue는 bShowMouseCursor = false;, bDialogueVisible = false;, SetCinematicMode(false, true, true); 선택 가능했던 마우스 커서를 사라지게 하고 
            // 불변수를 false로 setCinematicMode는 대화창 수행 도중에 막아놨던 화면 전환 및 입력키를 다시 풀어주는 역할
            else 
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
    else if (CurrentState == 3) // 플레이어 응답지를 설정한 상태 즉 대화창에서 npc에 대한 답변을 선택할 수 있는 경우 들어가는 파트
    {
        //데이터 테이블 상에서 보면 MessageOption에 Answer index가 있는데 이 변수는 플레이어가 대사 선택 가능한 상태에서 한 대사를 클릭 시 해당 숫자를 RowIndex로 넣어줌으로써 예를 들어 2일 경우 
        //배열이기 때문에 3번째 문장인 row3로 이동한다.
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

/// <summary>
/// 우선 타이머를 초기화하고 OutputMessage.AppendChar(InitialMessage[iLetter]); AppendChar가 한 글자씩 추가해줌
/// SoundCueMessage가 null 비어있지 않으면 SoundCueMessage를 기존 속도에 1/2 속도로 play한다
/// 그리고 만약 입력해야할 문자열보다 초기화된(확장된) 메세지의 길이보다 짧으면 계속해서 문자를 입력받고
/// OnAnimationTimerCompleted() 함수를 DelayBetweenLetters초 간격으로 수행함
/// 그러고 나서 다 입력 받았다면 CurrentState를 2로 지정해줌 
/// </summary>
void UUserInterface::OnAnimationTimerCompleted()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	/// <summary>
	/// OutputMessage에 한 글자씩 추가해줌 예를 들어 안녕하세요를 출력하고 싶을 때 아래 문장 수행 시 안 한 글자만 입력되고 화면상 출력 x
    /// PlayerDialogTextBlock을 setText 시 위젯 상에 지정해둔 텍스트 박스에 안이 화면상에 출력됨
	/// </summary>
	OutputMessage.AppendChar(InitialMessage[iLetter]);

	PlayerDialogTextBlock->SetText(FText::FromString(OutputMessage));

	if (SoundCueMessage != nullptr)
	{
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, SoundCueMessage, 0.5f);
	}
    //출력해야할 한 행의 글자수가 남아있는 경우
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
/// currentstate =0, 대화창에 들어가 있는 캐릭터 이름과 해당 캐릭터의 대화내용을 "" 초기화 추가적으로 OnResetOption()함수도 초기화
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
        //FNPCDDialogue* Row = FNPCDialogue* it.value 데이터 테이블 상 1번 행 전체를 FNCDialogue 형으로 변환한 후 Row값에 넣어줌
		FNPCDialogue* Row = (FNPCDialogue*)it.Value;

		Dialogue.Add(Row); // 새로운 대사 행들 추가
	}

	/// <summary>
	/// 위에서 Dialogue를 추가해줬는데 만약에 대화가 없으면 0일테니까 대화가 없는 것으로
    /// 판단 0보다 크다면 위에서 대화를 추가한 것으로 판단 진행할 대화가 있다
    /// 그럴 경우 CurrentState = 0; RowIndex = 0; CharacterNameText도 0번째 부터 초기화 및 수행 가능하도록 설정
	/// </summary>
	/// <param name="DialogueTable"></param>
	if (Dialogue.Num() > 0)
	{
		//CurrentState = 0;

		RowIndex = 0;

		CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

		/// <summary>
		/// 추가한 대화에 Messages.Num 즉 다음 대사가 있으면 
        /// Message.Num이 0보다 크면 다시 다음 대화 첫 대화부터 시작할 수 있도록 0으로 초기화 이후 대화창을 
        /// 볼 수 있도록 OnAnimationShowMessageUi();
		/// </summary>
		/// <param name="DialogueTable"></param>
		if (Dialogue[RowIndex]->Messages.Num() > 0)
		{
			MessageIndex = 0;

			OnAnimationShowMessageUI();

			/// <summary>
			/// 글씨를 한 글자씩 출력하기 위해 타이머로 텀주기
            /// 0.5초 뒤에 OnTimerCompleted 실행
			/// </summary>
			/// <param name="DialogueTable"></param>
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UUserInterface::OnTimerCompleted, 0.5f, false);
		}
	}
}

/// <summary>
/// 방향키 w,s를 통해 대화를 위아래로 선택 가능 
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