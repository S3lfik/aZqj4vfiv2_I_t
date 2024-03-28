// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/IT_GameModeDefault.h"
#include "StaticData.h"

#include "Actors/IT_GameActorBase.h"

void FGrid::PrintGrid() const
{
	// TODO: don't forget this one. Implement and use, or delete.
}

FString FGridPoint::GetDebugString() const
{
	const FString DebugString(TEXT("Index:%s, X:%s, Y:%s, Actor:%s, {0}, {1}, {2}, {3}"));
	return FString::Format(*DebugString, {
		                       *FString::FromInt(GridCoords.GridX * GridCoords.GridY),
		                       *FString::FromInt(GridCoords.GridX), *FString::FromInt
		                       (GridCoords.GridY),
		                       *GetNameSafe(GameActor)
	                       });
}

void FGrid::Init(int32 InSizeX, int32 InSizeY)
{
	SizeX = InSizeX;
	SizeY = InSizeY;
	//GridArray.Init(FGridPoint, SizeX * SizeY);
	GridArray.SetNumZeroed(SizeX * SizeY);

	for (auto& GridPoint : GridArray)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *GridPoint.GetDebugString())
	}
}

const TArray<FGridPoint>& FGrid::GetGrid() const
{
	return GridArray;
}

FGridPoint FGrid::At(int32 Index) const
{
	checkf(Index < GridArray.Num(), TEXT("[FGrid::At] Argument Index out of bounds."));
	return GridArray[Index];
}

FGridPoint FGrid::At(FGridCoordinates Coordinates) const
{
	const int32 Index = Coordinates.GridX * Coordinates.GridY;
	checkf(Index < GridArray.Num(), TEXT("[FGrid::At] Coordinates out of bounds."));

	return GridArray[Index];
}

bool FGrid::FindRandomEmptyPointOnGrid(FGridPoint& OutGridPoint) const
{
	bool bResult = false;

	const FGridPoint RandomPoint = FindRandomPointOnGrid();

	// If the GridPoint is not empty
	if (RandomPoint.GameActor != nullptr)
	{
		// Go heavy, copy all empty slots into temp grid copy and get random there.
		TArray<FGridPoint> TempGrid = GridArray;
		for (auto& Point : GridArray)
		{
			if (Point.GameActor != nullptr)
			{
				TempGrid.Add(Point);
			}
		}
		if (TempGrid.Num() > 0)
		{
			OutGridPoint = TempGrid[FMath::RandRange(0, TempGrid.Num() - 1)];
			bResult = true;
		}
		else
		{
			// The whole grid is occupied? Most probably, some sort of a error
			OutGridPoint = FGridPoint();
		}
	}
	else
	{
		OutGridPoint = RandomPoint;
		bResult = true;
	}

	return bResult;
}

FGridPoint FGrid::FindRandomPointOnGrid() const
{
	checkf(GridArray.Num() > 0, TEXT("[FGrid::FindRandomPointOnGrid] Operation on an empty grid."));
	return GridArray[FMath::RandRange(0, GridArray.Num() - 1)];
}


void AIT_GameModeDefault::SpawnActors()
{
	checkf(NumberOfActorsPerTeam > 0,
	       TEXT("[AIT_GameModeDefault::SpawnActors] NumberOfActorsPerTeam is equal or less than zero."))

	if (auto* const World = GetWorld())
	{
		for (int32 Index = 0; Index < NumberOfActorsPerTeam; ++Index)
		{
			AIT_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AIT_GameActorBase>(
				AIT_GameActorBase::StaticClass(),
				FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (Index % 2)
			{
				SpawnedActor->SetTeam(ETeam::BlueTeam);
			}
			else
			{
				SpawnedActor->SetTeam(ETeam::RedTeam);
			}

			FGridPoint GridPoint;
			if (Grid.FindRandomEmptyPointOnGrid(GridPoint))
			{
				FTransform SpawnTransform{};
				SpawnTransform.SetLocation(GridToGlobal(GridPoint.GridCoords));

				GridPoint.GameActor = SpawnedActor;
				SpawnedActor->FinishSpawning(SpawnTransform);
			}
		}
	}
}

FVector AIT_GameModeDefault::GridToGlobal(const FGridCoordinates& InCoordinates) const
{
	return FVector{InCoordinates.GridX * GridCellSize, InCoordinates.GridY * GridCellSize, 0.f};
}
