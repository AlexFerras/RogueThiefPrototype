// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridFunctionLib.h"
#include "GridComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathfindCallback, TArray<FGridPath>, Paths);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveToGrid, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveFinished);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ROGUETHIEFPROTOTYPE_API UGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGridComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	FGridVector GetGridPosition();

	FVector GetOwnerLocation();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetToGrid();

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetGridPosition(FGridVector Pos);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void AsyncPathFindToPositions(TArray<FGridVector> Positions, FGridVector Range, int32 MaxCost);

	bool NodesContain(TArray<TSharedPtr<struct FGridNode>> Array, FGridVector Coord);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool MoveOnGrid(FGridVector Pos);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool AddGridPos(FGridVector Pos);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	TArray<FGridVector> GetUnblockedSquarePositionsInRange(FGridVector Range);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	TArray<FGridVector> GetUnblockedPerimeterPositionsInRange(FGridVector Range);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void AsyncGetPathsInRange(int32 Cost);

	void PathfindCallback(class FPathFindToRadiusWorker* Worker);

	void ReturnPaths(const TArray<FGridPath>& Paths);
	
	bool IsGridPosFree(FGridVector Pos);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void StartGridMovingToPos(const FGridVector& Pos, float AcceptRadius = 10.f);

	UFUNCTION()
	void AddDirectMovementInputToTarget(FVector Target, float AcceptRadius);


public:

	UPROPERTY(BlueprintAssignable)
	FOnMoveToGrid OnMoveToGrid;

	UPROPERTY(BlueprintAssignable)
	FOnPathfindCallback OnPathFindCallBack;

	UPROPERTY(BlueprintAssignable)
	FOnMoveFinished OnMoveFinished;

protected:

	UPROPERTY(BlueprintReadOnly)
	class AGridSystemController* Grid;

	UPROPERTY(BlueprintReadOnly)
	struct FGridVector GridPosition;



};

