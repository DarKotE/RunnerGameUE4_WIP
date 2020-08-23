// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RunnerCharacter.generated.h"

UENUM()
enum ELane
{
	Left UMETA(DisplayName = "Left"),
    Middle UMETA(DisplayName = "Middle"),
    Right UMETA(DisplayName = "Right"),
};

UENUM()
enum EMoveDirection
{
	ToLeft UMETA(DisplayName = "ToLeft"),
    None UMETA(DisplayName = "None"),
    ToRight UMETA(DisplayName = "ToRight"),
};

UCLASS(config=Game)
class ARunnerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddCoin();
	
public:
	ARunnerCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Base distance between lanes to travel */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLaneSwitchDistance;

	/** Base possibility to turn at input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	bool bCanTurn;

	/** Base status for death */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	bool bDead;

	/** Base desired rotation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Character)
	FRotator DesiredRotation;
	
	/** Numbers of coins was picked up */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
    int32 TotalCoins;

	
	/** Current lane occupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	TEnumAsByte<ELane> CurrentLane;
	/** Next lane to occupy */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	TEnumAsByte<ELane> NextLane;

	
	TEnumAsByte<EMoveDirection> MoveDirection;
	
	
	
	
protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	
	void OnRunnerDeath();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
private:
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};

