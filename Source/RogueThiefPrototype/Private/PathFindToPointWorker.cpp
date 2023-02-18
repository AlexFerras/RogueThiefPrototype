// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "PathFindToPointWorker.h"


FPathFindToPointWorker::FPathFindToPointWorker(AGridSystemController* grid, FGridVector start, TArray<FGridVector> Targets, FGridVector Range, int32 MaxCost)
{
	Grid = grid;
	Start = start;
	Cost = MaxCost;
    bRunThread = true;
    Thread = FRunnableThread::Create(this, TEXT("PathFind"));
}

FPathFindToPointWorker::~FPathFindToPointWorker()
{
    if (Thread)
    {
        Thread->Kill();
        delete Thread;
    }

}

void FPathFindToPointWorker::Pathfind()
{
	while (OpenList.Num() > 0)
	{
		if (!Path.IsValid())
			WorkOnNode();
		break;
	}
	bRunThread = false;
}

bool FPathFindToPointWorker::Init()
{
	Positions = Grid->GetUnblockedSquarePositionsInRange(Start, FGridVector{ 1, Cost, Cost });
	CurrentNode = TSharedPtr<FGridNode>(new FGridNode(Start, nullptr));
	OpenList.Add(CurrentNode);
	return true;
}

uint32 FPathFindToPointWorker::Run()
{
	Pathfind();
	return 0;
}

void FPathFindToPointWorker::Exit()
{
    bRunThread = false;

}

void FPathFindToPointWorker::Stop()
{
    bRunThread = false;
}

void FPathFindToPointWorker::AddNeighborsToArray(TSharedPtr<FGridNode> Node)
{
	TArray<FGridVector> AdjustValues;
	AdjustValues.Add({ 0,1,0 });
	AdjustValues.Add({ 0,0,1 });
	AdjustValues.Add({ 0,-1,0 });
	AdjustValues.Add({ 0,0,-1 });

	for (int32 i = 0; i < AdjustValues.Num(); i++)
	{
		FGridVector TestPos = CurrentNode.Get()->Coord + AdjustValues[i];
		if (Positions.Contains(TestPos) && !Grid->NodesContain(OpenList, TestPos) && !Grid->NodesContain(CloseList, TestPos))
		{
			if (!(Grid->ArePositionsConnected(CurrentNode.Get()->Coord, TestPos)))
				continue;
			TSharedPtr<FGridNode> NewNode(new FGridNode(TestPos, CurrentNode));
			if (NewNode->Cost() < Cost)
				OpenList.Add(NewNode);
		}
	}
	OpenList.Remove(Node);
	CloseList.Add(Node);
}

bool FPathFindToPointWorker::CreatePath(FNodePtr Node, FGridPath& path)
{
	TArray<FGridVector> ResultPositions;
	ResultPositions.Add(Target);
	auto Parent = CurrentNode.Get()->P;
	while (Parent)
	{
		ResultPositions.Add(Parent.Get()->Coord);
		Parent = Parent.Get()->P;
	}

	//	if (ResultPositions.Num() > Cost - 1)
		//	return false;

	Algo::Reverse(ResultPositions);
	path = FGridPath(ResultPositions);
	return true;
}

bool FPathFindToPointWorker::GetNodeOnPos(const FGridVector& Pos, FNodePtr& node)
{
	for (auto& Node : OpenList)
	{
		if (Node && Node->Coord == Pos)
		{
			node = Node;
			break;
		}
	}
	//for (auto& Node : CloseList)
	//{
	//	if (Node && Node->Coord == Pos)
	//	{
	//		node = Node;
	//		break;
	//	}
	//}

	return node != nullptr;

}

TArray<FNodePtr> FPathFindToPointWorker::GetNodeNeighbors(FNodePtr node)
{
	TArray<FGridVector> AdjustValues;
	AdjustValues.Add({ 0,1,0 });
	AdjustValues.Add({ 0,0,1 });
	AdjustValues.Add({ 0,-1,0 });
	AdjustValues.Add({ 0,0,-1 });
	TArray<FNodePtr> Nodes;

	for (int32 i = 0; i < 4; i++)
	{
		FNodePtr Node;
		if (GetNodeOnPos(node->Coord + AdjustValues[i], Node) && node->P != Node)
		{
			Nodes.Add(Node);
		}
	}
	return Nodes;
}

int32 FPathFindToPointWorker::GetNodeCost(FNodePtr node)
{
	return node->Coord.ManhattanDistTo(Target) + node->Cost();
}

void FPathFindToPointWorker::WorkOnNode()
{
	if (CurrentNode->Coord != Target)
		AddNeighborsToArray(CurrentNode);
	else
	{
		CreatePath(CurrentNode, Path);
	}
	int32 CurrentCost = 100;
	int32 MaxIndex = 0;

	for (int32 i = 0; i < OpenList.Num(); i++)
	{
		int32 NewCost = GetNodeCost(OpenList[i]);
		if (NewCost <= CurrentCost)
		{
			CurrentCost = NewCost;
			MaxIndex = i;
		}
	}
	if (OpenList.IsValidIndex(MaxIndex))
		CurrentNode = OpenList[MaxIndex];
	else
		CurrentNode = nullptr;
}

bool FPathFindToPointWorker::IsRunning()
{
    return bRunThread;
}
