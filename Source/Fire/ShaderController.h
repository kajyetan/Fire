// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * Contains a representation of information to be passed to the shader for an object
 * Essentially a large amount of boxes, in the most effective representation for passing off to the GPU
 */
class FIRE_API ShaderController
{

private:
	// Final output boxes, bounded by 100
	TArray<FBoxSphereBounds *> ShaderBoxes;

	float VolumeIncreaseLimit;



public:
	ShaderController();

	~ShaderController();

	void init(float VolumeIncreaseLimit);

	void AddBox(FVector location, FVector extent);

	TArray<FBoxSphereBounds *> GetBoxes();

	bool ConsolidateBoxes();
};
