// Fill out your copyright notice in the Description page of Project Settings.


#include "GridActor.h"
#include "Kismet/GameplayStatics.h"
#include "GridSystemController.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"

// Sets default values
AGridActor::AGridActor()
{
	PrimaryActorTick.bCanEverTick = true;
	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifer"));
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
	Grid = nullptr;
	GridPosition = { 0,0,0};

}

// Called when the game starts or when spawned
void AGridActor::BeginPlay()
{
	Super::BeginPlay();
	Grid = Cast<AGridSystemController>(UGameplayStatics::GetActorOfClass(this, AGridSystemController::StaticClass()));
	SetToGrid();
	
	
}

void AGridActor::SetToGrid()
{
	if (!Grid)
		return;
	if (UGridFunctionLib::IsWorldLocationOnGrid(GetActorLocation()))
		return;
	GridPosition = UGridFunctionLib::ConvertWorldToGrid(GetActorLocation());
	Grid->MoveGridPosition(this,GridPosition);
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
	SetActorLocation(NewLoc);
}


bool AGridActor::MoveOnGrid_Implementation(FGridVector Pos)
{

	Grid->MoveGridPosition(this, Pos);
	GridPosition = Pos;
	FVector NewLoc = UGridFunctionLib::ConvertGridToWorld(GridPosition);
	SetActorLocation(NewLoc);
	return true;

	return false;
}

bool AGridActor::AddGridPos(FGridVector Pos)
{
	FGridVector NewPos = GetGridPosition() + Pos;
	return MoveOnGrid(NewPos);
}

FGridVector AGridActor::GetGridPosition()
{
	return GridPosition;

}

// Called every frame
void AGridActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

