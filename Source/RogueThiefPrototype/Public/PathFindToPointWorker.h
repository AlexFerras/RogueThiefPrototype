// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridFunctionLib.h"
#include "HAL/Runnable.h"
#include "GridSystemController.h"
typedef TSharedPtr<FGridNode> FNodePtr;

/**
 * 
 */
class ROGUETHIEFPROTOTYPE_API FPathFindToPointWorker : public FRunnable
{
public:
	FPathFindToPointWorker(AGridSystemController* grid, FGridVector start, TArray<FGridVector> Targets, FGridVector Range, int32 MaxCost);
	~FPathFindToPointWorker();

	// FRunnable begin
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	// FRunnable end

	void Pathfind();

	void AddNeighborsToArray(TSharedPtr<FGridNode> Node);

	bool CreatePath(FNodePtr Node, FGridPath& path);

	bool GetNodeOnPos(const FGridVector& Pos, FNodePtr& node);

	TArray<FNodePtr> GetNodeNeighbors(FNodePtr node);

	int32 GetNodeCost(FNodePtr node);

	void WorkOnNode();

	bool IsRunning();

public:
	class AGridSystemController* Grid;

	TArray<FGridVector> Positions;
	TSharedPtr<FGridNode> CurrentNode;
	TSharedPtr<FGridNode> TargetNode;
	TArray<TSharedPtr<FGridNode>> OpenList;
	TArray<TSharedPtr<FGridNode>> CloseList;
	FGridPath Path;


	FGridVector Start;
	FGridVector Target;
	int32 Cost;


private:
	FRunnableThread* Thread;
	bool bRunThread;
};
