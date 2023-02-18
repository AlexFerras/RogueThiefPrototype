// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridFunctionLib.h"
#include "GridSystemController.h"

typedef TSharedPtr<FGridNode> FNodePtr;

/**
 * 
 */
class ROGUETHIEFPROTOTYPE_API FStraightenPathWorker : public FRunnable
{
public:
	FStraightenPathWorker(AGridSystemController* Grid, TArray<FGridPath> Paths, TArray<FGridVector> positions, int32 Index);
	~FStraightenPathWorker();


	// FRunnable begin
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	// FRunnable end

	bool HitTestStraightLine(const FGridVector& PosA, const FGridVector& PosB, FGridVector& Hit, FGridVector IgnoredPos = FGridVector());

	void StreightenPath(FGridPath& Path);

	bool CreatePath(FNodePtr Node, FGridPath& Path);
	
	bool IsRunning();

public: 

	AGridSystemController* Grid;
	TArray<FGridVector> Positions;
	TArray<FGridPath> Paths;
	TArray<FGridPath> ResultPaths;


private:

	FRunnableThread* Thread;

	bool bRunThread;
};
