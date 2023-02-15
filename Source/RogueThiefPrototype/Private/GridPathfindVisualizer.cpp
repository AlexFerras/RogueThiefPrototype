// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSystemController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GridPathfindVisualizer.h"


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
			StreightenPath(Path);
			PrintPath(Path);
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
	TArray<FGridVector> AdjustValues;
	AdjustValues.Add({ 0,1,0 });
	AdjustValues.Add({ 0,0,1 });
	AdjustValues.Add({ 0,-1,0 });
	AdjustValues.Add({ 0,0,-1 });

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

void AGridPathfindVisualizer::StreightenPath(FGridPath& Path)
{
	
	TArray<FGridVector> PA = Path.ToArray();
	Algo::Reverse(PA);
	IsStraightLineToPos(PA[0], PA.Last());

	//TArray<FNodePtr> Nodes;
	//Nodes.Add(FNodePtr(new FGridNode(PA[0], nullptr)));

	//for (int32 i = 1; i < PA.Num(); i++)
	//{
	//	if (IsStraightLineToPos(Nodes.Last()->Coord, PA[i]))
	//	{
	//		continue;
	//	}
	//	else
	//		Nodes.Add(FNodePtr(new FGridNode(PA[i - 1], Nodes.Last())));
	//}
	//CreatePath(Nodes.Last(), Path);
}

bool AGridPathfindVisualizer::IsStraightLineToPos(const FGridVector& PosA, const FGridVector& PosB)
{
	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(PosA.ToWorld(), PosB.ToWorld());
	FVector Right = FRotator{ 0, 90, 0 }.RotateVector(Direction) * 45;
	FVector Left = FRotator{ 0, -90, 0 }.RotateVector(Direction) * 45;


	FGridVector Previous = PosA;
	FVector TestPos = PosA.ToWorld() + Direction * 50;
	auto GridTestPos = FGridVector::FromFVector(TestPos);

	TFunction<void()> Lambda = [&]()
	{
		while (FGridVector::FromFVector(TestPos) == Previous)
		{
			TestPos += Direction * 50;
			GridTestPos = FGridVector::FromFVector(TestPos);
		}

	};

	Lambda();

	UE_LOG(LogTemp, Warning, TEXT("Start TestPos: %s"), *TestPos.ToString());
	
	
	while (FGridVector::FromFVector(TestPos) != PosB)
	{

		for (const auto& c : FGridVector::DiagnalConnectors(Previous, GridTestPos))
		{
			PrintStraight(c);
		}

		if (Positions.Contains(GridTestPos)
			&& (Grid->ArePositionsConnected(Previous, GridTestPos)/* || Grid->IsDiagnalConnected(Previous, GridTestPos)*/)
			)
		{
			Previous = GridTestPos;
			Lambda();
			PrintStraight(GridTestPos);

		}
		else
		{
			return false;
		}
	}
	return true;


}

void AGridPathfindVisualizer::ReleaseTargetNeighbors()
{
	TArray<FGridVector> AdjustValues;
	AdjustValues.Add({ 0,1,0 });
	AdjustValues.Add({ 0,0,1 });
	AdjustValues.Add({ 0,-1,0 });
	AdjustValues.Add({ 0,0,-1 });

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
	return node->Coord.DistanceTo(Target) + node->Cost();
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

void AGridPathfindVisualizer::Init()
{
	Positions = Grid->GetUnblockedSquarePositionsInRange(Start, FGridVector{ 1, Cost, Cost });
	CurrentNode = TSharedPtr<FGridNode>(new FGridNode(Start, nullptr));
	OpenList.Add(CurrentNode);
	Pathfind();

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




