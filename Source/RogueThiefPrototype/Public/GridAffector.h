// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridFunctionLib.h"
#include "UObject/Interface.h"
#include "GridAffector.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UGridAffector : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ROGUETHIEFPROTOTYPE_API IGridAffector
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintImplementableEvent)
	bool IsBlock();

	UFUNCTION(BlueprintImplementableEvent)
	bool IsWall(FGridVector& A, FGridVector& B);

	UFUNCTION(BlueprintImplementableEvent)
	bool IsConnector(const FGridVector& Pos, FGridVector& B);

	UFUNCTION(BlueprintImplementableEvent)
	bool IsFloor();
};
