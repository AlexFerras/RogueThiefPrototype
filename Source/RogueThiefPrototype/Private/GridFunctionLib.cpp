// Fill out your copyright notice in the Description page of Project Settings.

#include "GridAffector.h"
#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "GridFunctionLib.h"


FGridVector UGridFunctionLib::ConvertWorldToGrid(FVector Location)
{
	int32 NewX = FMath::Floor(Location.X / 100);
	int32 NewY = FMath::Floor(Location.Y / 100);
	int32 NewZ = FMath::Floor(Location.Z / 200);
	return FGridVector{ NewZ,NewX,NewY };


}

FVector UGridFunctionLib::ConvertGridToWorld(FGridVector B)
{
	return B.ToWorld();

}

bool UGridFunctionLib::IsWorldLocationOnGrid(FVector Location)
{	
	int32 TestX = (int32)Location.X - 50;
	int32 TextY = (int32)Location.Y - 50;
	int32 TextZ = (int32)Location.Z - 50;

	if (TestX % 100 != 0)
		return false;
	if (TextY % 100 != 0)
		return false;
	if (TextZ % 200 != 0)
		return false;

	return true;
	
}

bool UGridFunctionLib::IsGridPositionValid(FGridVector B)
{
	if ((1 <= B.X && B.X < 100) && (1 <= B.Y && B.Y < 100) && (0<= B.Z && B.Z <= 1))
		return true;
	else
		return false;
}

TArray<FGridVector> UGridFunctionLib::ToArray(FGridPath Path)
{
	return Path.ToArray();
}

bool UGridFunctionLib::IsValid(FGridPath Path)
{
	return Path.IsValid();
}

FGridPath UGridFunctionLib::GetPathByPos(TArray<FGridPath> Paths, FGridVector Pos)
{
	for (auto& Path : Paths)
	{
		if (Path.ToArray().Last() == Pos)
			return Path;
	}
	return FGridPath();
}

int32 UGridFunctionLib::GetCost(FGridPath Path)
{
	return Path.Cost();
}

bool UGridFunctionLib::IsEqual(FGridVector A, FGridVector B)
{
	if (A == B)
		return true;
	return false;
}

FRotator UGridFunctionLib::GetRotationToGrid(const FVector& ForwardVector)
{
	TArray<FVector> Directions;
	Directions.Add({ 1.f,0.f,0.f });
	Directions.Add({ 0.f, 1.f, 0.f });
	Directions.Add({ -1.f, 0.f, 0.f });
	Directions.Add({ 0.f,-1.f,0.f });

	for (int32 i = 0; i < 4; i++)
	{
		if (FVector::DotProduct(ForwardVector, Directions[i]) > 0.5f)
		{
			return Directions[i].Rotation();
		}
		
	}
	return FRotator::ZeroRotator;
}

bool UGridFunctionLib::NodesContain(TArray<TSharedPtr<struct FGridNode>> Array, FGridVector Coord)
{
	for (auto& Node : Array)
	{
		if (Node.Get()->Coord == Coord)
			return true;
	}
	return false;
}

