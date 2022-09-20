// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"                   //언리얼 오브젝트가 동작할 수 있는 최소 기능만 선언된 헤더 파일 ex)Templates , Generic, Containers, Math 포함
#include "GameFramework/Character.h"
#include "Nelia.generated.h"

UENUM(BlueprintType)					  //블루프린트에서 사용 가능하도록 타입을 넣어줌
enum class EMovementStatus : uint8        //enum클래스 열거형 
{
	EMS_Normal		 UMETA(DisplayName = "Normal"), //열거자들의 이름을 추가 DisplayName에 넣은 이름이 언리얼 엔진에서 보여지는 이름
	EMS_Sprinting	 UMETA(DisplayName = "Sprinting"),
	EMS_Death		 UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)						
enum class EStaminaStatus : uint8
{
	ESS_Normal				UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum		UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted		    UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class DREAMCATCHER_API ANelia : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANelia();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	/** Set Movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Pressed down to enable sprinting */
	void ShiftKeyDown();

	/** Released to stop sprinting */
	void ShiftKeyUp();

	/**Camera boom positioning the camera behind the player*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAcess = "true"))
	class UCameraComponent* FollowCamera;

	/** Base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;


	//Player Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override;
	virtual void StopJumping() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** called for forwards/backwards input*/
	void MoveForward(float Value);

	/** called for side to side input*/
	void MoveRight(float Value);

	bool bMovingForward;
	bool bMovingRight;

	/** Called via input to turn at a given rate
	* @parm Rate This is a normalized rate, i.e 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate
	* @parm Rate This is a normalized rate, i.e 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	bool bPickup;
	void PickupPress();
	void PickupReleas();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
    class AItem* ActiveOverlappingItem;

	//장착 무기를 설정 매개변수로 무기를 가지고 (EquippedWeapon = WeaponToSet)	
	FORCEINLINE void SetEquippedWeapon(AWeapon* WeaponToSet) { EquippedWeapon = WeaponToSet; }

	//FORCEINLINE AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Anims")
	bool bAttacking;

	int combatCount = 0;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	
	/// ////////////////////
	UPROPERTY(EditAnywhere)
	class UAnimMontage* DashMontage;
	
	UPROPERTY(VisibleAnywhere)
	float DashDistance = 3000.f;

	UPROPERTY(VisibleAnywhere)
	bool bDash = true;
	void Dash();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bJump;

	//timer
	FTimerHandle DashTimer;

	void CanDash();

	class UMainAnimInstance* MainAnimInstance;

};