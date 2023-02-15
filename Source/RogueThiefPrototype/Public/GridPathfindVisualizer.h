// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridFunctionLib.h"
#include "GridPathfindVisualizer.generated.h"

typedef TSharedPtr<FGridNode> FNodePtr;

UCLASS()
class ROGUETHIEFPROTOTYPE_API AGridPathfindVisualizer : public AActor
{
	GENERATED_BODY()


protected:
	class AGridSystemController* Grid;

	TArray<FGridVector> Positions;
	TSharedPtr<FGridNode> CurrentNode;
	TSharedPtr<FGridNode> TargetNode;
	TArray<TSharedPtr<FGridNode>> OpenList;
	TArray<TSharedPtr<FGridNode>> CloseList;
	TArray<FGridPath> Paths;
	FGridPath ResultPath;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FGridVector Start;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	FGridVector Target;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	int32 Cost;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings)
	float Interval;
	
public:	
	// Sets default values for this actor's properties
	AGridPathfindVisualizer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Pathfind();

	void AddNeighborsToArray(TSharedPtr<FGridNode> Node);

	bool IsTarget(TSharedPtr<FGridNode> Node);

	bool CreatePath(const FNodePtr Node, FGridPath& Path);

	UFUNCTION(BlueprintImplementableEvent)
	void ReturnPaths(const TArray<FGridPath>& Path);

	UFUNCTION(BlueprintImplementableEvent)
	void PrintCurrentNode(const FGridVector& Position, const FVector& DirectionToParent);

	UFUNCTION(BlueprintImplementableEvent)
	void PrintPath(const FGridPath& Path);

	UFUNCTION(BlueprintImplementableEvent)
	void PrintStraight(const FGridVector& Pos);

	bool GetNodeOnPos(const FGridVector& Pos, FNodePtr& node);

	bool OnlyTargetRemains();

	void StreightenPath(FGridPath& Path);

	bool IsStraightLineToPos(const FGridVector& PosA, const FGridVector& PosB);

	UFUNCTION()
	void ReleaseTargetNeighbors();


	TArray<FNodePtr> GetNodeNeighbors(FNodePtr node);

	int32 GetNodeCost(FNodePtr node);

	UFUNCTION()
	void WorkOnNode();

	void Init();

	FGridPath GetShortestPath();

};
