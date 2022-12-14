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

	/** Variable to hold the widget after creating it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WDiedHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* DiedHUD;

	bool bPauseMenuVisible;

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

	void GameModeOnly();

	//???? ????
	int CheckInputKey();

	//???? ?????? ?????? ???? ?????? ????
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
