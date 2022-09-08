// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	//if (WEnemyHealthBar)
	//{
	//	EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
	//	if (EnemyHealthBar)
	//	{
	//		EnemyHealthBar->AddToViewport();
	//		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	//	}
	//	FVector2D Alignment(0.f, 0.f);
	//	EnemyHealthBar->SetAlignmentInViewport(Alignment);
	//}
}
