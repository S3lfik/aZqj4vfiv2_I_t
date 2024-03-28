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

	void PlayHit();
	void PlayAttack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp; 
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	ETeam Team;
};
