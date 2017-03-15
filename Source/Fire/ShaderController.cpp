// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "DrawDebugHelpers.h"
#include "ShaderController.h"

bool ShaderController::ConsolidateBoxes()
{
	// Takes the current set of boxes and sees if any one pair of boxes can be merged together
	// returns whether or not a merge occurs

	float proposedVolume;  // If we create one encompassing box, how much volume will that have?
	float additiveVolume;
	bool eaten = false;
	//UE_LOG(LogTemp, Warning, TEXT("Conslidating Boxes"));

	for (auto a : ShaderBoxes) {
		for (auto b : ShaderBoxes) {

			FBoxSphereBounds * combinedBox = new FBoxSphereBounds(*a + *b);			// Deleted if inneffecient, otherwise replaces box

			proposedVolume = (*combinedBox).GetBox().GetVolume();							// Holy overloaded plus batman
			additiveVolume = a->GetBox().GetVolume() + b->GetBox().GetVolume();		// Holy deferencing interleave batman

			if (proposedVolume / additiveVolume < VolumeIncreaseLimit && a != b) {
				// Eat the boxes
				ShaderBoxes.Remove(a);   // Concurrent modification? I assume it's ok since we break immediately
				ShaderBoxes.Remove(b);

				ShaderBoxes.Add(combinedBox);

				delete a;
				delete b;
				eaten = true;
				break;
			}

			delete combinedBox;

		}
		if (eaten) {
			break;    // GOTO considered harmfull...
		}
	}


	return eaten;
	
}

ShaderController::ShaderController()
{

}

ShaderController::~ShaderController()
{
}


void ShaderController::init(float limit) {
	// Basic init function, currently does nothing
	VolumeIncreaseLimit = limit;
}

void ShaderController::AddBox(FVector location, FVector extent)
{
	// Add this box to the collection of boxes

	// Basic process, iterate through collection, determining if that box consuming new box would result
	// in an appropriate volume wasteage.

	FBoxSphereBounds * newBox = new FBoxSphereBounds(location, extent, 0);		// Deleted if consumed, otherwise retained

	if (!newBox) {
		// Hmmm
		return;
	}


	float proposedVolume;  // If we create one encompassing box, how much volume will that have?
	float additiveVolume;  // If we kept them separate, how much volume would that have?
	bool eaten = false;


	for (auto box : ShaderBoxes) {
		// Figure out how much wasted volume we would have if this box absobed this box
		FBoxSphereBounds * combinedBox = new FBoxSphereBounds(*box + *newBox);			// Deleted if inneffecient, otherwise replaces box

		proposedVolume = (*combinedBox).GetBox().GetVolume();							// Holy overloaded plus batman
		additiveVolume = box->GetBox().GetVolume() + newBox->GetBox().GetVolume();		// Holy deferencing interleave batman

		if (proposedVolume / additiveVolume < VolumeIncreaseLimit) {
			// Eat the boxes
			ShaderBoxes.Remove(box);   // Concurrent modification? I assume it's ok since we break immediately
			ShaderBoxes.Add(combinedBox);

			delete box;
			delete newBox;
			eaten = true;
			break;
		}

		delete combinedBox;
	}

	if (!eaten) {
		// Our attempts to eat have failed, or there is just no boxes
		ShaderBoxes.Add(newBox);

		// Try and consolidate   -- TODO: THIS MAY HAPPEN TOO MUCH
		while (ConsolidateBoxes());


	}


}

TArray<FBoxSphereBounds*> ShaderController::GetBoxes()
{
	return ShaderBoxes;
}


