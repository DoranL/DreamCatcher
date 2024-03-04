// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Nelia.h"
#include "Kismet/GameplayStatics.h"
#include "UserInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/BlueprintGeneratedClass.h"

AMainPlayerController::AMainPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> UserInterfaceBpClass(TEXT("/Game/HUD/DialogueWidget"));
	
	if (UserInterfaceBpClass.Class != nullptr)
	{
		UserInterfaceClass = UserInterfaceBpClass.Class;
	}

	/*UBlueprintGeneratedClass* DialogueBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/HUD/DialogueWidget.DialogueWidget_C"));

	DialogueBlueprint = Cast<UClass>(DialogueBP);*/
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bDialogueVisible = false;
	checkDialogueNumber = 0;

	HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);

	if (WTargetPointer)
	{
		TargetPointer = CreateWidget<UUserWidget>(this, WTargetPointer);
		if (TargetPointer)
		{
			TargetPointer->AddToViewport();
			TargetPointer->SetVisibility(ESlateVisibility::Hidden);
		}

		FVector2D Alignment(0.f, 0.f);
		TargetPointer->SetAlignmentInViewport(Alignment);
	}
	//WD_Main = CreateWidget <UUserWidget > (this, WD_MainAsset);

	//if (WStartHud)
	//{
	//	StartHud = CreateWidget<UUserWidget>(this, WStartHud);
	//	if (StartHud)
	//	{
	//		UE_LOG(LogTemp, Log, TEXT("IF WStartHud"));

	//		StartHud->AddToViewport();
	//		StartHud->SetVisibility(ESlateVisibility::Hidden);
	//	}
	//	//FVector2D Alignment(0.f, 0.f);
	//	//StartHud->SetAlignmentInViewport(Alignment);
	//}

	if (HUDOverlay)
	{
		HUDOverlay->AddToViewport();
		HUDOverlay->SetVisibility(ESlateVisibility::Visible);
	}

	if (WEnemyHealthBar)
	{
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar)
		{
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignment);
	}

	if (WDiedHUD)
	{
		DiedHUD = CreateWidget<UUserWidget>(this, WDiedHUD);
		if (DiedHUD)
		{
			DiedHUD->AddToViewport();
			DiedHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WHintHUD)
	{
		HintHUD = CreateWidget<UUserWidget>(this, WHintHUD);
		if (HintHUD)
		{
			HintHUD->AddToViewport();
			HintHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WHint_Stage2HUD)
	{
		Hint_Stage2HUD = CreateWidget<UUserWidget>(this, WHint_Stage2HUD);
		if (Hint_Stage2HUD)
		{
			Hint_Stage2HUD->AddToViewport();
			Hint_Stage2HUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WHint_Stage3HUD)
	{
		Hint_Stage3HUD = CreateWidget<UUserWidget>(this, WHint_Stage3HUD);
		if (Hint_Stage3HUD)
		{
			Hint_Stage3HUD->AddToViewport();
			Hint_Stage3HUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WkeyHUD)
	{
		keyHUD = CreateWidget<UUserWidget>(this, WkeyHUD);
		if (keyHUD)
		{
			keyHUD->AddToViewport();
			keyHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WLoadingHUD)
	{
		LoadingHUD = CreateWidget<UUserWidget>(this, WLoadingHUD);
		if (LoadingHUD)
		{
			LoadingHUD->AddToViewport();
			LoadingHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WEndingSelectHUD)
	{
		EndingSelectHUD = CreateWidget<UUserWidget>(this, WEndingSelectHUD);
		if (EndingSelectHUD)
		{
			EndingSelectHUD->AddToViewport();
			EndingSelectHUD->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (UserInterfaceClass != nullptr)
	{
		UserInterface = CreateWidget<UUserInterface>(this, UserInterfaceClass);
		if (UserInterface)
		{
			UserInterface->AddToViewport();
			UserInterface->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar)
	{
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);

		PositionInViewport.Y -= 100.f;
		PositionInViewport.X -= 100.f;
		EnemyHealthBar->SetPositionInViewport(PositionInViewport);

		FVector2D SizeInViewport = FVector2D(250.f, 25.f);

		//EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}

	if (TargetPointer)
	{
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		PositionInViewport.Y -= 60.f;
		PositionInViewport.X -= 70.f;

		//FVector2D SizeInViewport = FVector2D(10.f, 10.f);

		TargetPointer->SetPositionInViewport(PositionInViewport);
		//TargetArrow->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMainPlayerController::DialogueEvents()
{

}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputMode;

		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		//GameModeOnly();
		bPauseMenuVisible = false;

		PauseMenu->SetVisibility(ESlateVisibility::Hidden);

		if (!bPauseMenuVisible)
		{
			FInputModeGameOnly InputModeGameOnly;
			SetInputMode(InputModeGameOnly);
			bShowMouseCursor = false;
		}
	}
}

//ESC키 입력시 메뉴창이 띄어져 있으면 제거하고 없을 시 보이게 함
void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}


/// <summary>
/// 처음 대화 시작 시 수행하는 함수 UserInterface를 Visible로 값을 지정하고 대화 시작 시 카메라를 
/// 돌리지 못하도록 SetCinematicMode를 지정 
/// FInputModeGameAndUI InputModeUIOnly; SetInputMode(InputModeUIOnly);를 통해 입력을 ui만 가능하도록
/// 그리고 대화창에서 대사 선택을 마우스를 통해서 하도록 구현하였기 때문에 마우스를 화면에 볼 수 있도록 하고 
/// InitializeDialogue에 매개변수로 IntroDialogue를 넣어주고 호출한다.
/// </summary>
void AMainPlayerController::DisplayDialogue()
{
	if (UserInterface)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%d"), Nelia->dialogueCheckNum);
		bDialogueVisible = true;
		UserInterface->SetVisibility(ESlateVisibility::Visible);
		SetCinematicMode(true, true, true);

		FInputModeGameAndUI InputModeUIOnly;

		SetInputMode(InputModeUIOnly);

		bShowMouseCursor = true;

		switch (checkDialogueNumber)
		{
		case 0:
			UserInterface->InitializeDialogue(DrumEventDialogue);
			break;
		case 1:
			UserInterface->InitializeDialogue(IntroDialogue);
			break;
		default:
			break;
		}
	}
}

/// <summary>
/// userinterface가 있을경우 마우스 커서를 사용 못하도록 없애고 dialoguevisible 불 변수를 false로두고 사용자 이동 및 카메라 이동관련 입력키를 받을 수 있도록 SetCinematicMode 설정
/// </summary>
void AMainPlayerController::RemoveDialogue()
{
	if (UserInterface)
	{
		FInputModeGameOnly InputModeGameOnly;

		SetInputMode(InputModeGameOnly);

		bShowMouseCursor = false;

		bDialogueVisible = false;
		SetCinematicMode(false, true, true);
	}
}

int AMainPlayerController::CheckInputKey()
{
	checkPressKey = 0;
	
	if (this->WasInputKeyJustPressed(EKeys::One) || (this->WasInputKeyJustPressed(EKeys::NumPadOne)))
	{
		checkPressKey = 1;

	}
	if (this->WasInputKeyJustPressed(EKeys::Two) || (this->WasInputKeyJustPressed(EKeys::NumPadTwo)))
	{
		checkPressKey = 2;
	}
	if (this->WasInputKeyJustPressed(EKeys::Three) || (this->WasInputKeyJustPressed(EKeys::NumPadThree)))
	{
		checkPressKey = 3;
	}
	if (this->WasInputKeyJustPressed(EKeys::Four) || (this->WasInputKeyJustPressed(EKeys::NumPadFour)))
	{
		checkPressKey = 4;
	}
	
	return checkPressKey;
}
//int AMainPlayerController::CheckInputKeyTime()
//{
//
//}

void AMainPlayerController::DisplayTargetPointer()
{
	if (TargetPointer)
	{
		bTargetPointerVisible = true;
		TargetPointer->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveTargetPointer()
{
	if (TargetPointer)
	{
		bTargetPointerVisible = false;
		TargetPointer->SetVisibility(ESlateVisibility::Hidden);
	}
}