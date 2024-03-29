// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticData.h"
#include "IT_GameActorBase.generated.h"

UCLASS()
class ILLUVIUMTASK_API AIT_GameActorBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIT_GameActorBase();

	void SetTeam(ETeam InTeam);
	ETeam GetTeam() const;

	void SetGridCoordinates(const FGridCoordinates& GridCoordinates);
	FGridCoordinates GetGridCoordinates() const;

	void SetAttackPower(float InNewAttackPower);
	float GetAttackPower() const;

	void SetHealthPoints(float InHealthPoints);
	float GetHealthPoints() const;

	bool IsAlive() const;

	void PlayHit();
	void PlayAttack();
	void HandleZeroHealth();
	void StartDestroy();

	void MoveActorInterp(const FVector& InNewLocation, float InInterpTime_ms);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	UMaterialInterface* RedTeamMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	UMaterialInterface* BlueTeamMaterial;

	ETeam Team;

	FGridCoordinates GridCoordinates;

	float AttackPower = 1.f;
	float Health = 1.f;
	bool bIsAlive = true;
};
