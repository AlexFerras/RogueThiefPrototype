// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridFunctionLib.h"
#include "GridSystemController.generated.h"




USTRUCT(BlueprintType)
struct FActors
{
	GENERATED_BODY()

	FActors()
	{
	};
	
	FActors(TArray<AActor*> NewArray)
	{
		Array = NewArray;
	}

	UPROPERTY()
	TArray<class AActor*> Array;

	FORCEINLINE void Add(AActor* Actor)
	{
		Array.Add(Actor);
	};

	FORCEINLINE void Remove(AActor* Actor)
	{
		Array.Remove(Actor);
	}

};

USTRUCT(BlueprintType)
struct FYCoords
{
	GENERATED_BODY()

	FYCoords()
	{
	};


	FYCoords(TArray<FActors> NewArray)
	{
		Array = NewArray;
	};

	FYCoords(FActors NewInitElement, int32 Num)
	{
		Array.Init(NewInitElement, Num);
	};
	
	UPROPERTY()
	TArray<FActors> Array;

	FORCEINLINE FActors& operator[](int32 Index)
	{
		return Array[Index];
	}
	FORCEINLINE FActors operator[](int32 Index) const
	{
		return Array[Index];
	}
	FORCEINLINE void Init(FActors Value, int32 Num)
	{
		Array.Init(Value, Num);
	}

};


USTRUCT(BlueprintType)
struct FXCoords
{
	
	GENERATED_BODY()
	FXCoords()
	{
	};

	FXCoords(TArray<FYCoords> NewArray)
	{
		Array = NewArray;
	};

	FXCoords(FYCoords NewInitElement, int32 Num)
	{
		Array.Init(NewInitElement, Num);
	};


	UPROPERTY()
	TArray<FYCoords> Array;

	FORCEINLINE FYCoords& operator[](int32 Index)
	{
		return Array[Index];
	}
	FORCEINLINE FYCoords operator[](int32 Index) const
	{
		return Array[Index];
	}
	FORCEINLINE void Init(FYCoords Value, int32 Num)
	{
		Array.Init(Value,Num);
	}
};

USTRUCT(BlueprintType)
struct FGridNode
{
	GENERATED_BODY()

		FGridNode()
	{
		Coord = { 0,0,0 };
	};

	FGridNode(const FGridNode&)
	{

	};
	FGridNode(FGridVector Coord, TSharedPtr<FGridNode> Parent)
	{
		this->Coord = Coord;
		P = Parent;
	};

	FORCEINLINE int32 Cost() const
	{
		if (P)
			return 1 + P->Cost();
		return 1;
	}

	UPROPERTY()
	FGridVector Coord;
	TSharedPtr<FGridNode> P;
};



UCLASS()
class ROGUETHIEFPROTOTYPE_API AGridSystemController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridSystemController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void GenerateGrid();
	void FillFloor();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void SetGridPosition(AActor* Actor, FGridVector Pos);

	UFUNCTION()
	void FreeGridPosition(FGridVector Pos, AActor* Actor);

	UFUNCTION()
	void MoveGridPosition(AActor* Actor, FGridVector Pos);

	UFUNCTION()
	TArray<FGridVector> GetPositionsInRange(FGridVector Origin, FGridVector Range);

	UFUNCTION()
	FGridVector GetActorGridPosition(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetActorsAtPos(FGridVector Pos);

	bool ArePositionsConnected(const FGridVector& Pos1, const FGridVector& Pos2);

	bool GridPosHasFloor(const FGridVector& Pos);

	bool IsGridPosFree(FGridVector Pos);

	bool IsDiagnalConnected(const FGridVector& Pos1, const FGridVector& Pos2);

	TArray<FGridVector> GetConnectorsPositions(const FGridVector& Pos);

	static bool NodesContain(TArray<TSharedPtr<struct FGridNode>> Array, FGridVector Coord);

	TArray<FGridVector> GetUnblockedSquarePositionsInRange(const FGridVector& Origin, FGridVector Range);


	

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<FXCoords> Grid;

	TArray<FGridVector> DefaultFloorPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 FillFloorRange;
};
