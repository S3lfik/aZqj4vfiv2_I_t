// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/IT_GameModeDefault.h"
#include "Actors/IT_GameActorBase.h"
#include "Grid/IT_GridTestActor.h"
#include "Grid/IT_Pathfinder.h"
#include "IlluviumTask/IlluviumTask.h"


AIT_GameModeDefault::AIT_GameModeDefault(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	bStartPlayersAsSpectators = true;
	ActorClass = AIT_GameActorBase::StaticClass();

	//Pathfinder = MakeUnique<IT_Pathfinder>();
	Pathfinder = MakePimpl<IT_Pathfinder>();
}

void AIT_GameModeDefault::TestGrid()
{
	for (auto& Point : Grid.GetGrid())
	{
		if (auto* const World = GetWorld())
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(GridToGlobal(Point.GridCoords));
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AIT_GridTestActor* GridActor = World->SpawnActor<AIT_GridTestActor>(
				GridActorDummyClass, SpawnTransform, Params);
			GridActor->DebugText.Append(FString::FromInt(Point.Index)).Append("\n").Append(Point.GridCoords.ToString());
		}
	}
}

void AIT_GameModeDefault::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Grid.Init(GridSizeX, GridSizeY, EGridType::Rectangular);

	if (Pathfinder.IsValid())
	{
		Pathfinder->InitGraph(Grid);
	}
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

void AIT_GameModeDefault::BeginPlay()
{
	Super::BeginPlay();

	SpawnActors();
	StartSimulation();
}

void AIT_GameModeDefault::SpawnActorAt(TSubclassOf<AIT_GameActorBase> InActorClass, FIntPoint InGridPoint, ETeam InTeam)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(LogTask, Warning, TEXT("[SpawnActorAt] World is nullptr."))
		return;
	}

	AIT_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AIT_GameActorBase>(
		ActorClass,
		FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	SpawnedActor->SetTeam(InTeam);

	SpawnedActor->SetAttackPower(FMath::RandRange(AttackPowerMin, AttackPowerMax));
	SpawnedActor->SetHealthPoints(FMath::RandRange(HealthPointsMin, HealthPointsMax));

	FGridPoint& GridPoint = Grid.At(InGridPoint);
	GridPoint.GameActor = SpawnedActor;

	FTransform SpawnTransform{};
	SpawnTransform.SetLocation(GridToGlobal(GridPoint.GridCoords));

	GridPoint.GameActor = SpawnedActor;
	SpawnedActor->GridPointIndex = Grid.At(GridPoint.Index).Index;
	SpawnedActor->SetGridCoordinates(InGridPoint);

	SpawnedActor->FinishSpawning(SpawnTransform);

	GameActors.Add(SpawnedActor);
}

void AIT_GameModeDefault::K2_StartSimulation()
{
	StartSimulation();
}

void AIT_GameModeDefault::SpawnActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::SpawnActors] World ptr is nullptr."));
		return;
	}

	TArray<FGridPoint> SpawnLocations;
	Grid.OnStartSpawningActors();
	// Spawn actors
	for (int32 Index = 0; Index < NumberOfActorsPerTeam * 2/*NumberOfTeams*/; ++Index)
	{
		// First create an actor and populate it with required gameplay information
		AIT_GameActorBase* SpawnedActor = World->SpawnActorDeferred<AIT_GameActorBase>(
			ActorClass,
			FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		SpawnedActor->SetTeam(Index % 2 ? ETeam::BlueTeam : ETeam::RedTeam);
		SpawnedActor->SetAttackPower(FMath::RandRange(AttackPowerMin, AttackPowerMax));
		SpawnedActor->SetHealthPoints(FMath::RandRange(HealthPointsMin, HealthPointsMax));

		// Next, pass it to the Grid, or GridManager, or GridGenerator to register it on the grid
		// if( bool bSuccessful = RegisterActor(SpawnedActor) )

		//re-make FindRandomEmptyPointOnGrid to return an Index, I guess. Get Point ref by that point then
		FGridPoint GridPoint;
		Grid.FindRandomEmptyPointOnGrid(GridPoint);
		FGridPoint& GridPointRef = Grid.At(GridPoint.Index);
		if (GridPointRef.GameActor)
		{
			UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::SpawnActors] Received an occupied grid point."));
		}
		// Do Grid related stuff here.
		GridPointRef.GameActor = SpawnedActor;
		SpawnedActor->GridPointIndex = GridPointRef.Index;
		SpawnedActor->SetGridCoordinates(GridPointRef.GridCoords);


		// --
		// Next, with an Actor registered on the Grid, it should know it's Grid coordinates.
		// Use them to finalize the spawn
		FTransform FinalTransform;
		FinalTransform.SetLocation(GridToGlobal(SpawnedActor->GetGridCoordinates()));
		SpawnedActor->FinishSpawning(FinalTransform);

		++(ActorsNumPerTeam.FindOrAdd(SpawnedActor->GetTeam()));
		GameActors.Add(SpawnedActor);
	}
	Grid.OnFinishSpawningActors();
}

void AIT_GameModeDefault::StartSimulation()
{
	bSimulationOngoing = true;
}

void AIT_GameModeDefault::EndSimulation()
{
	bSimulationOngoing = false;
	UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::EndSimulation] Simulation is over."))
}

void AIT_GameModeDefault::IsSimulationOver()
{
	for (auto& TeamActorsNum : ActorsNumPerTeam)
	{
		// Technically we check, if any of teams is the only one that is left on the board.
		if (TeamActorsNum.Value == GameActors.Num())
		{
			EndSimulation();
			break;
		}
	}
}

