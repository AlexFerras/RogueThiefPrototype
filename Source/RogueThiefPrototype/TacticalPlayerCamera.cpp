// Fill out your copyright notice in the Description page of Project Settings.


#include "TacticalPlayerCamera.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATacticalPlayerCamera::ATacticalPlayerCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bRotating = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));


	RootMesh->SetupAttachment(RootComponent);
	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm,USpringArmComponent::SocketName);

	SpringArm->TargetArmLength = 400.f;
	SpringArm->TargetOffset = FVector{ 0.f,400.f,400.f };
	
}
  
// Called when the game starts or when spawned
void ATacticalPlayerCamera::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());
	PlayerController->bShowMouseCursor = true;

}

// Called every frame
void ATacticalPlayerCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATacticalPlayerCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ATacticalPlayerCamera::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATacticalPlayerCamera::MoveRight);
	PlayerInputComponent->BindAction("RotateRight", IE_Pressed, this, &ATacticalPlayerCamera::RotateRight);
	PlayerInputComponent->BindAction("RotateLeft", IE_Pressed, this, &ATacticalPlayerCamera::RotateLeft);

}

void ATacticalPlayerCamera::MoveForward(float Value)
{
	FVector MovementVector = Camera->GetForwardVector();
	MovementVector.Z = 0;
	MovementVector.Normalize();

	AddMovementInput(MovementVector, Value);
}

void ATacticalPlayerCamera::MoveRight(float Value)
{
	FVector MovementVector = Camera->GetRightVector();
	MovementVector.Z = 0;
	MovementVector.Normalize();

	AddMovementInput(MovementVector, Value);
}

