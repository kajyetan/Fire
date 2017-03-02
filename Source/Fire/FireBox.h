// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/BoxComponent.h"
#include "FireBox.generated.h"

/**
*	WHEN A FIRE STARTS TO BURN
*/

//For UE4 Profiler ~ Stat Group
DECLARE_STATS_GROUP(TEXT("FireBox"), STATGROUP_FireBox, STATCAT_Advanced);


UCLASS()
class FIRE_API UFireBox : public UBoxComponent
{
	GENERATED_BODY()

private:
	// How much health before we catch fire
	int health;

	// How much remaning fuel
	int fuel;

	// Consumtion fuel
	int consumeFuel;

	// Are we deactivated?
	bool deactivated;

	// The particle emitter
	UParticleSystemComponent * emitter;

	// Whether we are currently on fire
	bool burning;

	// Has this been consumed?
	bool consumed;

	// Maximum size to consume
	float MAXIMUM_SIZE = 1000000.0f;
	int START_HEALTH = 300;
	int START_FUEL = 1000;
	int CONSUMPTION_FUEL = START_FUEL / 2;

	// Decal location
	FVector decalLocation;
	FVector decalDirection;
	UMaterial * decalMaterial;

	float timeSinceUpdate;

	// Emitter location
	FVector emitterLocation;

	float TimeBetweenUpdates;

public:

	UFireBox();

	void SetTimeBetweenUpdates(float time);

	// Getters and setters baby
	int GetHealth();

	int GetFuel();

	UParticleSystemComponent * GetEmitter();

	bool IsBurning();

	void SetHealth(int value);

	void SetFuel(int value);

	void SetEmitter(UParticleSystemComponent * target);

	bool IsConsumed();

	void SetConsumed(bool value);

	void SetDecalMaterial(UMaterial * decal);

	// Relative to centre of ze box in world orientation
	void SetDecalLocation(FVector location);
	void SetDecalDirection(FVector direction);

	void SetEmitterLocation(FVector location);

	// Tick Override
	virtual void TickComponent
	(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction * ThisTickFunction
	) override;

	// Notifications
	void Damage(int damage);   // Take damage -- take alot to instantly light


							   // Remove this from the world
	void Remove();

	void SetConsumptionFuel(int value);




};
