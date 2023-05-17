// Copyright Epic Games, Inc. All Rights Reserved.

#include "OSU_Gamejam_2023Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyCharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "ECustomMovement.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "EnhancedInputSubsystems.h"

//////////////////////////////////////////////////////////////////////////
// AOSU_Gamejam_2023Character

AOSU_Gamejam_2023Character::AOSU_Gamejam_2023Character(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	MovementComponent = Cast<UMyCharacterMovementComponent>(GetCharacterMovement()); // <--

	Hook_force = CreateDefaultSubobject<URadialForceComponent>(TEXT("Hook"));
	
	Hook_force->Radius = 50000;
	Hook_force->Activate(false);
	Hook_force->ForceStrength = 10;
	Hook_force->ImpulseStrength = -20;
	Hook_force->bIgnoreOwningActor = false;
	Hook_force->bImpulseVelChange = true;
	HookQueryParams.AddIgnoredActor(this);
}

void AOSU_Gamejam_2023Character::Try_hook()
{
	//FVector Start = Hook_start->GetComponentLocation();
	FVector Start = GetActorLocation();
	FVector End = FollowCamera->GetForwardVector() * 5000 + Start;


	GetWorld()->LineTraceSingleByChannel(HookHit, Start, End, ECC_WorldStatic, HookQueryParams);
	if (HookHit.bBlockingHit) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Is_hook"));
		DrawDebugLine(GetWorld(), Start, HookHit.Location, FColor::Red, false, 1, 0, 5);
		
		Hook_force->SetWorldLocation(HookHit.Location, false, tmp_hit, ETeleportType::ResetPhysics);
		Hook_force->Activate(true);
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Hooking);

		/*CustomMovementComponent->Is_hooking = true;
		CustomMovementComponent->SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_Hooking);
		CustomMovementComponent->bOrientRotationToMovement = false;*/
	}
}

void AOSU_Gamejam_2023Character::Stop_hook()
{
	if (HookHit.bBlockingHit) {
		Hook_force->Activate(false);

		MovementComponent->SetMovementMode(MOVE_Falling);
	}

}

void AOSU_Gamejam_2023Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AOSU_Gamejam_2023Character::Climb(const FInputActionValue& Value)
{
	bool climb = Value.Get<bool>();
	if (climb) {
		if (MovementComponent->IsClimbing()) {
			MovementComponent->CancelClimbing();
		}
		else {
			MovementComponent->TryClimbing();
		}
			
	}
	
}

void AOSU_Gamejam_2023Character::CancelClimb(const FInputActionValue& Value)
{
	bool climb = Value.Get<bool>();

	if (climb) {
		
		MovementComponent->CancelClimbing();
	}

	
}

void AOSU_Gamejam_2023Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementComponent->IsHooking()) {
		Hook_force->FireImpulse();
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOSU_Gamejam_2023Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOSU_Gamejam_2023Character::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOSU_Gamejam_2023Character::Look);

		//Climbing
		EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Triggered, this, &AOSU_Gamejam_2023Character::Climb);
		//EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Triggered, this, &AOSU_Gamejam_2023Character::CancelClimb);

	}

}

FCollisionQueryParams AOSU_Gamejam_2023Character::GetIgnoreCharacterParams()
{
	FCollisionQueryParams Params;

	//TArray<AActor*> CharacterChildren;
	//GetAllChildActors(CharacterChildren);
	//Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

void AOSU_Gamejam_2023Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		FRotator Rotation = Controller->GetControlRotation();
		FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 

		if (MovementComponent->IsClimbing())
		{
			ForwardDirection = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), -GetActorRightVector());

			RightDirection = FVector::CrossProduct(MovementComponent->GetClimbSurfaceNormal(), GetActorUpVector());
			

		}
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AOSU_Gamejam_2023Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AOSU_Gamejam_2023Character::Jump()
{
	


	if (MovementComponent->IsClimbing()) {

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Dash"));
		MovementComponent->TryClimbDashing();

	}
	else {
		Super::Jump();
	}

}