void AIT_GameModeDefault::MakeSimulationTurn()
{
	// If there is just one, or even no Actors - cease the simulation
	// TODO: remove or modify this condition into "CanStartSimultaionTurn" 
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
		int32 DistanceSqr = 0;
		if (auto* TargetActor = FindClosestActor(Actor, DistanceSqr))
		{
			if (DistanceSqr <= FMath::Square(Actor->GetAttackRange()))
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

	// Clean-up
	for (auto* Actor : KilledGameActors)
	{
		GameActors.Remove(Actor);
		Actor->StartDestroy();
	}
	KilledGameActors.Empty();

	// Check simulation end conditions
	IsSimulationOver();
}

AIT_GameActorBase* AIT_GameModeDefault::FindClosestActor(AIT_GameActorBase* InActor, int32& OutDistanceSqr)
{
	AIT_GameActorBase* TargetActor = nullptr;

	if (InActor == nullptr)
	{
		UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::FindClosestActor] Use of null pointer."))
		return TargetActor;
	}

	int32 ClosestDist = -1;
	AIT_GameActorBase* ClosestTarget = nullptr;
	// TODO: upon adding new actors, put them into separate arrays, and iterate through the opponents array here.
	for (auto* Actor : GameActors)
	{
		if (Actor == InActor || !Actor->IsAlive())
		{
			continue;
		}

		if (InActor->GetTeam() != Actor->GetTeam())
		{
			FIntPoint Diff = InActor->GetGridCoordinates() - Actor->GetGridCoordinates();
			const int32 DistSqr = FIntPoint(InActor->GetGridCoordinates() - Actor->GetGridCoordinates()).SizeSquared();

			if ((ClosestDist < 0) || (DistSqr < ClosestDist))
			{
				ClosestDist = DistSqr;
				ClosestTarget = Actor;
			}

			// Maybe, this is a good idea to break as soon as we find an actor that is one square away from us.
			if (DistSqr <= 1)
			{
				break;
			}
		}
	}

	if (ClosestTarget != nullptr)
	{
		OutDistanceSqr = ClosestDist;
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
		HandleActorKilled(InTargetActor, InActionActor);
	}
}

FIntPoint AIT_GameModeDefault::GetNextMoveLocation(AIT_GameActorBase* InActionActor, AIT_GameActorBase* InTargetActor,
                                                   const FGrid& InGrid)
{
	FIntPoint ResultPoint = FIntPoint::ZeroValue;

	if (InTargetActor != nullptr)
	{
		FGridPoint CurrentGridPoint = InGrid.At(InActionActor->GetGridCoordinates());
		const TArray<FGridPoint> NeighborPoints = Grid.GetNodeConnections(CurrentGridPoint);

		int32 LeastDistance = FIntPoint(InTargetActor->GetGridCoordinates() - InActionActor->GetGridCoordinates()).
			SizeSquared();

		auto IsCloserThanBefore = [&LeastDistance](const FIntPoint& Left, const FIntPoint& Right)
		{
			return FIntPoint(Left - Right).SizeSquared() < LeastDistance;
		};

		for (const auto& Point : NeighborPoints)
		{
			const auto PointCoordinates = Point.GridCoords;
			if (Point.GameActor == nullptr && IsCloserThanBefore(PointCoordinates, InTargetActor->GetGridCoordinates()))
			{
				LeastDistance = FIntPoint(PointCoordinates - InTargetActor->GetGridCoordinates()).SizeSquared();
				ResultPoint = PointCoordinates;
			}
		}
	}

	return ResultPoint;
}

void AIT_GameModeDefault::ActorMoveTowards(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InActionActor)
{
	UE_LOG(LogTask, Display, TEXT("[AIT_GameModeDefault::ActorMove] Target: %s, ActionActor %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InActionActor));

	if (InTargetActor == nullptr || InActionActor == nullptr)
	{
		UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::ActorMove] Nullptr is passed as an argument."));
		return;
	}

	FIntPoint NextMove = GetNextMoveLocation(InActionActor, InTargetActor, Grid);
	// Alternatively, use pathfinding, if the grid will have obstacles:
	//auto DummyPath = Pathfinder->FindPath(Path::FNode(InActionActor->GetGridCoordinates()),
	//                                      Path::FNode(InTargetActor->GetGridCoordinates()));
	//if(DummyPath.Num() > 0)
	//{
	//	NextMove = (*DummyPath.begin()).XY;
	//}
	

	if (NextMove == FIntPoint::ZeroValue)
	{
		UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::ActorMove] Failed to find a point closer"));
		return;
	}

	// Clear current point on grid, then assign new coordinates to the actor and assign the actor to the new grid point
	Grid.At(InActionActor->GetGridCoordinates()).GameActor = nullptr;
	InActionActor->SetGridCoordinates(NextMove);
	Grid.At(NextMove).GameActor = InActionActor;


	// TODO: fix the lerp first. Then delete the SetActorLocation call.
	//InActionActor->MoveActorInterp(GridToGlobal(NextMove), SimulationTimeStep_ms);
	InActionActor->SetActorLocation(GridToGlobal(NextMove));
}

void AIT_GameModeDefault::HandleActorKilled(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor)
{
	UE_LOG(LogTask, Warning, TEXT("[AIT_GameModeDefault::HandleActorKilled] %s is killed by %s."),
	       *GetNameSafe(InTargetActor), *GetNameSafe(InInstigatorActor));

	InTargetActor->HandleZeroHealth();
	Grid.At(InTargetActor->GetGridCoordinates()).GameActor = nullptr;
	KilledGameActors.Add(InTargetActor);
	--ActorsNumPerTeam.FindOrAdd(InTargetActor->GetTeam());
}

FVector AIT_GameModeDefault::GridToGlobal(const FIntPoint& InCoordinates) const
{
	return FVector{InCoordinates.X * GridCellSize, InCoordinates.Y * GridCellSize, 0.f};
}
