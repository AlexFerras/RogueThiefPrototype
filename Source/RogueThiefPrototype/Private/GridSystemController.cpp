// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSystemController.h"
#include "GridComponent.h"
#include "GridAffector.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGridSystemController::AGridSystemController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GenerateGrid();

}

// Called when the game starts or when spawned
void AGridSystemController::BeginPlay()
{
	Super::BeginPlay();
	FillFloor();
	
}

void AGridSystemController::GenerateGrid()
{	// ZCoords
	TArray<AActor*> AssignedArray;
	AssignedArray.Empty();
	FActors ActorsInCoord(AssignedArray);

	FYCoords ZCoord(ActorsInCoord, 100);
	FXCoords YCoord(ZCoord, 100);
	Grid.Init(YCoord, 2);

}

void AGridSystemController::FillFloor()
{
	auto Positions = GetPositionsInRange({ 0,0,0 }, { 0, FillFloorRange, FillFloorRange });
	for (const auto& Position : Positions)
	{
		DefaultFloorPositions.Add(Position);
	}

}

void AGridSystemController::SetGridPosition(AActor* Actor, FGridVector Pos)
{
	if (UGridFunctionLib::IsGridPositionValid(Pos) && !Grid[Pos.Z][Pos.X][Pos.Y].Array.Contains(Actor))
		Grid[Pos.Z][Pos.X][Pos.Y].Array.Add(Actor);

}

void AGridSystemController::FreeGridPosition(FGridVector Pos, AActor* Actor)
{
	if (UGridFunctionLib::IsGridPositionValid(Pos))
		Grid[Pos.Z][Pos.X][Pos.Y].Remove(Actor);
}

void AGridSystemController::MoveGridPosition(AActor* Actor, FGridVector Pos)
{
	FreeGridPosition(GetActorGridPosition(Actor), Actor);
	SetGridPosition(Actor, Pos);
}

TArray<FGridVector> AGridSystemController::GetPositionsInRange(FGridVector Origin, FGridVector Range)
{
	TArray<FGridVector> Positions;

	for (int32 IndexZ = 0; IndexZ < Grid.Num(); IndexZ++)
	{
		if (IndexZ < Origin.Z - Range.Z)
			continue;
		if (IndexZ > Origin.Z + Range.Z)
			break;
		for (int32 IndexX = 0; IndexX < Grid[IndexZ].Array.Num(); IndexX++)
		{
			if (IndexX < Origin.X - Range.X)
				continue;
			if (IndexX > Origin.X + Range.X)
				break;
			for (int32 IndexY = 0; IndexY < Grid[IndexZ][IndexX].Array.Num(); IndexY++)
			{
				if (IndexY < Origin.Y-Range.Y)
					continue;
				if (IndexY > Origin.Y + Range.Y)
					break;
				Positions.Add({ IndexZ, IndexX, IndexY });
			}
		}
	}


	return Positions;
}


FGridVector AGridSystemController::GetActorGridPosition(AActor* Actor)
{
	UGridComponent* Component = Cast<UGridComponent>(Actor->GetComponentByClass(UGridComponent::StaticClass()));
	if (Component)
		return Component->GetGridPosition();
	else
		return {0,0,0};
}

TArray<AActor*> AGridSystemController::GetActorsAtPos(FGridVector Pos)
{
	TArray<AActor*> EmptyArray;

	if (!IsValid(this))
	{
		return EmptyArray;
	}

	if (UGridFunctionLib::IsGridPositionValid(Pos))
		return Grid[Pos.Z][Pos.X][Pos.Y].Array;
	else
	{
		return EmptyArray;
	}
}

bool AGridSystemController::ArePositionsConnected(const FGridVector& Pos1, const FGridVector& Pos2)
{
	if (Pos1 == Pos2)
		return true;

	if (!UGridFunctionLib::IsGridPositionValid(Pos1) || !UGridFunctionLib::IsGridPositionValid(Pos2))
		return false;
	
	if (!IsGridPosFree(Pos1) || !IsGridPosFree(Pos2))
	{
		return false;
	}
	if (FGridVector::IsDiagnal(Pos1, Pos2))
		return false;


	TArray<AActor*> Actors = GetActorsAtPos(Pos1);
	for (auto& Actor : Actors)
	{
		if (!Actor->Implements<UGridAffector>())
			continue;
		FGridVector WallPos1;
		FGridVector WallPos2;
		if (!IGridAffector::Execute_IsWall(Actor, WallPos1, WallPos2))
			continue;

		if (WallPos1 == Pos1 && WallPos2 == Pos2)
			return false;

		if (WallPos1 == Pos2 && WallPos2 == Pos1)
			return false;

	}
	return true;
}

bool AGridSystemController::GridPosHasFloor(const FGridVector& Pos)
{
	if (DefaultFloorPositions.Contains(Pos))
		return true;

	auto Actors = GetActorsAtPos(Pos);
	for (auto& Actor : Actors)
	{
		if (Actor->Implements<UGridAffector>() && IGridAffector::Execute_IsFloor(Actor))
			return true;
	}
	return false;
}

bool AGridSystemController::IsGridPosFree(FGridVector Pos)
{
	if (!UGridFunctionLib::IsGridPositionValid(Pos))
		return false;
	if (!GridPosHasFloor(Pos))
		return false;
	
	auto Actors = GetActorsAtPos(Pos);
	for (auto& Actor : Actors)
	{
		if (!Actor->Implements<UGridAffector>())
			continue;

		if (IGridAffector::Execute_IsBlock(Actor))
			return false;

	}
	return true;
}

bool AGridSystemController::IsDiagnalConnected(const FGridVector& Pos1, const FGridVector& Pos2)
{
	if (ArePositionsConnected(Pos1, Pos2))
		return true;

	int32 Y = Pos1.Y - Pos2.Y;
	FGridVector D = Pos1 - Pos2;
	
	UE_LOG(LogTemp, Warning, TEXT("Diag fail result: X = %i Y = %i"), D.X, Y);
	if (FMath::Abs(D.X) + FMath::Abs(D.Y) != 2)
	{
		return false;

	}
	

	auto Connectors = FGridVector::DiagnalConnectors(Pos1, Pos2);
	if (Connectors.Num() == 0)
		return false;

	for (auto& c : Connectors)
	{
		if (!(ArePositionsConnected(Pos1, c) && ArePositionsConnected(c, Pos2)))
			return false;
	}
	return true;

}

bool AGridSystemController::NodesContain(TArray<TSharedPtr<struct FGridNode>> Array, FGridVector Coord)
{
	for (auto& Node : Array)
	{
		if (Node.Get()->Coord == Coord)
			return true;
	}
	return false;
}


TArray<FGridVector> AGridSystemController::GetUnblockedSquarePositionsInRange(const FGridVector& Origin, FGridVector Range)
{
	auto Positions = GetPositionsInRange(Origin, Range);
	TArray<FGridVector> NewPositions;
	for (int32 i = 0; i < Positions.Num(); i++)
	{
		if (GridPosHasFloor(Positions[i]) && IsGridPosFree(Positions[i]))
			NewPositions.Add(Positions[i]);
	}
	return NewPositions;
}

// Called every frame
void AGridSystemController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

