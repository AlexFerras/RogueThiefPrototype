// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GridPathfindVisualizer.h"
#include "DrawDebugHelpers.h"
#include "GridAffector.h"


#define ADJUSTVALUES { {0, 1, 0}, {0, 0, 1}, {0, -1, 0}, {0, 0, -1}, {0, 1, 1}, {0, 1, -1}, {0, -1, 1}, {0, -1, -1} };

// Sets default values
AGridPathfindVisualizer::AGridPathfindVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;
	CurrentNode = nullptr;
	TargetNode = nullptr;
	Start = FGridVector(0,0,0);
	Target = FGridVector(0,0,0);
	Cost = 0;
	Interval = .5f;

}

// Called when the game starts or when spawned
void AGridPathfindVisualizer::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AGridSystemController>(UGameplayStatics::GetActorOfClass(this, AGridSystemController::StaticClass()));
	FTimerHandle Handle;
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AGridPathfindVisualizer::Init);
	GetWorldTimerManager().SetTimer(Handle, Delegate, 0.5f, false);
}

// Called every frame
void AGridPathfindVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGridPathfindVisualizer::Pathfind()
{
	if (CurrentNode == nullptr)
	{
		ReturnPaths(Paths);
		return;
	}

	if (CurrentNode->Coord == Target)
	{
		FGridPath Path;
		if (CreatePath(CurrentNode, Path))
		{
			PrintPath(Path);

			StraightenPath(Path);
			PrintStraightPath(Path);
			return;
			Paths.Add(Path);
			PrintPath(Path);
			OpenList.Remove(CurrentNode);
		}
	}

	if (OpenList.Num() > 0 || OnlyTargetRemains())
	{
		FTimerHandle Handle;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AGridPathfindVisualizer::Pathfind);
		WorkOnNode();
		GetWorldTimerManager().SetTimer(Handle, Delegate, Interval, false);
	}
	else
		ReturnPaths(Paths);
}

void AGridPathfindVisualizer::AddNeighborsToArray(TSharedPtr<FGridNode> Node)
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
				if (NewNode->Cost() < Cost)
					OpenList.Add(NewNode);
			}
		}
	}
	OpenList.Remove(Node);
	CloseList.Add(Node);

	AddOtherConnected();
}

bool AGridPathfindVisualizer::IsTarget(TSharedPtr<FGridNode> Node)
{
	return false;
}

bool AGridPathfindVisualizer::CreatePath(const FNodePtr Node, FGridPath& Path)
{
	TArray<FGridVector> ResultPositions;
	ResultPositions.Add(Target);
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



bool AGridPathfindVisualizer::GetNodeOnPos(const FGridVector& Pos, FNodePtr& node)
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

	if (node)
		return true;
	return false;

}

bool AGridPathfindVisualizer::OnlyTargetRemains()
{
	TArray<FGridVector> AdjustValues = ADJUSTVALUES;

	int32 index = 0;
	for (auto& Value : AdjustValues)
	{
		FNodePtr Node;
		if (GetNodeOnPos(Target + Value, Node))
		{
			if (OpenList.Contains(Node))
				index++;
		}
	}
	return index == 4;

}

void AGridPathfindVisualizer::AddOtherConnected()
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

void AGridPathfindVisualizer::StraightenPath(FGridPath& Path)
{
	
	TArray<FGridVector> PA = Path.ToArray();

	auto PathStart = PA[0];
	PA.RemoveAt(0);
	auto PathTarget = PA.Last();
	FGridVector Hit;


	TArray<FNodePtr> Nodes;
	Nodes.Add(FNodePtr(new FGridNode(PathStart, nullptr)));

	if (!HitTestStraightLine(PathStart, PathTarget, Hit))
	{
		Nodes.Add(FNodePtr(new FGridNode(PathTarget, Nodes.Last())));
		CreatePath(Nodes.Last(), Path);
		return;
	}

	if (HitTestStraightLine(PathStart, PathTarget, Hit))
		UE_LOG(LogTemp, Warning, TEXT("No straight line"))
	else
		UE_LOG(LogTemp, Warning, TEXT("Straight line"))

	
	for (int32 i = 0; i < PA.Num(); i++)
	{
		if (!HitTestStraightLine(Nodes.Last()->Coord, PA[i], Hit))
		{
			if (PA[i] == PathTarget)
				Nodes.Add(FNodePtr(new FGridNode(PA[i], Nodes.Last())));
		}
		else
		{
			if (PA[i].Z != (PA.IsValidIndex(i - 1) ? PA[i - 1].Z : PA[i + 1].Z))
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
			for (int32 ri = i-1; ri > PA.Find(Nodes.Last()->Coord); ri--)
			{
				if (HitTestStraightLine(PA[ri], PA[i], Hit))
				{
					Nodes.Add(FNodePtr(new FGridNode(PA[ri+1], Nodes.Last())));
					bFound = true;
				}
			}
			if (!bFound)
				Nodes.Add(FNodePtr(new FGridNode(PA.IsValidIndex(i-1) ? PA[i-1] : PA[i], Nodes.Last())));
			
			//Nodes.Add(FNodePtr(new FGridNode(PA[i - 1], Nodes.Last())));
			//Nodes.Add(FNodePtr(new FGridNode(PA[i], Nodes.Last())));
		}

	}
	if (!Grid->NodesContain(Nodes, PathTarget))
	{
		Nodes.Add(FNodePtr(new FGridNode(PathTarget, Nodes.Last())));
	}


	CreatePath(Nodes.Last(), Path);
	return;
}

