// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainAnimInstance.h"
#include "FollowerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API UFollowerAnimInstance : public UMainAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
		float Speed;
};
