// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/IT_GameActorBase.h"

// Sets default values
AIT_GameActorBase::AIT_GameActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(StaticMeshComp);
}

// Called when the game starts or when spawned
void AIT_GameActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIT_GameActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AIT_GameActorBase::SetTeam(ETeam InTeam)
{
	Team = InTeam;
}

ETeam AIT_GameActorBase::GetTeam() const
{
	return Team;
}

