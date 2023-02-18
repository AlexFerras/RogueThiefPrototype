// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/KismetMathLibrary.h"
#include "StraightenPathWorker.h"
#include "GridAffector.h"



FStraightenPathWorker::FStraightenPathWorker(AGridSystemController* Grid, TArray<FGridPath> Paths, TArray<FGridVector> positions, int32 Index)
{
	this->Grid = Grid;
	this->Paths = Paths;
	Positions = positions;
	Thread = FRunnableThread::Create(this, TEXT("Straight"));
	bRunThread = true;
}

FStraightenPathWorker::~FStraightenPathWorker()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

bool FStraightenPathWorker::Init()
{
	return true;
}

uint32 FStraightenPathWorker::Run()
{
	for (auto& Path : Paths)
	{
		StreightenPath(Path);
		ResultPaths.Add(Path);
	}
	bRunThread = false;
	return 0;
}

void FStraightenPathWorker::Exit()
{
	bRunThread = false;
}

void FStraightenPathWorker::Stop()
{
	bRunThread = false;
}

bool FStraightenPathWorker::HitTestStraightLine(const FGridVector& PosA, const FGridVector& PosB, FGridVector& Hit, FGridVector IgnoredPos)
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


}

void FStraightenPathWorker::StreightenPath(FGridPath& Path)
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

	TFunction<void()> CheckLastToTarget = [&]()
	{

		if (!HitTestStraightLine(Nodes.Last()->Coord, PathTarget, Hit, PathStart))
		{
			Nodes.Add(FNodePtr(new FGridNode(PathTarget, Nodes.Last())));
			FGridPath LocalP;
			CreatePath(Nodes.Last(), LocalP);
			SetBlockersBack();
			Path = LocalP;
			return;
		}
	};

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
			for (int32 ri = PA.IsValidIndex(i - 1) ? PA.Find(Nodes.Last()->Coord) + 1 : i; ri < i; ri++)
			{
				if (!HitTestStraightLine(PA[ri], PA[i], Hit, PathStart))
				{
					Nodes.Add(FNodePtr(new FGridNode(PA[ri], Nodes.Last())));
					//CheckLastToTarget();
					bFound = true;
					break;
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

bool FStraightenPathWorker::CreatePath(FNodePtr Node, FGridPath& Path)
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

bool FStraightenPathWorker::IsRunning()
{
	return bRunThread;
}
