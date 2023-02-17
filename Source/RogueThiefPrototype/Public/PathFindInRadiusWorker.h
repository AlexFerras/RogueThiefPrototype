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
class ROGUETHIEFPROTOTYPE_API FPathFindToRadiusWorker : public FRunnable
{
public:
	FPathFindToRadiusWorker(AGridSystemController* grid, FGridVector start, int32 MaxCost);
	~FPathFindToRadiusWorker();

	// FRunnable begin
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	// FRunnable end

	void Pathfind();

	void AddNeighborsToArray(TSharedPtr<FGridNode> Node);

	void AddOtherConnected();

	bool CreatePath(FNodePtr Node, FGridPath& Path);

	void StreightenPath(FGridPath& Path);

	bool HitTestStraightLine(const FGridVector& PosA, const FGridVector& PosB, FGridVector& Hit, FGridVector IgnoredPos = FGridVector());

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
	TArray<FGridPath> Paths;


	FGridVector Start;
	int32 Cost;


private:
	FRunnableThread* Thread;
	bool bRunThread;

	class FStraightenPathWorker* Worker1;
	class FStraightenPathWorker* Worker2;
};
