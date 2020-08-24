// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
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

	/** Strafe actor sideways */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Timeline, meta = (AllowPrivateAccess = "true"))
	class UTimelineComponent* StrafingTimeLineComponent;
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddCoin();

	/** Declare strafe timeline delegate to be binded with StrafeTimeLineReturn (float Value) */
	FOnTimelineFloat InterpFunction{};
	/** Declare strafe timeline delegate to be binded with StrafeTimeLineFinished() */
	FOnTimelineEvent OnStrafeTimeLineFinished{};
	UFUNCTION()
	void StrafeTimeLineReturn (float Value);
		
	UFUNCTION()
	void StrafeTimeLineFinished();
	
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

	/** Base curve function for strafe timeline. */
	UPROPERTY(EditAnywhere, Category=Timeline)
	class UCurveFloat* FCurve;
	
	/** Base possibility to turn at input. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	bool bCanTurn;

	/** Base status for death */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	bool bDead;
	/** Base status for strafing */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Character)
	bool bStrafing;

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
	UPROPERTY()
	TEnumAsByte<EMoveDirection> MoveDirection;
	UPROPERTY()
	FVector CurrentLocation;
	UPROPERTY()
	FVector DestinationLocation;
	
	
	
	
	
	
protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for side to side input */
	void MoveRight();
	void MoveLeft();


	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	void MoveForward(float Value);

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
	// Called at play
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick( float DeltaTime ) override;
};

