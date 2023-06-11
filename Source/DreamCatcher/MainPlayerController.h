 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()


public:
	AMainPlayerController();

	//UFUNCTION(BlueprintCallable, Category = "Input")
	//void LookInput();

	//UFUNCTION(BlueprintCallable, Category = "Input")
	//void UnLockInput();

public:
	/** Reference to the UMG asset in the editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> WStartHud;*/

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> WD_MainAsset;*/

	///** Variable to hold the widget after creating it */
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* WD_Main;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WDiedHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WHintHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* DiedHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HintHUD;

	bool bPauseMenuVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	bool bDialogueVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category ="HUD")
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void RemovePauseMenu();

	void DisplayDialogue();

	void RemoveDialogue();

	UPROPERTY(BlueprintReadWrite, Category = "Player UI")
	class UUserInterface* UserInterface;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class ANelia* Nelia;

	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;*/

	void DialogueEvents();

	TSubclassOf<class UUserWidget> UserInterfaceClass;

	void TogglePauseMenu();

	void ToggleDialogue();

	bool bEnemyHealthBarVisible;

	void DisplayEnemyHealthBar();
	void RemoveEnemyHealthBar();
	 
	FVector EnemyLocation;

	//스킬 사용
	int CheckInputKey();

	//스킬 버튼을 누르고 있는 시간을 체크
	//int CheckInputKeyTime();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	FORCEINLINE UUserInterface* GetUI() { return UserInterface; };

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* IntroDialogue;
};
