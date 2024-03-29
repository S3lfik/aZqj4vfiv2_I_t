// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/IT_GameModeDefault.h"
#include "Actors/IT_GameActorBase.h"
#include "IlluviumTask/IlluviumTask.h"

void FGrid::PrintGrid() const
{
	for (auto& Point : GridArray)
	{
		UE_LOG(LogTask, Display, TEXT("%s"), *Point.GetDebugString());
	}
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

	for (int Rows = 0; Rows < SizeY; ++Rows)
	{
		for (int Cols = 0; Cols < SizeX; ++Cols)
		{
			FGridPoint& GridPoint = GridArray[Cols + Rows * SizeY];
			GridPoint.GridCoords = FGridCoordinates{Cols, Rows};
			UE_LOG(LogTask, Display, TEXT("Created point: %s"), *GridPoint.GetDebugString());
		}
	}
	UE_LOG(LogTask, Display, TEXT("Number of Points: %s"), *FString::FromInt(GridArray.Num()));;
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

AIT_GameModeDefault::AIT_GameModeDefault(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	bStartPlayersAsSpectators = true;
	ActorClass = AIT_GameActorBase::StaticClass();
}

void AIT_GameModeDefault::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Grid.Init(GridSizeX, GridSizeY);
	//Grid.PrintGrid();

	SpawnActors();

	StartSimulation();
}

void AIT_GameModeDefault::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bSimulationOngoing)
	{
		TimeStepAccumulator += DeltaSeconds;

		if (TimeStepAccumulator >= SimulationTimeStep_ms)
		{
			MakeSimulationTurn();

			TimeStepAccumulator = 0.f;
		}
	}
}


void AIT_GameModeDefault::SpawnActors()
{
	checkf(NumberOfActorsPerTeam > 0,
	       TEXT("[AIT_GameModeDefault::SpawnActors] NumberOfActorsPerTeam is equal or less than zero."))

	if (auto* const World = GetWorld())
	{
		for (int32 Index = 0; Index < NumberOfActorsPerTeam * NumberOfTeams; ++Index)
		{
			AIT_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AIT_GameActorBase>(
				ActorClass,
				FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (Index % 2)
			{
				SpawnedActor->SetTeam(ETeam::BlueTeam);
			}
			else
			{
				SpawnedActor->SetTeam(ETeam::RedTeam);
			}
			SpawnedActor->SetAttackPower(FMath::RandRange(AttackPowerMin, AttackPowerMax));
			SpawnedActor->SetHealthPoints(FMath::RandRange(HealthPointsMin, HealthPointsMax));

			FGridPoint GridPoint;
			if (Grid.FindRandomEmptyPointOnGrid(GridPoint))
			{
				FTransform SpawnTransform{};
				SpawnTransform.SetLocation(GridToGlobal(GridPoint.GridCoords));

				GridPoint.GameActor = SpawnedActor;
				SpawnedActor->SetGridCoordinates(GridPoint.GridCoords);
				SpawnedActor->FinishSpawning(SpawnTransform);
				GameActors.Add(SpawnedActor);
			}
			UE_LOG(LogTask, Display,
			       TEXT("[AIT_GameModeDefault::SpawnActors] Created Actor: Team:%s, Health:%s, Attack:%s"),
			       SpawnedActor->GetTeam() == ETeam::BlueTeam ? *FString("Blue") : *FString("Red"),
			       *FString::SanitizeFloat(SpawnedActor->GetHealthPoints()),
			       *FString::SanitizeFloat(SpawnedActor->GetAttackPower()));
		}
	}
}

void AIT_GameModeDefault::StartSimulation()
{
	bSimulationOngoing = true;
}

void AIT_GameModeDefault::MakeSimulationTurn()
{
	// If there is just one, or even no Actors - cease the simulation
	if (GameActors.Num() <= 1)
	{
		UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::MakeSimulationTurn] Simulation is over."))
		bSimulationOngoing = false;
		return;
	}

	// for each actor:
	for (auto* Actor : GameActors)
	{
		// find closest
		float Distance = 0.f;
		if (auto* TargetActor = FindClosestActor(Actor, Distance))
		{
			if (Distance - 1.f < UE_KINDA_SMALL_NUMBER)
			{
				ActorAttack(TargetActor, Actor);
			}
			else
			{
				ActorMoveTowards(TargetActor, Actor);
			}
		}
		else
		{
			UE_LOG(LogTask, Warning,
			       TEXT("[AIT_GameModeDefault::MakeSimulationTurn] Failed to find the closest actor."));
		}
	}

	for (auto* Actor : KilledGameActors)
	{
		GameActors.Remove(Actor);
		Actor->StartDestroy();
	}
}

