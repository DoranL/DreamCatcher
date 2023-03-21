// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Nelia.h"
#include "Kismet/GameplayStatics.h"
#include "UserInterface.h"
#include "UObject/ConstructorHelpers.h"

AMainPlayerController::AMainPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> UserInterfaceBpClass(TEXT("/Game/HUD/DialogueWidget"));

	if (UserInterfaceBpClass.Class != nullptr)
	{
		UserInterfaceClass = UserInterfaceBpClass.Class;
	}
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bDialogueVisible = false;

	HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	//WD_Main = CreateWidget <UUserWidget > (this, WD_MainAsset);

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
			UE_LOG(LogTemp, Log, TEXT("Wenemy healthbar if"));

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

		PositionInViewport.Y -= 160.f;
		PositionInViewport.X -= 100.f;
		EnemyHealthBar->SetPositionInViewport(PositionInViewport);

		FVector2D SizeInViewport = FVector2D(250.f, 25.f);

		//EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMainPlayerController::DialogueEvents()
{

}

void AMainPlayerController::DisplayEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		UE_LOG(LogTemp, Log, TEXT("DisplayEnemyHealthBar"));

		bEnemyHealthBarVisible = true;
		EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnemyHealthBar)
	{
		UE_LOG(LogTemp, Log, TEXT("enemy healthbar hidden"));

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

		FInputModeGameAndUI InputModeGameAndUI;

		SetInputMode(InputModeGameAndUI);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		//��������
		//GameModeOnly();

		bShowMouseCursor = false;

		bPauseMenuVisible = false;
	}
}

//ESCŰ �Է½� �޴�â�� ����� ������ �����ϰ� ���� �� ���̰� ��
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
/// ó�� ��ȭ ���� �� �����ϴ� �Լ� UserInterface�� Visible�� ���� �����ϰ� ��ȭ ���� �� ī�޶� 
/// ������ ���ϵ��� SetCinematicMode�� ���� 
/// FInputModeGameAndUI InputModeUIOnly; SetInputMode(InputModeUIOnly);�� ���� �Է��� ui�� �����ϵ���
/// �׸��� ��ȭâ���� ��� ������ ���콺�� ���ؼ� �ϵ��� �����Ͽ��� ������ ���콺�� ȭ�鿡 �� �� �ֵ��� �ϰ� 
/// InitializeDialogue�� �Ű������� IntroDialogue�� �־��ְ� ȣ���Ѵ�.
/// </summary>
void AMainPlayerController::DisplayDialogue()
{
	if (UserInterface)
	{
		bDialogueVisible = true;
		UserInterface->SetVisibility(ESlateVisibility::Visible);
		SetCinematicMode(true, true, true);

		FInputModeGameAndUI InputModeUIOnly;

		SetInputMode(InputModeUIOnly);

		bShowMouseCursor = true;
		UserInterface->InitializeDialogue(IntroDialogue);
	}
}

/// <summary>
/// userinterface�� ������� ���콺 Ŀ���� ��� ���ϵ��� ���ְ� dialoguevisible �� ������ false�εΰ� ����� �̵� �� ī�޶� �̵����� �Է�Ű�� ���� �� �ֵ��� SetCinematicMode ����
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
	int checkPressKey = 0;
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
	
	return checkPressKey;
}

//int AMainPlayerController::CheckInputKeyTime()
//{
//
//}