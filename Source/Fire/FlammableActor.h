// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/StaticMeshActor.h"
#include "ShaderController.h"
#include "FireBox.h"
#include "FlammableActor.generated.h"

/**
*
*/
UCLASS()
class FIRE_API AFlammableActor : public AStaticMeshActor
{
	GENERATED_BODY()

private:
	TArray<UFireBox *> boxes;

	void AFlammableActor::Subdivide(TArray<UFireBox *>& boxes);

	int numberOfBoxes;
	int numberOfBoxesOnFire;

	ShaderController shaderController;

	UPROPERTY()
	TArray<UFireBox *> boxesOnFire;

	void UpdateMaterialBoxes();

public:
	AFlammableActor();

	virtual void BeginPlay() override;

	void AddBurningBox(UFireBox * box);

	void RemoveBurningBox(UFireBox * box);

	virtual void Tick(float DeltaTime) override;

	void printNumberOfBoxes();

	void incrementBoxesOnFire();

	void decrementBoxes();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		float FireBoxSize = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		float StartingFuel = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		float StartingHealth = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		float ConsumptionFuel = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		int MaxFireBoxNeighbours = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		UParticleSystem * ParticleAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		UMaterial * DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Flammable)
		float TimeBetweenUpdates = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Flammable)
		float PercentOnFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Flammable)
		TArray<UBoxComponent *> boxMasks;



};