bool AGridPathfindVisualizer::HitTestStraightLine(const FGridVector& PosA, const FGridVector& PosB, FGridVector& Hit)
{
	if (PosA.Z != PosB.Z)
		return true;


	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(PosA.ToWorld(), PosB.ToWorld());
	DrawDebugLine(GetWorld(), PosA.ToWorld()+FVector{0.f,0.f,30.f}, PosB.ToWorld()+FVector { 0.f, 0.f, 30.f }, FColor::Red, false, 20.f);


	FVector Right = FRotator{ 0, 90, 0 }.RotateVector(Direction) * 45.f;
	FVector Left = FRotator{ 0, -90, 0 }.RotateVector(Direction) * 45.f;


	FGridVector Previous = PosA;
	FVector TestPos = PosA.ToWorld();
	auto GridTestPos = FGridVector::FromFVector(TestPos);

	Hit = FGridVector();

	TFunction<void()> Lambda = [&]()
	{
		TestPos += Direction * 50;
		if (FGridVector::FromFVector(TestPos) != GridTestPos)
			GridTestPos = FGridVector::FromFVector(TestPos);
		DrawDebugLine(GetWorld(), TestPos + FVector{ 0.f,0.f,30.f }, TestPos + Left + FVector{ 0.f, 0.f, 30.f }, FColor::Red, false, 20.f);
		DrawDebugLine(GetWorld(), TestPos + FVector{ 0.f,0.f,30.f }, TestPos + Right + FVector{ 0.f, 0.f, 30.f }, FColor::Red, false, 20.f);
		PrintStraight(GridTestPos);
		PrintStraight(FGridVector::FromFVector(TestPos + Left));
		PrintStraight(FGridVector::FromFVector(TestPos + Right));

	};
	while (GridTestPos == Previous)
		Lambda();

	UE_LOG(LogTemp, Warning, TEXT("Start TestPos: %s"), *TestPos.ToString());
	
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
			FVector HitDebug = Hit.ToWorld() + Direction * 50;
			DrawDebugBox(GetWorld(), Hit, FVector{ 25.f,25.f,25.f }, FColor::Green, false, 20.f);
			return true;
		}
	}
	return false;


}

void AGridPathfindVisualizer::ReleaseTargetNeighbors()
{
	TArray<FGridVector> AdjustValues = ADJUSTVALUES

	for (auto& Value : AdjustValues)
	{
		FNodePtr Node;
		if (GetNodeOnPos(Target + Value, Node))
		{
			OpenList.Remove(Node);
			CloseList.Remove(Node);
		}
	}

}

TArray<FNodePtr> AGridPathfindVisualizer::GetNodeNeighbors(FNodePtr node)
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

int32 AGridPathfindVisualizer::GetNodeCost(FNodePtr node)
{
	int32 dx = FMath::Abs(Target.X - node->Coord.X);
	int32 dy = FMath::Abs(Target.Y - node->Coord.Y);

	int32 H;

	if (dx > dy)
		H = 1 * dy + 2 * (dx - dy);
	else 
	{
		H = 1 * dx + 2 * (dy - dx);
	}


	UE_LOG(LogTemp, Warning, TEXT("Distance Cost: %i"), H);



	return /*node->Coord.DiagonalDistanceTo(Target)*/ H + node->Cost();
		//FMath::Abs(node->Coord.X - Target.X) + FMath::Abs(node->Coord.Y - Target.Y) /* node.Cost()*/;
}

void AGridPathfindVisualizer::WorkOnNode()
{
	if (CurrentNode->Coord != Target)
		AddNeighborsToArray(CurrentNode);
	else
	{
		FGridPath Path;
		if (CreatePath(CurrentNode, Path))
			return;
	}
	int32 CurrentCost = 100;
	int32 MaxIndex = 0;

	FVector ParentLoc = (CurrentNode->P.IsValid()) ? CurrentNode->P->Coord.ToWorld() : FVector();
	PrintCurrentNode(CurrentNode->Coord, UKismetMathLibrary::GetDirectionUnitVector(CurrentNode->Coord.ToWorld(), ParentLoc));

	//for (int32 i = 0; i < OpenList.Num(); i++)
	//{
	//	int32 NewCost = GetNodeCost(OpenList[i]);
	//	if (NewCost <= CurrentCost)
	//	{
	//		CurrentCost = NewCost;
	//		MaxIndex = i;
	//	}
	//}
	if (OpenList.IsValidIndex(0))
		CurrentNode = OpenList[0];
	else
		CurrentNode = nullptr;
}

void AGridPathfindVisualizer::Init()
{
	Positions = Grid->GetUnblockedSquarePositionsInRange(Start, FGridVector{ 1, Cost, Cost });
	CurrentNode = TSharedPtr<FGridNode>(new FGridNode(Start, nullptr));
	OpenList.Add(CurrentNode);
	//Pathfind();

}

FGridPath AGridPathfindVisualizer::GetShortestPath()
{
	int32 LowestCost = 100;
	int32 LowestIndex = 0;


	for (int32 i = 0; i < Paths.Num(); i++)
	{
		if (Paths[i].Cost() < LowestCost)
		{
			LowestCost = Paths[i].Cost();
			LowestIndex = i;
		}
	}
	return Paths[LowestIndex];
}




