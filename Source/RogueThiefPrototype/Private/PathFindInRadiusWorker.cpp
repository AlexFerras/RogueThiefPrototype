// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "PathFindInRadiusWorker.h"
#include "GridAffector.h"
#include "Kismet/KismetMathLibrary.h"
#include "StraightenPathWorker.h"


#define ADJUSTVALUES { {0, 1, 0}, {0, 0, 1}, {0, -1, 0}, {0, 0, -1}, {0, 1, 1}, {0, 1, -1}, {0, -1, 1}, {0, -1, -1} };

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
	if (Paths.Num() < 50)
	{
		for (auto& Path : Paths)
		{
			StreightenPath(Path);
		}
	}

	TArray<FGridPath> Paths1;
	TArray<FGridPath> Paths2;

	for (int32 i = 0; i < Paths.Num(); i++)
	{
		if (i % 2 == 0)
			Paths1.Add(Paths[i]);
		else
			Paths2.Add(Paths[i]);
	}
	Worker1 = new FStraightenPathWorker(Grid, Paths1, Positions, 0);
	Worker2 = new FStraightenPathWorker(Grid, Paths2, Positions, 1);

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

	while (bRunThread)
	{
		if (!Worker1->IsRunning() && !Worker2->IsRunning())
		{
			Paths.Empty();
			Paths.Append(Worker1->ResultPaths);
			Paths.Append(Worker2->ResultPaths);
			Worker1->Stop();
			delete Worker1;
			Worker2->Stop();
			delete Worker2;
			return 0;

		}
	}
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
	TArray<FGridVector> AdjustValues = ADJUSTVALUES;


	for (int32 i = 0; i < AdjustValues.Num(); i++)
	{
		FGridVector TestPos = CurrentNode.Get()->Coord + AdjustValues[i];
		if (Positions.Contains(TestPos) && !Grid->NodesContain(OpenList, TestPos) && !Grid->NodesContain(CloseList, TestPos))
		{
			if ((Grid->ArePositionsConnected(CurrentNode.Get()->Coord, TestPos)) || (Grid->IsDiagnalConnected(CurrentNode.Get()->Coord, TestPos)))
			{
				TSharedPtr<FGridNode> NewNode(new FGridNode(TestPos, CurrentNode));
				//if (NewNode->Cost() < Cost)
				OpenList.Add(NewNode);
			}
		}
	}
	OpenList.Remove(Node);
	CloseList.Add(Node);

	AddOtherConnected();
}

