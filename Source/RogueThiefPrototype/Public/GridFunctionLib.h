// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "GridFunctionLib.generated.h"


USTRUCT(BlueprintType)
struct FGridVector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Z;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y;

	FGridVector() { Z = 0; X = 0; Y = 0; };
	FGridVector(int32 newZ, int32 newX, int32 newY) { Z = newZ; X = newX; Y = newY; };

	FORCEINLINE friend bool operator==(const FGridVector B, const FGridVector C)
	{
		if (C.X == B.X && C.Y == B.Y && C.Z == B.Z)
			return true;
		return false;
	}

	FORCEINLINE friend bool operator!=(const FGridVector B, const FGridVector C)
	{
		if (C.X == B.X && C.Y == B.Y && C.Z == B.Z)
			return false;
		return true;
	}

	FORCEINLINE friend bool operator ==(const FVector A, const FGridVector B)
	{
		return FGridVector::FromFVector(A) == B;
	}

	FORCEINLINE operator FVector() const
	{
		return this->ToWorld();
	}

	FORCEINLINE static FGridVector FromFVector(const FVector& B)
	{
		int32 NewX = FMath::Floor(B.X / 100);
		int32 NewY = FMath::Floor(B.Y / 100);
		int32 NewZ = FMath::Floor(B.Z / 200);

		return FGridVector{ NewZ,NewX,NewY };
	}

	FORCEINLINE static TArray<FGridVector> DiagnalConnectors(const FGridVector& A, const FGridVector& B)
	{
		if (!IsDiagnal(A,B))
			return TArray<FGridVector>();
		TArray<FGridVector> Results;
		// X Connector
		Results.Add(FGridVector{ A.Z, B.X, A.Y });
		
		// Y Connector
		Results.Add(FGridVector{ A.Z, A.X, B.Y });

		return Results;
	}

	FORCEINLINE static bool IsDiagnal(const FGridVector& A, const FGridVector& B)
	{
		auto D = A - B;
		return FMath::Abs(D.X) + FMath::Abs(D.Y) == 2;

	}


	FORCEINLINE FGridVector operator- (const FGridVector& B) const
	{
		FGridVector result;
		result.Z = this->Z - B.Z;
		result.X = this->X - B.X;
		result.Y = this->Y - B.Y;
		return result;

	}

	FORCEINLINE int32 ManhattanDistanceTo(const FGridVector& B) const
	{
		return FMath::Abs(this->X - B.X) + FMath::Abs(this->Y - B.Y) + FMath::Abs(this->Z - B.Z);
	}

	FORCEINLINE int32 DiagonalDistanceTo(const FGridVector& B) const
	{
		int32 dx = abs(B.X - this->X);
		int32 dy = abs(B.Y - this->Y);

		int32 min = FMath::Min(dx, dy);
		int32 max = FMath::Min(dx, dy);

		int32 diagonalSteps = min;
		int32 straightSteps = max - min;

		return sqrt(2) * diagonalSteps + straightSteps;


	}


	FORCEINLINE FGridVector operator+(const FGridVector B) const
	{
		int32 NewX = this->X + B.X;
		int32 NewY = this->Y + B.Y;
		int32 NewZ = this->Z + B.Z;
		return FGridVector{ NewZ, NewX, NewY };
	};

	FORCEINLINE FVector operator*(const float B) const
	{
		float NewX = this->X * B;
		float NewY = this->Y * B;
		float NewZ = this->Z * B;
		return FVector{ NewX,NewY,NewZ };
	}
	FORCEINLINE FGridVector operator*(const int32 B) const
	{
		return FGridVector{ this->X * B, this->Y * B, this->Z * B };
	}

	FORCEINLINE int32 Length() const
	{
		float Sum = pow(this->X, 2) + pow(this->Y, 2) + pow(this->Z, 2);
		float len = sqrt(Sum);
		return (int32)len;
	}

	FORCEINLINE FVector ToWorld() const
	{
		float NewX = (this->X * 100.f) + 50.f;
		float NewY = (this->Y * 100.f) + 50.f;
		float NewZ = (this->Z * 200.f) + 50.f;
		return FVector(NewX, NewY, NewZ);
	}

	FORCEINLINE FGridVector operator+ (const FVector& B) const
	{
		FGridVector New;

		New.X = this->X + B.X;
		New.Y = this->Y + B.Y;
		New.Z = this->Z + B.Z;

		return New;

	}
};

USTRUCT(BlueprintType)
struct FGridPath
{
	GENERATED_BODY()

	FGridPath()
	{
	};
	FGridPath(TArray<FGridVector> NewArray)
	{
		Path = NewArray;
	};


	FORCEINLINE int32 Cost() const
	{
		return Path.Num();
	};

	FORCEINLINE TArray<FGridVector> ToArray() const
	{
		return Path;
	}

	FORCEINLINE FGridVector Target() const
	{
		return Path.Last();
	}

	FORCEINLINE bool IsValid() const
	{
		if (Path.Num() > 0)
			return true;
		return false;
	}




	UPROPERTY()
	TArray<FGridVector> Path;

};
/**
 * 
 */

UCLASS()
class ROGUETHIEFPROTOTYPE_API UGridFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static FGridVector ConvertWorldToGrid(FVector Location);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static FVector ConvertGridToWorld(FGridVector Position);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static bool IsWorldLocationOnGrid(FVector Location);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static bool IsGridPositionValid(FGridVector Possition);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static TArray<FGridVector> ToArray(FGridPath Path);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static bool IsValid(FGridPath Path);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static FGridPath GetPathByPos(TArray<FGridPath> Paths, FGridVector Pos);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static int32 GetCost(FGridPath Path);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static bool IsEqual(FGridVector A, FGridVector B);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Grid")
	static FRotator GetRotationToGrid(const FVector& ForwardVector);

	static bool NodesContain(TArray<TSharedPtr<struct FGridNode>> Array, FGridVector Coord);
};
