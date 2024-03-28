// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IT_GameModeDefault.generated.h"

/**
 * Grid oriented coordinates
 */
struct FGridCoordinates
{
	static const FGridCoordinates Empty()
	{
		return FGridCoordinates{-1, -1};
	};

	int32 GridX = 0;
	int32 GridY = 0;

	bool operator==(const FGridCoordinates& InCoordinates) const
	{
		return (GridX == InCoordinates.GridX) && (GridY == InCoordinates.GridY);
	}
};

/**
 * The struct to represent a grid element.
 */
USTRUCT()
struct FGridPoint
{
	GENERATED_BODY()

	FGridCoordinates GridCoords;
	float Weight = 0.f;

	TObjectPtr<class AIT_GameActorBase> GameActor = nullptr;

	FString GetDebugString() const;
};

/**
 * The struct (but rather a class already) to represent the grid.
 */
USTRUCT()
struct FGrid
{
	GENERATED_BODY()
	;

	/**
	 * Init Grid
	 * @param InSizeX Size of the X side of the Grid 
	 * @param InSizeY Size of the Y side of the Grid
	 */
	void Init(int32 InSizeX, int32 InSizeY);

	void PrintGrid() const;

	/**
	 * A getter for the Grid Array
	 * @return GridArray
	 */
	const TArray<FGridPoint>& GetGrid() const;

	/**
	 * Get Grid element at index
	 * @param Index Element index
	 * @return Element at index
	 */
	FGridPoint At(int32 Index) const;

	/**
	 * Get Grid element by coordinates
	 * @param Coordinates Element coordinates
	 * @return Element at coordinates
	 */
	FGridPoint At(FGridCoordinates Coordinates) const;

	/**
	* @return Returns a random position on Grid that is not yet occupied
	*/
	bool FindRandomEmptyPointOnGrid(FGridPoint& OutGridPoint) const;

	/**
	* @return Returns a random position on Grid
	*/
	FGridPoint FindRandomPointOnGrid() const;

private:
	TArray<FGridPoint> GridArray;
	int32 SizeX = 0;
	int32 SizeY = 0;
};

/**
 * 
 */
UCLASS()
class ILLUVIUMTASK_API AIT_GameModeDefault : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// The X size of grid to generate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 GridSizeX = 100;

	// The Y size of grid to generate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 GridSizeY = 100;

	// The size of grid cells for scaling to the world coordinates
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	float GridCellSize = 50.f;

	// The number of actor each team will have
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 NumberOfActorsPerTeam = 1;

private:
	/**
	 * Simple Actor spawning method
	 */
	void SpawnActors();

	/**
	 * A conversion method to receive Global coordinates from the Grid Coordinates
	 * @param GridX Grid X coordinate
	 * @param GridY Grid Y coordinate
	 * @return Global Coordinates
	 */
	FVector GridToGlobal(const FGridCoordinates& GridCoordinates ) const;

	// An array of Game Actors
	TArray<class IT_GameActorBase*> GameActors;

	// The grid
	FGrid Grid;
};