void FPathFindToRadiusWorker::AddOtherConnected()
{
	auto Actors = Grid->GetActorsAtPos(CurrentNode->Coord);
	for (auto& Actor : Actors)
	{
		FGridVector Connected;
		if (Actor->Implements<UGridAffector>() && IGridAffector::Execute_IsConnector(Actor, CurrentNode->Coord, Connected) && Grid->IsGridPosFree(Connected)
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
	auto Parent = Node.Get()->P;
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

	if (PA.Num() < 2)
		return;

	auto PathStart = PA[0];

	TArray<AActor*> TempRemovedBlockers;
	for (auto& Actor : Grid->GetActorsAtPos(PathStart))
	{
		if (Actor->Implements<UGridAffector>() && IGridAffector::Execute_IsBlock(Actor))
		{
			TempRemovedBlockers.Add(Actor);
			Grid->FreeGridPosition(PathStart, Actor);
		}
	}

	TFunction<void()> SetBlockersBack = [&]()
	{
		for (auto& Blocker : TempRemovedBlockers)
		{
			Grid->SetGridPosition(Blocker, PathStart);
		}
	};

	PA.RemoveAt(0);
	auto PathTarget = PA.Last();
	FGridVector Hit;


	TArray<FNodePtr> Nodes;
	Nodes.Add(FNodePtr(new FGridNode(PathStart, nullptr)));

	if (!HitTestStraightLine(PathStart, PathTarget, Hit, PathStart))
	{
		Nodes.Add(FNodePtr(new FGridNode(PathTarget, Nodes.Last())));
		FGridPath LocalP;
		CreatePath(Nodes.Last(), LocalP);
		SetBlockersBack();
		Path = LocalP;
		return;
	}

		for (int32 i = 0; i < PA.Num(); i++)
		{
			if (!HitTestStraightLine(Nodes.Last()->Coord, PA[i], Hit, PathStart))
			{
				if (PA[i] == PathTarget)
					Nodes.Add(FNodePtr(new FGridNode(PA[i], Nodes.Last())));
			}
			else
			{
				if (PA[i].Z != (PA.IsValidIndex(i - 1) ? PA[i - 1].Z : PathStart.Z))
				{
					auto Poses = Grid->GetConnectorsPositions(PA[i]);
					for (auto& Pos : Poses)
					{
						if (PA.Contains(Pos))
						{
							Nodes.Add(FNodePtr(new FGridNode(Pos, Nodes.Last())));
						}
					}
				}

				bool bFound = false;
				for (int32 ri = i - 1; ri > PA.Find(Nodes.Last()->Coord); ri--)
				{
					if (HitTestStraightLine(PA[ri], PA[i], Hit, PathStart))
					{
						Nodes.Add(FNodePtr(new FGridNode(PA[ri + 1], Nodes.Last())));
						bFound = true;
					}
				}
				if (!bFound)
					Nodes.Add(FNodePtr(new FGridNode(PA.IsValidIndex(i - 1) ? PA[i - 1] : PA[i], Nodes.Last())));

				//Nodes.Add(FNodePtr(new FGridNode(PA[i - 1], Nodes.Last())));
				//Nodes.Add(FNodePtr(new FGridNode(PA[i], Nodes.Last())));
			}

		}
	if (!Grid->NodesContain(Nodes, PathTarget))
	{
		Nodes.Add(FNodePtr(new FGridNode(PathTarget, Nodes.Last())));
	}

	SetBlockersBack();
	CreatePath(Nodes.Last(), Path);
}

bool FPathFindToRadiusWorker::HitTestStraightLine(const FGridVector& PosA, const FGridVector& PosB, FGridVector& Hit, FGridVector IgnoredPos)
{
	if (PosA.Z != PosB.Z)
		return true;


	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(PosA.ToWorld(), PosB.ToWorld());


	FVector Right = FRotator{ 0, 90, 0 }.RotateVector(Direction) * 45.f;
	FVector Left = FRotator{ 0, -90, 0 }.RotateVector(Direction) * 45.f;


	FGridVector Previous = PosA;
	FVector TestPos = PosA.ToWorld();
	auto GridTestPos = FGridVector::FromFVector(TestPos);

	Hit = FGridVector();

	TFunction<void()> Lambda = [&]()
	{
		TestPos += Direction * 24.5f;
		if (FGridVector::FromFVector(TestPos) != GridTestPos)
			GridTestPos = FGridVector::FromFVector(TestPos);

	};
	while (GridTestPos == Previous)
		Lambda();

	if (FGridVector::FromFVector(TestPos) == PosB)
	{
		return !(Positions.Contains(GridTestPos)
			&& ((Grid->ArePositionsConnected(Previous, GridTestPos) || Grid->IsDiagnalConnected(Previous, GridTestPos))
				&& Grid->IsDiagnalConnected(GridTestPos, FGridVector::FromFVector(TestPos + Right))
				&& Grid->IsDiagnalConnected(GridTestPos, FGridVector::FromFVector(TestPos + Left))
				&& Grid->IsDiagnalConnected(Previous, FGridVector::FromFVector(TestPos + Right))
				&& Grid->IsDiagnalConnected(Previous, FGridVector::FromFVector(TestPos + Left))
				));

	}


	while (FGridVector::FromFVector(TestPos) != PosB)

	{

		//for (const auto& c : FGridVector::DiagnalConnectors(Previous, GridTestPos))
		//{
		//	PrintStraight(c);
		//}

		while (GridTestPos == Previous)
			Lambda();

		if (Positions.Contains(GridTestPos)
			&& ((Grid->ArePositionsConnected(Previous, GridTestPos) || Grid->IsDiagnalConnected(Previous, GridTestPos))
				&& Grid->IsDiagnalConnected(GridTestPos, FGridVector::FromFVector(TestPos + Right))
				&& Grid->IsDiagnalConnected(GridTestPos, FGridVector::FromFVector(TestPos + Left))
				&& Grid->IsDiagnalConnected(Previous, FGridVector::FromFVector(TestPos + Right))
				&& Grid->IsDiagnalConnected(Previous, FGridVector::FromFVector(TestPos + Left))/* || Grid->IsDiagnalConnected(Previous, GridTestPos)*/)
			)
		{
			Previous = GridTestPos;
		}
		else
		{
			Hit = Previous;
			return true;
		}
	}
	return false;


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
	if (CurrentNode->Coord != Start)
	{
		FGridPath Path;
		CreatePath(CurrentNode, Path);
		Paths.Add(Path);
	}

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
