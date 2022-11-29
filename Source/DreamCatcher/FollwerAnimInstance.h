// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FollwerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class DREAMCATCHER_API UFollwerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float Speed;
};
