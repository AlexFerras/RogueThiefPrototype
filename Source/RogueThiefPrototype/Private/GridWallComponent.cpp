// Fill out your copyright notice in the Description page of Project Settings.

#include "GridSystemController.h"
#include "GridWallComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UGridWallComponent::UGridWallComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// ...
}


// Called when the game starts
void UGridWallComponent::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AGridSystemController>(UGameplayStatics::GetActorOfClass(this, AGridSystemController::StaticClass()));
	SetToGrid();

	// ...
	
}


// Called every frame
void UGridWallComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGridWallComponent::GetGridPosition(FGridVector& Pos1, FGridVector& Pos2)
{
	Pos1 = GridPosition1;
	Pos2 = GridPosition2;
}


FVector UGridWallComponent::GetOwnerLocation()
{
	return GetOwner()->GetActorLocation();
}

void UGridWallComponent::SetToGrid()
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
			GridPosition2 = GridPosition1 + Directions[i];
			Grid->SetGridPosition(Owner, GridPosition2);
			
		}
	}
	GetOwner()->SetActorLocation(NewLoc);
}

void UGridWallComponent::SetGridPosition(FGridVector Pos)
{
	//Grid->MoveGridPosition(GetOwner(), Pos);
	//GridPosition = Pos;
	//FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
}

