// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "PathFindInRadiusWorker.h"
#include "GridAffector.h"
#include "Kismet/KismetMathLibrary.h"


FPathFindToRadiusWorker::FPathFindToRadiusWorker(AGridSystemController* grid, FGridVector start, int32 MaxCost)
{
	Grid = grid;
	Start = start;
	Cost = MaxCost;
    bRunThread = true;
    Thread = FRunnableThread::Create(this, TEXT("PathFind"));
}

FPathFindToRadiusWorker::~FPathFindToRadiusWorker()
{
    if (Thread)
    {
        Thread->Kill();
        delete Thread;
    }

}

void FPathFindToRadiusWorker::Pathfind()
{
	while (OpenList.Num() > 0)
	{
		WorkOnNode();
	}
	bRunThread = false;
}

bool FPathFindToRadiusWorker::Init()
{
	Positions = Grid->GetUnblockedSquarePositionsInRange(Start, FGridVector{ 1, Cost, Cost });
	CurrentNode = TSharedPtr<FGridNode>(new FGridNode(Start, nullptr));
	OpenList.Add(CurrentNode);
	return true;
}

uint32 FPathFindToRadiusWorker::Run()
{
	Pathfind();
	return 0;
}

void FPathFindToRadiusWorker::Exit()
{
    bRunThread = false;

}

void FPathFindToRadiusWorker::Stop()
{
    bRunThread = false;
}

void FPathFindToRadiusWorker::AddNeighborsToArray(TSharedPtr<FGridNode> Node)
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
			OpenList.Add(NewNode);
		}
	}
	if (true, true)
	AddOtherConnected();

	OpenList.Remove(Node);
	CloseList.Add(Node);
}

void FPathFindToRadiusWorker::AddOtherConnected()
{
	auto Actors = Grid->GetActorsAtPos(CurrentNode->Coord);
	for (auto& Actor : Actors)
	{
		FGridVector Connected;
		if (Actor->Implements<UGridAffector>() && IGridAffector::Execute_IsConnector(Actor, CurrentNode->Coord, Connected) && Grid->GridPosHasFloor(Connected)
		 	&& !Grid->NodesContain(OpenList, Connected) && !Grid->NodesContain(CloseList, Connected))
		{
			FNodePtr NewNode(new FGridNode(Connected, CurrentNode));
			OpenList.Add(NewNode);
		}
	}
}

bool FPathFindToRadiusWorker::CreatePath(FNodePtr Node, FGridPath& Path)
{
	TArray<FGridVector> ResultPositions;
	ResultPositions.Add(Node->Coord);
	auto Parent = CurrentNode.Get()->P;
	while (Parent)
	{
		ResultPositions.Add(Parent.Get()->Coord);
		Parent = Parent.Get()->P;
	}

	//	if (ResultPositions.Num() > Cost - 1)
		//	return false;

	Algo::Reverse(ResultPositions);
	Path = FGridPath(ResultPositions);
	return true;
}

void FPathFindToRadiusWorker::StreightenPath(FGridPath& Path)
{

	TArray<FGridVector> PA = Path.ToArray();
	Algo::Reverse(PA);

	TArray<FNodePtr> Nodes;
	Nodes.Add(FNodePtr(new FGridNode(PA[0], nullptr)));

	for (int32 i = 1; i < PA.Num(); i++)
	{
		if (IsStraightLineToPos(Nodes.Last()->Coord, PA[i]))
		{
			continue;
		}
		else
			Nodes.Add(FNodePtr(new FGridNode(PA[i-1], Nodes.Last())));
	}
	CreatePath(Nodes[0], Path);
}

bool FPathFindToRadiusWorker::IsStraightLineToPos(const FGridVector& PosA, const FGridVector& PosB)
{
	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(PosA.ToWorld(), PosB.ToWorld());
	FVector Right = FRotator{ 0, 90, 0 }.RotateVector(Direction) * 45;
	FVector Left = FRotator{ 0, -90, 0 }.RotateVector(Direction) * 45;
	


	FGridVector Previous = PosA;
	FVector TestPos = PosA.ToWorld();
	while (TestPos != PosB.ToWorld())
	{
		if (Positions.Contains(FGridVector::FromFVector(TestPos)) 
			/* && Positions.Contains(FGridVector::ToFVector(TestPos + Right)) 
			&& Positions.Contains(FGridVector::ToFVector(TestPos + Left))*/)
		{
			Previous = FGridVector::FromFVector(TestPos);
			TestPos = TestPos + Direction * 95;
			UE_LOG(LogTemp, Warning, TEXT("TestPos: %s"), *TestPos.ToString());
		}
		else
			return false;
	}
	return true;

	
};


bool FPathFindToRadiusWorker::GetNodeOnPos(const FGridVector& Pos, FNodePtr& node)
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

TArray<FNodePtr> FPathFindToRadiusWorker::GetNodeNeighbors(FNodePtr node)
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

int32 FPathFindToRadiusWorker::GetNodeCost(FNodePtr node)
{
	return 0;
}

void FPathFindToRadiusWorker::WorkOnNode()
{
	FGridPath Path;
	CreatePath(CurrentNode, Path);
	StreightenPath(Path);
	Paths.Add(Path);

	AddNeighborsToArray(CurrentNode);

	if (OpenList.IsValidIndex(0))
		CurrentNode = OpenList[0];
	else
		CurrentNode = nullptr;
}

bool FPathFindToRadiusWorker::IsRunning()
{
    return bRunThread;
}
