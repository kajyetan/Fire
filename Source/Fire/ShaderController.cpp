// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "ShaderController.h"

ShaderController::ShaderController()
{
}

ShaderController::~ShaderController()
{
}


void ShaderController::init() {
	// Basic init function, currently does nothing

}

void ShaderController::AddBox(FVector location, FVector extent)
{
	// Add this box to the collection of boxes

	// Basic process, iterate through collection, determining if that box consuming new box would result
	// in an appropriate volume wasteage.

	FBoxSphereBounds * newBox = new FBoxSphereBounds(location, extent, 0);		// REMEMBER TO DEALLOC IF NOT USED





}


/* */