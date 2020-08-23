// Fill out your copyright notice in the Description page of Project Settings.

#include "RunnerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "UnrealAudioSoundFile.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ARunnerCharacter

ARunnerCharacter::ARunnerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
	CameraBoom->SocketOffset = FVector(0.0f,0.0f,100.0f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true; //Smooth camera movements

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	BaseLaneSwitchDistance = 300.0f;
	bCanTurn = false;
	bDead = false;
	DesiredRotation = {0,90.0f,0};
	TotalCoins = 0;
	CurrentLane = NextLane = Middle;	
	MoveDirection = None;
	
}


//////////////////////////////////////////////////////////////////////////
// Input

void ARunnerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	//PlayerInputComponent->BindAxis("MoveForward", this, &ARunnerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARunnerCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARunnerCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARunnerCharacter::OnResetVR);
	
}

void ARunnerCharacter::AddCoin()
{
	TotalCoins++;
}

void ARunnerCharacter::OnRunnerDeath()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ARunnerCharacter::DisableInput(PlayerController);
	
	
}

void ARunnerCharacter::Tick(float DeltaTime )
{
	Super::Tick( DeltaTime );
	if (!bDead) MoveForward(1.0f);
	
}

void ARunnerCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARunnerCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ARunnerCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}


void ARunnerCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARunnerCharacter::MoveRight(float Value)
{
	const FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0, Rotation.Yaw, 0);
	
	if (!bCanTurn)
	{
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
	
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		
		YawRotation.Add(0,DesiredRotation.Yaw * Value,0);		
		Controller->SetControlRotation(YawRotation);		
		bCanTurn = false;
		
		// get right vector 
		
		
		//AddControllerYawInput(Value*  BaseTurnRate * GetWorld()->GetDeltaSeconds());
		
		
		
		//FMath::RInterpTo(Rotation, DesiredRotation,GetWorld()->GetDeltaSeconds(),5)
		// add movement in that direction
		
	}
}
