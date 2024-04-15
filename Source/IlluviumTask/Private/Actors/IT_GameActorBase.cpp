// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/IT_GameActorBase.h"

// Sets default values
AIT_GameActorBase::AIT_GameActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = SceneComponent;
	
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMeshComp->SetupAttachment(RootComponent);
}

void AIT_GameActorBase::HandleZeroHealth()
{
	Health = 0.f;
	bIsAlive = false;
}

void AIT_GameActorBase::StartDestroy()
{
	// TODO: spawn some emitter maybe?
	Destroy();
}

void AIT_GameActorBase::MoveActorInterp(const FVector& InNewLocation, float InInterpTime_ms)
{
	// TODO: make it lerp here..
	//SetActorLocation(FMath::Lerp(GetActorLocation(), InNewLocation, InInterpTime_ms));
}

void AIT_GameActorBase::SetTeam(ETeam InTeam)
{
	Team = InTeam;

	if(StaticMeshComp)
	{
		switch(Team)
		{
		case ETeam::BlueTeam:
			StaticMeshComp->SetMaterial(0, BlueTeamMaterial);
			break;
		case ETeam::RedTeam:
			StaticMeshComp->SetMaterial(0, RedTeamMaterial);
			break;
		};
	}
}

ETeam AIT_GameActorBase::GetTeam() const
{
	return Team;
}

FIntPoint AIT_GameActorBase::GetGridCoordinates() const
{
	return GridCoordinates;
}

void AIT_GameActorBase::SetAttackPower(float InNewAttackPower)
{
	AttackPower = InNewAttackPower;
}

float AIT_GameActorBase::GetAttackPower() const
{
	return AttackPower;
}

void AIT_GameActorBase::SetAttackRange(int32 InNewAttackRange)
{
	AttackRange = InNewAttackRange;
}

int32 AIT_GameActorBase::GetAttackRange() const
{
	return AttackRange;
}

void AIT_GameActorBase::SetHealthPoints(float InHealthPoints)
{
	Health = InHealthPoints;
}

float AIT_GameActorBase::GetHealthPoints() const
{
	return Health;
}

bool AIT_GameActorBase::IsAlive() const
{
	return bIsAlive;
}

void AIT_GameActorBase::PlayHit()
{
	// TODO: Make it blink
}

void AIT_GameActorBase::PlayAttack()
{
	// TODO: Make it blink
}

void AIT_GameActorBase::SetGridCoordinates(const FIntPoint& InGridCoordinates)
{
	GridCoordinates = InGridCoordinates;
}
