// Copyright Epic Games, Inc. All Rights Reserved.

#include "Examen2MultiplayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Examen2MultiplayerProjectile.h"


//////////////////////////////////////////////////////////////////////////
// AExamen2MultiplayerCharacter

AExamen2MultiplayerCharacter::AExamen2MultiplayerCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(20.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	ProjectileClass = AExamen2MultiplayerProjectile::StaticClass();

	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

}

void AExamen2MultiplayerCharacter::BeginPlay()
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

void AExamen2MultiplayerCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		// FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			// FString deathMessage = FString::Printf(TEXT("You have been killed."));
			// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		// FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}

	//Functions that occur on all machines. 
	/*  
		Any special functionality that should occur as a result of damage or death should be placed here. 
	*/
}

void AExamen2MultiplayerCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}



//////////////////////////////////////////////////////////////////////////// Input

void AExamen2MultiplayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExamen2MultiplayerCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExamen2MultiplayerCharacter::Look);
		
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AExamen2MultiplayerCharacter::HandleFire);
	}
}


void AExamen2MultiplayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AExamen2MultiplayerCharacter::Look(const FInputActionValue& Value)
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

void AExamen2MultiplayerCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AExamen2MultiplayerCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AExamen2MultiplayerCharacter::HandleFire_Implementation()
{
	if(bHasRifle)
	{
		FVector spawnLocation = GetActorLocation() + ( GetActorRotation().Vector()  * 100.0f ) + (GetActorUpVector() * 50.0f);
		FRotator spawnRotation = GetActorRotation();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		AExamen2MultiplayerProjectile* spawnedProjectile = GetWorld()->SpawnActor<AExamen2MultiplayerProjectile>(spawnLocation, spawnRotation, spawnParameters);
	}
}

// Replicated Properties

void AExamen2MultiplayerCharacter::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicate current health.
	DOREPLIFETIME(AExamen2MultiplayerCharacter, CurrentHealth);
}


void AExamen2MultiplayerCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}


float AExamen2MultiplayerCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}
