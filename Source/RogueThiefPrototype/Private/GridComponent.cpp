// Fill out your copyright notice in the Description page of Project Settings.


#include "GridComponent.h"
#include "GridSystemController.h"
#include "Algo/Reverse.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GridAffector.h"
#include "PathFindInRadiusWorker.h"

// Sets default values for this component's properties
UGridComponent::UGridComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Grid = nullptr;
	GridPosition = {};

	// ...
}


// Called when the game starts
void UGridComponent::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AGridSystemController>(UGameplayStatics::GetActorOfClass(this, AGridSystemController::StaticClass()));
	SetToGrid();

	// ...
	
}


// Called every frame
void UGridComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FGridVector UGridComponent::GetGridPosition()
{
	return GridPosition;
}

FVector UGridComponent::GetOwnerLocation()
{
	return GetOwner()->GetActorLocation();
}

void UGridComponent::SetToGrid()
{
	if (!Grid)
		return;
	if (UGridFunctionLib::IsWorldLocationOnGrid(GetOwnerLocation()))
		return;
	GridPosition = UGridFunctionLib::ConvertWorldToGrid(GetOwnerLocation());
	Grid->MoveGridPosition(GetOwner(), GridPosition);
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
	GetOwner()->SetActorLocation(NewLoc);
}

void UGridComponent::SetGridPosition(FGridVector Pos)
{
	Grid->MoveGridPosition(GetOwner(), Pos);
	GridPosition = Pos;
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
}

void UGridComponent::AsyncPathFindToPositions(TArray<FGridVector> Positions, FGridVector Range, int32 MaxCost)
{
	FPathFindToRadiusWorker* Worker = new FPathFindToRadiusWorker(Grid, GetGridPosition(), MaxCost);
	FTimerHandle TimerHandle;
	FTimerDelegate ThreadDelegate = FTimerDelegate::CreateUObject(this, &UGridComponent::PathfindCallback, Worker);
	GetOwner()->GetWorldTimerManager().SetTimerForNextTick(ThreadDelegate);

}



bool UGridComponent::NodesContain(TArray<TSharedPtr<FGridNode>> Array, FGridVector Coord)
{
	for (auto& Node : Array)
	{
		if (Node.Get()->Coord == Coord)
			return true;
	}
	return false;
}


bool UGridComponent::MoveOnGrid(FGridVector Pos)
{
	if (!IsGridPosFree(Pos))
		return false;

	Grid->MoveGridPosition(GetOwner(), Pos);
	GridPosition = Pos;
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
	OnMoveToGrid.Broadcast(NewLoc);
	return true;
}

bool UGridComponent::AddGridPos(FGridVector Pos)
{
	FGridVector NewPos = GetGridPosition() + Pos;
	return MoveOnGrid(NewPos);
}

TArray<FGridVector> UGridComponent::GetUnblockedSquarePositionsInRange(FGridVector Range)
{
	return Grid->GetUnblockedSquarePositionsInRange(GetGridPosition(), Range);
}

TArray<FGridVector> UGridComponent::GetUnblockedPerimeterPositionsInRange(FGridVector Range)
{
	FGridVector GridPos = GetGridPosition();
	TArray<FGridVector> SelectedPositions;
	int32 Multiply = -1;
	for (int32 IndexA = 0; IndexA < 2; IndexA++)
	{
		for (int32 Index = GridPos.X - Range.X; Index <= GridPos.X + Range.X; Index++)
		{
			FGridVector TestPos = { 0, Index, GridPos.Y + Range.Y * Multiply };
			SelectedPositions.Add(TestPos);
		}
		for (int32 Index = GridPos.Y - Range.Y; Index <= GridPos.Y + Range.Y; Index++)
		{
			FGridVector TestPos = { 0, GridPos.X + Range.X * Multiply, Index };
			SelectedPositions.Add(TestPos);
		}
		Multiply += 2;
	}
	auto Copy = SelectedPositions;
	for (int32 Index = GridPos.Z - Range.Z; Index <= GridPos.Z + Range.Z; Index++)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::FromInt(Index));
		if (Index == 0)
			continue;
		for (auto& Pos : Copy)
		{
			FGridVector TestPos = { Index, Pos.X, Pos.Y };
			SelectedPositions.Add(TestPos);
		}
	}
	for (int32 Index = 0; Index < SelectedPositions.Num(); Index++)
	{
		if (!IsGridPosFree(SelectedPositions[Index]))
		{
			SelectedPositions.RemoveAt(Index);
		}
	}
	return SelectedPositions;
}

void UGridComponent::AsyncGetPathsInRange(int32 Cost)
{
	FPathFindToRadiusWorker* Worker = new FPathFindToRadiusWorker(Grid, GetGridPosition(), Cost);
	FTimerHandle TimerHandle;
	FTimerDelegate ThreadDelegate = FTimerDelegate::CreateUObject(this, &UGridComponent::PathfindCallback, Worker);
	GetOwner()->GetWorldTimerManager().SetTimerForNextTick(ThreadDelegate);
	
}

void UGridComponent::PathfindCallback(FPathFindToRadiusWorker* Worker)
{
	if (!Worker->IsRunning())
	{
		auto ResultPaths = Worker->Paths;
		Worker->Stop();
		delete Worker;
		ReturnPaths(ResultPaths);
	}
	else
	{
	FTimerDelegate ThreadDelegate = FTimerDelegate::CreateUObject(this, &UGridComponent::PathfindCallback, Worker);
	GetOwner()->GetWorldTimerManager().SetTimerForNextTick(ThreadDelegate);
	}

}

void UGridComponent::ReturnPaths(const TArray<FGridPath>& Paths)
{
	auto NewPaths = Paths;
	OnPathFindCallBack.Broadcast(Paths);
}


bool UGridComponent::IsGridPosFree(FGridVector Pos)
{
	if (!UGridFunctionLib::IsGridPositionValid(Pos))
		return false;
	auto Actors = Grid->GetActorsAtPos(Pos);
	for (auto& Actor : Actors)
	{
		if (!Actor->Implements<UGridAffector>())
			continue;

		if (IGridAffector::Execute_IsBlock(Actor))
			return false;

	}
	return true;
}



void UGridComponent::StartGridMovingToPos(const FGridVector& Pos)
{
	AddDirectMovementInputToTarget(Pos.ToWorld(), 1.f);

}

void UGridComponent::AddDirectMovementInputToTarget(FVector Target, float AcceptRadius)
{
	APawn* Owner = Cast<APawn>(GetOwner());
	FVector OwnerLoc = UGridFunctionLib::ConvertWorldToGrid(Owner->GetActorLocation()).ToWorld();
	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(OwnerLoc, Target);
	if (!(UGridFunctionLib::ConvertWorldToGrid(Owner->GetActorLocation()) == UGridFunctionLib::ConvertWorldToGrid(Target)))
	{
		Owner->AddMovementInput(Direction);
		UE_LOG(LogTemp, Warning, TEXT("AddedMovementInput to %s"), *Direction.ToString());
		FTimerDelegate NextDelegate = FTimerDelegate::CreateUObject(this, &UGridComponent::AddDirectMovementInputToTarget, Target, AcceptRadius);
		GetOwner()->GetWorldTimerManager().SetTimerForNextTick(NextDelegate);
	}
	else
	{
		OnMoveFinished.Broadcast();
	}

}

