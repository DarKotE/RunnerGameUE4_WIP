// Fill out your copyright notice in the Description page of Project Settings.

#include "RunnerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "UnrealAudioSoundFile.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"

//////////////////////////////////////////////////////////////////////////
// ARunnerCharacter

ARunnerCharacter::ARunnerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate   = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate              = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity             = 600.f;	
	GetCharacterMovement()->AirControl                = 0.1f;
	GetCharacterMovement()->MaxWalkSpeed              = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength         = 300.0f; // The camera follows at this distance behind the character
	CameraBoom->SocketOffset            = FVector(0.0f, 0.0f, 100.0f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag        = true; //Smooth camera movements

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//Create timeline
	StrafingTimeLineComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("StrafingTimeLine"));
	InterpFunction.BindUFunction(this,FName("StrafeTimeLineReturn"));
	OnStrafeTimeLineFinished.BindUFunction(this,FName("StrafeTimeLineFinished"));
	
	BaseLaneSwitchDistance = 300.0f;
	
	bCanTurn               = false;
	bDead                  = false;
	bStrafing = false;
	DesiredRotation        = {0, 90.0f, 0};
	TotalCoins             = 0;
	CurrentLane            = NextLane = ELane::Middle;
	MoveDirection          = EMoveDirection::None;
}


//////////////////////////////////////////////////////////////////////////
// Input

void ARunnerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARunnerCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ARunnerCharacter::StopJumping);

	//PlayerInputComponent->BindAxis("MoveForward", this, &ARunnerCharacter::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);
	PlayerInputComponent->BindAction("MoveRight", IE_Pressed, this, &ARunnerCharacter::MoveRight);
	PlayerInputComponent->BindAction("MoveLeft", IE_Pressed, this, &ARunnerCharacter::MoveLeft);

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

void ARunnerCharacter::StrafeTimeLineReturn(float Value)
{

	
	SetActorLocation(FMath::Lerp(CurrentLocation,DestinationLocation, Value),false,nullptr, ETeleportType::None);
	
	//SetActorLocation(FMath::VInterpTo(GetActorLocation(), DestinationLocation, Value, Value), false, nullptr,ETeleportType::None	);
	                 
	//TeleportPhysics;
}

void ARunnerCharacter::StrafeTimeLineFinished()
{
//	if (StrafingTimeLineComponent->GetPlaybackPosition()== 0.0f)
//	{
//		GLog->Log("Stop");
//		StrafingTimeLineComponent->Stop();
//		
//	}
//	else
//	{
//		GLog->Log("Reverse");
//		//StrafingTimeLineComponent->Stop();
//		//StrafingTimeLineComponent->Reverse();
//		
//	}
	bStrafing = false;
	MoveDirection = None;	
}

void ARunnerCharacter::OnRunnerDeath()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ARunnerCharacter::DisableInput(PlayerController);
}

void ARunnerCharacter::BeginPlay()
{
	Super::BeginPlay();
	//Check if curve asset is valid
	if (FCurve)
	{
		//Add float curve to timeline and connect it with delegate
		StrafingTimeLineComponent->AddInterpFloat(FCurve, InterpFunction,FName("Alpha"));
		//Add timeline finished function
		StrafingTimeLineComponent->SetTimelineFinishedFunc(OnStrafeTimeLineFinished);		
		//Setting timeline's settings
		StrafingTimeLineComponent->SetLooping(false);
		StrafingTimeLineComponent->SetIgnoreTimeDilation(true);
		
		
	}
}

void ARunnerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto TempVelocity = GetCharacterMovement()->VisualizeMovement();
	if (!bDead)
		MoveForward(1.0f);
		
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

