// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StaticData.h"
#include "IT_GameModeDefault.generated.h"

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

public:
	AIT_GameModeDefault(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;
	
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
	 * Or rather a Step, not a turn.. A simulation iteration functional unit.
	 */
	void MakeSimulationTurn();

	/**
	 * Find the closest opponent
	 * @param InActor An actor to look opponents for
	 * @param OutDistance Distance to the found opponent 
	 * @return Pointer to the found opponent
	 */
	AIT_GameActorBase* FindClosestActor(AIT_GameActorBase* InActor, float& OutDistance);
	
	void ActorAttack(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);

	void ActorMoveTowards(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);

	void HandleActorKilled(AIT_GameActorBase* InTargetActor, AIT_GameActorBase* InInstigatorActor);
	
	/**
	 * A conversion method to receive Global coordinates from the Grid Coordinates
	 * @param GridX Grid X coordinate
	 * @param GridY Grid Y coordinate
	 * @return Global Coordinates
	 */
	FVector GridToGlobal(const FGridCoordinates& GridCoordinates ) const;

	// TODO: Move it to GameState.
	// An array of Game Actors.
	TArray<class AIT_GameActorBase*> GameActors;

	// An array of Game Actors pending to be destroyed.
	TArray<class AIT_GameActorBase*> KilledGameActors;

	// The grid
	FGrid Grid;

	// A bool flag to check if simulation is active
	bool bSimulationOngoing = false;

	// A counter to accumulate delta time from ticks to simulate TimeSteps
	float TimeStepAccumulator = 0.f;
};