AIT_GameActorBase* AIT_GameModeDefault::FindClosestActor(AIT_GameActorBase* InActor, float& OutDistance)
{
	AIT_GameActorBase* TargetActor = nullptr;

	if (InActor == nullptr)
	{
		UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::FindClosestActor] Use of null pointer."))
		return TargetActor;
	}

	float ClosestDist = -1.f;
	AIT_GameActorBase* ClosestTarget = nullptr;
	for (auto* Actor : GameActors)
	{
		if (Actor == InActor || !Actor->IsAlive())
		{
			continue;
		}

		if (InActor->GetTeam() != Actor->GetTeam())
		{
			FGridCoordinates Diff = InActor->GetGridCoordinates() - Actor->GetGridCoordinates();
			const float DistSqr = FGridCoordinates::DistSqr(InActor->GetGridCoordinates(), Actor->GetGridCoordinates());

			if ((ClosestDist < 0) || (DistSqr < ClosestDist))
			{
				ClosestDist = DistSqr;
				ClosestTarget = Actor;
			}

			// Maybe, this is a good idea to break as soon as we find an actor that is one square away from us.
			if (DistSqr <= 1.f)
			{
				break;
			}
		}
	}

	if (ClosestTarget != nullptr)
	{
		OutDistance = ClosestDist;
	}

	return ClosestTarget;
}

void AIT_GameModeDefault::ActorAttack(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InActionActor)
{
	UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::ActorAttack] Target: %s, Instigator %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InActionActor));

	if (!InTargetActor || !InActionActor)
	{
		UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::ActorAttack] One of the actors is nullptr."));
		return;
	}
	InTargetActor->SetHealthPoints(InTargetActor->GetHealthPoints() - InActionActor->GetAttackPower());
	if (InTargetActor->GetHealthPoints() <= 0.f)
	{
		InTargetActor->HandleZeroHealth();
		KilledGameActors.Add(InTargetActor);
	}
}

void AIT_GameModeDefault::ActorMoveTowards(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InActionActor)
{
	UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::ActorMove] Target: %s, ActionActor %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InActionActor));

	const FGridCoordinates Diff = InTargetActor->GetGridCoordinates() - InActionActor->GetGridCoordinates();
	// To get rid of signs, we better operate with squared diff when we need to compare them
	const FGridCoordinates DiffSqr(FMath::Square(Diff.GridX), FMath::Square(Diff.GridY));
	FGridCoordinates NextMove;

	// Use double negation to get an axis unit direction from the axis diff
	// For an equal X and Y diff go for random axis direction
	if (DiffSqr.GridX == DiffSqr.GridY)
	{
		FMath::RandBool() ? NextMove.GridX = 1 * FMath::Sign(Diff.GridX) : NextMove.GridY = 1 * FMath::Sign(Diff.GridY);
	}
	else if (DiffSqr.GridX > DiffSqr.GridY)
	{
		NextMove.GridX = 1 * FMath::Sign(Diff.GridX);
	}
	else
	{
		NextMove.GridY = 1 * FMath::Sign(Diff.GridY);
	}

	UE_LOG(LogTask, Display,
	       TEXT(
		       "[AIT_GameModeDefault::ActorMove] Target: %s is at %s,%s, ActionActor %s is at %s,%s. Move Difference: %s,%s"
	       ),
	       *GetNameSafe(InTargetActor), *FString::FromInt(InTargetActor->GetGridCoordinates().GridX),
	       *FString::FromInt(InTargetActor->GetGridCoordinates().GridY),
	       *GetNameSafe(InActionActor), *FString::FromInt(InActionActor->GetGridCoordinates().GridX),
	       *FString::FromInt(InActionActor->GetGridCoordinates().GridY),
	       *FString::FromInt(NextMove.GridX), *FString::FromInt(NextMove.GridY));

	NextMove.GridX += InActionActor->GetGridCoordinates().GridX;
	NextMove.GridY += InActionActor->GetGridCoordinates().GridY;
	InActionActor->SetGridCoordinates(NextMove);


	// TODO: fix the lerp first. Then delete the SetActorLocation call.
	//InActionActor->MoveActorInterp(GridToGlobal(NextMove), SimulationTimeStep_ms);
	InActionActor->SetActorLocation(GridToGlobal(NextMove));
}

void AIT_GameModeDefault::HandleActorKilled(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor)
{
	UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::HandleActorKilled] %s is killed by %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InInstigatorActor));

	// Remove actor form the array
	// Clear actor's grid point
	// Trigger actor's BeginDeath 
	// what else?
}

FVector AIT_GameModeDefault::GridToGlobal(const FGridCoordinates& InCoordinates) const
{
	return FVector{InCoordinates.GridX * GridCellSize, InCoordinates.GridY * GridCellSize, 0.f};
}
