// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "Array.h"
#include "FlammableActor.h"
#include "DrawDebugHelpers.h"
#include "DrawDebugHelpers.h"


AFlammableActor::AFlammableActor()
{

	// Nothing dynamic in constructors
	PrimaryActorTick.bCanEverTick = true;

	this->GetStaticMeshComponent()->CreateDynamicMaterialInstance(0, this->GetStaticMeshComponent()->GetMaterial(0));

	shaderController.init();

}



void AFlammableActor::AddBurningBox(UFireBox * box) {

	boxesOnFire.Add(box);
	UpdateMaterialBoxes();

}

void AFlammableActor::RemoveBurningBox(UFireBox * box) {

	boxesOnFire.Remove(box);
	UpdateMaterialBoxes();

}

void AFlammableActor::UpdateMaterialBoxes() {
	//	UE_LOG(LogTemp, Warning, TEXT("Number of boxes on fire %d"), boxesOnFire.Num());
	// 40 currently the limit
	char buf1[40];
	char buf2[40];
	for (int i = 0; i < FMath::Min(boxesOnFire.Num(), 100); i++) {

		snprintf(buf1, 40, "Box%dLocation", i);
		snprintf(buf2, 40, "Box%dExtent", i);

		this->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(buf1, boxesOnFire[i]->GetComponentLocation());
		this->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(buf2, boxesOnFire[i]->GetScaledBoxExtent());

	}

}




void AFlammableActor::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	//	UpdateMaterialBoxes();

	//	UE_LOG(LogTemp, Warning, TEXT("Location: %s, Extent: %s"), *boxes[0]->GetComponentLocation().ToString(), *boxes[0]->GetScaledBoxExtent().ToString()); 
	//	flameController.update();
}

void AFlammableActor::printNumberOfBoxes()
{
	//UE_LOG(LogTemp, Warning, TEXT("Number of boxes %d"), numberOfBoxes);
}

void AFlammableActor::incrementBoxesOnFire()
{
	numberOfBoxesOnFire++;

	// Update float;
	PercentOnFire = (float)numberOfBoxesOnFire / (float)numberOfBoxes;

}

void AFlammableActor::decrementBoxes()
{
	numberOfBoxes--;
}


void AFlammableActor::Subdivide(TArray<UFireBox *>& tempBoxes) {
	// Take a TArray and modify it such that it contains valid (colliding) fireboxes with extents half of that provided

	if (tempBoxes.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Attempted to subdivide 0 boxes"));
		return;
	}

	// Figure out new extents
	FVector currentExtent = tempBoxes[0]->GetUnscaledBoxExtent();
	FVector newExtent = currentExtent / 2.0f;

	UE_LOG(LogTemp, Warning, TEXT("Recieved unscaled extent of %s"), *currentExtent.ToString());

	//UE_LOG(LogTemp, Warning, TEXT("Starting subdivide with %d boxes"), tempBoxes.Num());

	TArray<UFireBox *> boxesForRemoval;
	TArray<UFireBox *> boxesForAddition;
	TArray<UPrimitiveComponent *> overlappingComponents;

	float i, j, k; // iterators

	for (UFireBox * parentBox : tempBoxes) {
		// Make eight smaller boxes inside the larger one

		// Get world location and offset of box
		FVector boxWorldLocation = parentBox->GetComponentLocation();

		// Iterate through positions
		for (i = -1; i <= 1; i += 2) {  // just give me -1 and 1 over 3 iterators
			for (j = -1; j <= 1; j += 2) {
				for (k = -1; k <= 1; k += 2) {

					// Create a box of appropriate size
					UFireBox * box = NewObject<UFireBox>(this);

					box->InitBoxExtent(newExtent);

					box->RegisterComponent();

					// Technically middle of box is equiv to start extent 
					box->SetWorldLocation((FVector(i, j, k) * newExtent) + parentBox->GetComponentLocation());

					// Enable collision and determine who is overlapping this box
					box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

					box->GetOverlappingComponents(overlappingComponents);

					if (overlappingComponents.Contains(RootComponent)) {

						boxesForAddition.Add(box);

					}
					else {
						box->UnregisterComponent();
					}
				}
			}
		}

		// We've split this box up, remove it.
		boxesForRemoval.Add(parentBox);
	}

	// All boxes considered, apply removal and addition
	for (auto box : boxesForRemoval) {
		tempBoxes.Remove(box);
		//	UE_LOG(LogTemp, Warning, TEXT("Removing boxes"));
		box->Remove();
	}

	for (auto box : boxesForAddition) {
		tempBoxes.Add(box);
	}

	// Current count of boxes


}


