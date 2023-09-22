// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "Hypono.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API AHypono : public AEnemy
{
	GENERATED_BODY()
	
public:
	AHypono();

	void Attack() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
