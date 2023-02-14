// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridFunctionLib.h"
#include "GridConnectorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROGUETHIEFPROTOTYPE_API UGridConnectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGridConnectorComponent();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void GetGridPosition(FGridVector& Pos1, FGridVector& Pos2);

	FVector GetOwnerLocation();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetToGrid();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetGridPosition(FGridVector Pos);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	FGridVector GetConnectedPos(const FGridVector& Pos);
		

protected: 

	UPROPERTY(BlueprintReadOnly)
	class AGridSystemController* Grid;

	UPROPERTY(BlueprintReadOnly)
	FGridVector GridPosition1;

	UPROPERTY(BlueprintReadOnly)
	FGridVector GridPosition2;
};
