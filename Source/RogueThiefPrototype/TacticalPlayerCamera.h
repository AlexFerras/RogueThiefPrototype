// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TacticalPlayerCamera.generated.h"


UCLASS()
class ROGUETHIEFPROTOTYPE_API ATacticalPlayerCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATacticalPlayerCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void MoveForward(float Value);

	UFUNCTION()
	void MoveRight(float Value);

	UFUNCTION(BlueprintImplementableEvent)
	void RotateRight();

	UFUNCTION(BlueprintImplementableEvent)
	void RotateLeft();


public:

protected:
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Movement")
	class UFloatingPawnMovement* Movement;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* RootMesh;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class USpringArmComponent* SpringArm;

	UPROPERTY(BlueprintReadOnly)
	FRotator TargetRotation;
	
	UPROPERTY(BlueprintReadOnly)
	bool bRotating;

};