void AFlammableActor::BeginPlay()
{

	Super::BeginPlay();

	// Here we go againnnnnn




	/* Get Actor Bounds */
	FVector GlobalOrigin;
	FVector GlobalBoundsExtent;
	GetActorBounds(false, GlobalOrigin, GlobalBoundsExtent);


	// Setup the initial box size
	int depth = 0;  // How many subdivisions
	float initialSize = FMath::Pow(2, depth) * FireBoxSize;


	// Figure out how many boxes we'll need in each direction, then add one for good measure
	int NumXBoxes = FMath::CeilToInt((GlobalBoundsExtent.X / initialSize)) + 1;
	int NumYBoxes = FMath::CeilToInt((GlobalBoundsExtent.Y / initialSize)) + 1;
	int NumZBoxes = FMath::CeilToInt((GlobalBoundsExtent.Z / initialSize)) + 1;

	/* Calulate the vector to the middle of the starting box, basically on the corner*/

	float StartYPos = -1.0f * (((NumYBoxes * initialSize * 2) / 2.0f) - (0.5f * initialSize));
	float StartXPos = -1.0f * (((NumXBoxes * initialSize * 2) / 2.0f) - (0.5f * initialSize));
	float StartZPos = -1.0f * (((NumZBoxes * initialSize * 2) / 2.0f) - (0.5f * initialSize));

	// Iterators
	float i, j, k;

	for (i = StartXPos; i <= -1.0f * StartXPos; i += initialSize * 2.0f) {
		for (j = StartYPos; j <= -1.0f * StartYPos; j += initialSize * 2.0f) {
			for (k = StartZPos; k <= -1.0f * StartZPos; k += initialSize * 2.0f) {

				// Create a box that will possibly interest the mesh
				UFireBox * possibleBox = NewObject<UFireBox>(this);

				possibleBox->InitBoxExtent(FVector(1.0f, 1.0f, 1.0f) * initialSize);

				possibleBox->RegisterComponent();

				// Relocate this box to the desired position
				possibleBox->SetWorldLocation(FVector(i, j, k) + GlobalOrigin);

				// Enable query collision
				possibleBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

				if (possibleBox->IsOverlappingActor(this)) {
					// Box is legit.
					boxes.Add(possibleBox);
					possibleBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

				}
				else {
					possibleBox->Remove();
				}
			}
		}
	}

	// Run Subdivision
	for (int l = 0; l < depth; l++) {
		Subdivide(boxes);
	}



	// Move the interesting boxes to be part of the object
	FRotator GlobalActorRotation = GetActorRotation();
	FVector GlobalLocation = GetActorLocation();

	for (auto box : boxes) {
		box->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

		box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

		box->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		// Create a particle system that we can activate or deactivate
		UParticleSystemComponent * fireParticle = NewObject<UParticleSystemComponent>(this);

		fireParticle->AttachTo(RootComponent);

		fireParticle->SetWorldLocation(box->GetComponentLocation());
		fireParticle->RegisterComponent();

		fireParticle->bAutoActivate = false;

		fireParticle->SetWorldRotation(FRotator(0, 0, 0));

		// TO DO HERE : intellegent placement

		fireParticle->SetTemplate(ParticleAsset);

		box->SetEmitter(fireParticle);

		box->SetHealth(StartingHealth);
		box->SetFuel(StartingFuel);
		box->SetConsumptionFuel(ConsumptionFuel);
		box->SetTimeBetweenUpdates(TimeBetweenUpdates);

		box->SetDecalMaterial(DecalMaterial);

		// WE SHOULD ALSO EXTEND THE BOX SIZES BECAUSE WE MADE THEM NOT OVERLAP
		box->SetBoxExtent(box->GetScaledBoxExtent() + 0.1f);

	}


	// Build array of boxes overlapping many boxes
	TArray<UFireBox *> boxesForRemoval;
	TArray<UPrimitiveComponent *> neighbours;

	for (UFireBox* box : boxes) {

		box->GetOverlappingComponents(neighbours);
		int count = 0;
		for (UPrimitiveComponent * n : neighbours) {
			if (n->IsA(UFireBox::StaticClass()) && n->GetOwner() == this) {
				count++;
			}
		}

		if (count > MaxFireBoxNeighbours) {
			boxesForRemoval.Add(box);
			UE_LOG(LogTemp, Warning, TEXT("Removing uneeded component"));
		}
		else if (count > 26) {
			//	UE_LOG(LogTemp, Warning, TEXT("More than 26 neighbours"));
		}

	}

	// Unregister those boxes
	for (UFireBox * box : boxesForRemoval) {
		box->Remove();
		boxes.Remove(box);
	}



	// Figure out where we would apply a decal and where the 
	for (UFireBox * box : boxes) {



		FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true);
		//RV_TraceParams.AddIgnoredComponent(box);
		RV_TraceParams.bTraceComplex = true;
		RV_TraceParams.bTraceAsyncScene = true;
		RV_TraceParams.bReturnPhysicalMaterial = true;

		//Re-initialize hit info
		FHitResult RV_OutwardsHit(ForceInit);
		FHitResult RV_InwardsHit(ForceInit);

		int outwardsCount = 0;
		int inwardsCount = 0;

		bool found = false;

		for (i = -1.0f; i < 2.0f; i += 2.0f) {	// Three set of negative to positive please
			for (j = -1.0f; j < 2.0f; j += 2.0f) {
				for (k = -1.0f; k < 2.0f; k += 2.0f) {


					/*
					DrawDebugLine(
					GetWorld(),
					(FVector(i, j, k) * FireBoxSize) + box->GetComponentLocation(),
					box->GetComponentLocation(),
					FColor(255, 0, 0),
					true, -1, 0,
					1
					);
					*/


					// Cast a line trace to the respective corner
					// ISSUE - another actor in the way?
					GetWorld()->LineTraceSingleByChannel(RV_InwardsHit,
						(FVector(i, j, k) * FireBoxSize) + box->GetComponentLocation(),
						box->GetComponentLocation(),
						ECC_MAX,
						RV_TraceParams);


					if (RV_InwardsHit.GetComponent()) {

						// Inwards hit on a component -- literally just go with the first one
						box->SetDecalLocation(RV_InwardsHit.ImpactPoint - box->GetComponentLocation());
						box->SetDecalDirection(RV_InwardsHit.ImpactNormal);

						box->SetEmitterLocation(RV_InwardsHit.ImpactPoint - box->GetComponentLocation());

						//				DrawDebugPoint(GetWorld(), RV_InwardsHit.ImpactPoint, 3, FColor(0, 255, 0), true, -1.0f, 0);

						found = true;
						break;



					}




					// Cast a line trace to the respective corner
					// ISSUE - another actor in the way?
					GetWorld()->LineTraceSingleByChannel(RV_OutwardsHit,
						box->GetComponentLocation(),
						(FVector(i, j, k) * FireBoxSize) + box->GetComponentLocation(),
						ECC_MAX,
						RV_TraceParams);


					if (RV_OutwardsHit.GetComponent()) {

						// Outwards hit on a component -- literally just go with the first one
						box->SetDecalLocation(RV_OutwardsHit.ImpactPoint - box->GetComponentLocation());
						box->SetDecalDirection(RV_OutwardsHit.ImpactNormal);

						box->SetEmitterLocation(RV_OutwardsHit.ImpactPoint - box->GetComponentLocation());


						//			DrawDebugPoint(GetWorld(), RV_OutwardsHit.ImpactPoint, 3, FColor(0, 0, 255), true, -1.0f, 0);
						found = true;
						break;

					}




				}

				if (found) {    // Yeah yeah loops and shit
					break;
				}
			}
			if (found) {
				break;
			}
		}

		if (!found) {
			// No hits, fuck? Just try our best
			GetWorld()->LineTraceSingleByChannel(RV_OutwardsHit,
				box->GetComponentLocation(),
				box->GetAttachmentRoot()->GetComponentLocation(),
				ECC_MAX,
				RV_TraceParams);

			// If this doesn't hit....
			if (!RV_OutwardsHit.GetComponent()) {
				//		UE_LOG(LogTemp, Warning, TEXT("Unable to guess appropriate decal and emitter locations - panic"));

				box->SetDecalLocation(box->GetComponentLocation() - box->GetComponentLocation());
				box->SetDecalDirection(this->GetActorLocation() - box->GetComponentLocation());

				//		DrawDebugPoint(GetWorld(), box->GetComponentLocation(), 3, FColor(0, 0, 0), true, -1.0f, 0);
				box->SetEmitterLocation(box->GetComponentLocation() - box->GetComponentLocation());

			}
			else {
				//		UE_LOG(LogTemp, Warning, TEXT("Unable to guess appropriate decal and emitter locations - RESOLVED"));
				box->SetDecalLocation(RV_OutwardsHit.ImpactPoint - box->GetComponentLocation());
				box->SetDecalDirection(RV_OutwardsHit.ImpactNormal);

				//		DrawDebugPoint(GetWorld(), RV_OutwardsHit.ImpactPoint, 3, FColor(0, 255, 255), true, -1.0f, 0);
				box->SetEmitterLocation(RV_OutwardsHit.ImpactPoint - box->GetComponentLocation());

			}





		}



		//// Here we go?
		//UDecalComponent * decal = NewObject<UDecalComponent>(this);
		//decal->SetMaterial(0, decalMaterial);
		//UE_LOG(LogTemp, Warning, TEXT("Decal Scale %s"), *decal->GetComponentScale().ToString());

		//decal->SetWorldLocation(decalLocation);

		//decal->AttachToComponent(this->GetAttachmentRoot(), FAttachmentTransformRules::KeepWorldTransform);

		//decal->SetWorldRotation(decalDirection.Rotation());

		//decal->SetWorldScale3D(FVector(0.05f, 0.05f, 0.05f));

		//decal->RegisterComponent();


		//FBoxSphereBounds bounds = decal->CalcBounds(decal->GetComponentToWorld());



		//DrawDebugBox(GetWorld(), bounds.Origin, bounds.BoxExtent, FColor(255, 0, 0), true, -1.0f, 0, 1);

		/*
		DrawDebugLine(
		GetWorld(),
		(FVector(i, j, k) * FireBoxSize) + box->GetComponentLocation(),
		box->GetComponentLocation(),
		FColor(255, 0, 0),
		true, -1, 0,
		1
		);*/




	}
	// Record initial number of boxes
	numberOfBoxes = boxes.Num();

	UE_LOG(LogTemp, Warning, TEXT("Starting with %d boxes"), boxes.Num());

}


