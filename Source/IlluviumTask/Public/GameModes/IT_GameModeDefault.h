// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StaticData.h"
#include "Grid/IT_Grid.h"
#include "IT_GameModeDefault.generated.h"


class AIT_GridTestActor;
/**
 * 
 */
UCLASS()
class ILLUVIUMTASK_API AIT_GameModeDefault : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIT_GameModeDefault(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void BeginPlay() override;

	void SpawnActorAt(TSubclassOf<AIT_GameActorBase> ActorClass, FIntPoint GridPoint, ETeam Team);

	UFUNCTION(BlueprintCallable)
	void K2_StartSimulation();
	
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

	// Generally, we don't need this variable, but for now I simply use it instead of magic numbers for spawning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	int32 NumberOfTeams = 2;

	// The subclass to be used for the simulation. It's just a single class at the moment tho
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TSubclassOf<AIT_GameActorBase> ActorClass;

	// The subclass to be used for the simulation. It's just a single class at the moment tho
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	TSubclassOf<AIT_GridTestActor> GridActorDummyClass;
	
	// Minimum Attack power that will be used for random AttackPower setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Attack")
	float AttackPowerMin = 1.f;
	// Maximum Attack power that will be used for random AttackPower setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Attack")
	float AttackPowerMax = 10.f;
	// Minimum Health that will be used for random Health setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Health")
	float HealthPointsMin = 1.f;
	// Maximum Health that will be used for random Health setup
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings|ActorsSetting|Health")
	float HealthPointsMax = 10.f;
	// TimeStep duration that will be used for simulation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameSettings")
	float SimulationTimeStep_ms = 0.1f;
	

private:
	/**
	 * Simple Actor spawning method
	 */
	void SpawnActors();

	/**
	 * Starts the simulation
	 */
	void StartSimulation();

	/**
	 * Ends the simulation
	 */
	void EndSimulation(/*EReason*/);
	void IsSimulationOver();

	/**
	 * Or rather a Step, not a turn.. A simulation iteration functional unit.
	 */
	void MakeSimulationTurn();

	/**
	 * Find the closest opponent
	 * @param InActor An actor to look opponents for
	 * @param OutDistanceSqr Square distance to the found opponent 
	 * @return Pointer to the found opponent
	 */
	AIT_GameActorBase* FindClosestActor(AIT_GameActorBase* InActor, int32& OutDistanceSqr);
	
	void ActorAttack(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);

	FIntPoint GetNextMoveLocation(AIT_GameActorBase* InActionActor, AIT_GameActorBase* InTargetActor, const FGrid& InGrid);
	void ActorMoveTowards(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);

	void HandleActorKilled(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);
	
	/**
	 * A conversion method to receive Global coordinates from the Grid Coordinates
	 * @param GridX Grid X coordinate
	 * @param GridY Grid Y coordinate
	 * @return Global Coordinates
	 */
	FVector GridToGlobal(const FIntPoint& GridCoordinates ) const;

	/**
	 * Spawns info actors at each Grid cell
	 */
	void TestGrid();

	// TODO: Move it to GameState.
	// An array of Game Actors.
	TArray<class AIT_GameActorBase*> GameActors;

	TMap<ETeam, int32> ActorsNumPerTeam;

	// An array of Game Actors pending to be destroyed.
	TArray<class AIT_GameActorBase*> KilledGameActors;

	// The grid
	FGrid Grid;

	// A bool flag to check if simulation is active
	bool bSimulationOngoing = false;

	// A counter to accumulate delta time from ticks to simulate TimeSteps
	float TimeStepAccumulator = 0.f;

	TPimplPtr<class IT_Pathfinder> Pathfinder;
};