void ARunnerCharacter::MoveRight()
{
	
	const FRotator Rotation = Controller->GetControlRotation();
	FRotator       YawRotation(0, Rotation.Yaw, 0);
	//MoveDirection = None;
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
//	if (bCanTurn)
//	{
	//		YawRotation.Add(0, DesiredRotation.Yaw * Value, 0);
	//Controller->SetControlRotation(YawRotation); //TODO переделать под lerp(?)
	//bCanTurn = false;
//	}
	
	if ((Controller != NULL))
	{
			MoveDirection = ToRight;
			GLog->Log(TEXT("ToRight"));
		}
		if (!bStrafing)
		{
			bStrafing = true;
			GLog->Log(TEXT("тру"));
			CurrentLocation = DestinationLocation = GetActorLocation();
			DestinationLocation.Y+=BaseLaneSwitchDistance;
			DestinationLocation.X += GetActorForwardVector().X * GetCharacterMovement()->MaxWalkSpeed * 0.1f ;
			StrafingTimeLineComponent->PlayFromStart();			
			
		}       
		
		switch (MoveDirection) //TODO переделать(?) http://gameprogrammingpatterns.com/state.html
		{
		case ToLeft:
			if (CurrentLane == Right)
			{
				//MoveLeft
				
				//AddMovementInput()
				CurrentLane = Middle;
				break;
			}
			if (CurrentLane == Middle)
			{
				CurrentLane = Left;
				break;
			}
			if (CurrentLane == Left)
			{
				break;
			}

			break;
		case ToRight:
		{
			if (CurrentLane == Right)
			{
				break;
			}
			if (CurrentLane == Middle)
			{
				//MoveRight
				CurrentLane = Right;
				break;
			}
			if (CurrentLane == Left)
			{
				//MoveRight
				CurrentLane = Middle;
				break;
			}
		}
		break;
		default:
			break;
		
		}
		

		// get right vector 


		//AddControllerYawInput(Value*  BaseTurnRate * GetWorld()->GetDeltaSeconds());


		//FMath::RInterpTo(Rotation, DesiredRotation,GetWorld()->GetDeltaSeconds(),5)
		// add movement in that direction
	}

void ARunnerCharacter::MoveLeft()
{
	const FRotator Rotation = Controller->GetControlRotation();
	FRotator       YawRotation(0, Rotation.Yaw, 0);
	
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	//	if (bCanTurn)
	//	{
	//		YawRotation.Add(0, DesiredRotation.Yaw * Value, 0);
	//Controller->SetControlRotation(YawRotation); //TODO переделать под lerp(?)
	//bCanTurn = false;
	//	}
	
	if ((Controller != NULL))
    {
            MoveDirection = ToLeft;
            GLog->Log(TEXT("ToLeft"));
        }
        if (!bStrafing)
        {
            bStrafing = true;
            GLog->Log(TEXT("тру"));
        	CurrentLocation = DestinationLocation = GetActorLocation();
        	DestinationLocation.Y-=BaseLaneSwitchDistance;
        	DestinationLocation.X += GetActorForwardVector().X * GetCharacterMovement()->MaxWalkSpeed * 0.1f;
            StrafingTimeLineComponent->PlayFromStart();
            
			
        }       
		
        switch (MoveDirection) //TODO переделать(?) http://gameprogrammingpatterns.com/state.html
        {
        case ToLeft:
        	if (CurrentLane == Right)
        	{
                //MoveLeft
				
                //AddMovementInput()
                CurrentLane = Middle;
                break;
            }
            if (CurrentLane == Middle)
            {
                CurrentLane = Left;
                break;
            }
            if (CurrentLane == Left)
            {
                break;
            }

            break;
        case ToRight:
        {
            if (CurrentLane == Right)
            {
                break;
            }
            if (CurrentLane == Middle)
            {
                //MoveRight
                CurrentLane = Right;
                break;
            }
            if (CurrentLane == Left)
            {
                //MoveRight
                CurrentLane = Middle;
                break;
            }
        }
        break;
        default:
        	break;
		
        }
		

        // get right vector 


        //AddControllerYawInput(Value*  BaseTurnRate * GetWorld()->GetDeltaSeconds());


        //FMath::RInterpTo(Rotation, DesiredRotation,GetWorld()->GetDeltaSeconds(),5)
        // add movement in that direction
    



}
