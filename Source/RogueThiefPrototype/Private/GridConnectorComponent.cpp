// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSystemController.h"
#include "GridConnectorComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UGridConnectorComponent::UGridConnectorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// ...
}


// Called when the game starts
void UGridConnectorComponent::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AGridSystemController>(UGameplayStatics::GetActorOfClass(this, AGridSystemController::StaticClass()));
	SetToGrid();

	// ...
	
}


// Called every frame
void UGridConnectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGridConnectorComponent::GetGridPosition(FGridVector& Pos1, FGridVector& Pos2)
{
	Pos1 = GridPosition1;
	Pos2 = GridPosition2;
}


FVector UGridConnectorComponent::GetOwnerLocation()
{
	return GetOwner()->GetActorLocation();
}

void UGridConnectorComponent::SetToGrid()
{
	if (!Grid)
		return;
	
	AActor* Owner = GetOwner();
	GridPosition1 = UGridFunctionLib::ConvertWorldToGrid(GetOwnerLocation());
	Grid->SetGridPosition(Owner, GridPosition1);
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition1);

	TArray<FVector> Directions;
	Directions.Add({ 1.f,0.f,0.f });
	Directions.Add({ 0.f, 1.f, 0.f });
	Directions.Add({ -1.f, 0.f, 0.f });
	Directions.Add({ 0.f,-1.f,0.f });

	for (int32 i = 0; i < 4; i++)
	{
		if (FVector::DotProduct(Owner->GetActorForwardVector(), Directions[i]) > 0.5f)
		{
			Owner->SetActorRotation(Directions[i].Rotation());
			GridPosition2 = GridPosition1 + Directions[i] + FGridVector{1, 0, 0};
			Grid->SetGridPosition(Owner, GridPosition2);
			
		}
	}
	GetOwner()->SetActorLocation(NewLoc);
}

void UGridConnectorComponent::SetGridPosition(FGridVector Pos)
{
	//Grid->MoveGridPosition(GetOwner(), Pos);
	//GridPosition = Pos;
	//FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
}

FGridVector UGridConnectorComponent::GetConnectedPos(const FGridVector& Pos)
{
	if (GridPosition1 == Pos)
		return GridPosition2;
	else if (GridPosition2 == Pos)
		return GridPosition1;
	else
		return FGridVector();
}

