// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "FlammableActor.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "FireBox.h"
#include "DrawDebugHelpers.h"


DECLARE_CYCLE_STAT(TEXT("Firebox Get Neighbours"), STAT_GetNeighbours, STATGROUP_FireBox);
DECLARE_CYCLE_STAT(TEXT("Firebox Consolidate"), STAT_Consolidate, STATGROUP_FireBox);
DECLARE_CYCLE_STAT(TEXT("Firebox Damage"), STAT_Damage, STATGROUP_FireBox);

UFireBox::UFireBox()
{
	// Every tick please -- maybe only needed when on fire
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Default values
	health = START_HEALTH;
	fuel = START_FUEL;
	emitter = nullptr;
	consumed = false;
	deactivated = false;
	timeSinceUpdate = 0.0f;
	TimeBetweenUpdates = 10000.0f;
}

void UFireBox::SetTimeBetweenUpdates(float time) {
	TimeBetweenUpdates = time;
}

void UFireBox::SetHealth(int value) {
	health = value;
}

void UFireBox::SetFuel(int value) {
	fuel = value;
}

void UFireBox::SetEmitter(UParticleSystemComponent * target) {
	this->emitter = target;
}

void UFireBox::SetConsumptionFuel(int value) {
	consumeFuel = value;
}

bool UFireBox::IsConsumed()
{
	return consumed;
}

void UFireBox::SetConsumed(bool value)
{
	consumed = value;
}

void UFireBox::SetDecalMaterial(UMaterial * decal)
{
	decalMaterial = decal;
}

void UFireBox::SetDecalLocation(FVector location)
{
	decalLocation = location;
}

void UFireBox::SetDecalDirection(FVector direction)
{
	decalDirection = direction;
}

void UFireBox::SetEmitterLocation(FVector location)
{
	emitterLocation = location;
}

int UFireBox::GetHealth()
{
	return health;
}

int UFireBox::GetFuel()
{
	return fuel;
}

UParticleSystemComponent * UFireBox::GetEmitter()
{
	return emitter;
}

bool UFireBox::IsBurning()
{
	return burning;
}

void UFireBox::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction * ThisTickFunction) {


	if (deactivated) {
		UE_LOG(LogTemp, Warning, TEXT("Deactivated Emitter Ticking!!"));
	}


	// Point upwards ALWAYSSSSS
	if (emitter) {
		emitter->SetWorldRotation(FRotator(0, 0, 0));
	}

	timeSinceUpdate += DeltaTime;
	if (!(timeSinceUpdate > TimeBetweenUpdates)) {
		return;
	}
	timeSinceUpdate = 0.0f;


	if (consumed) {
		PrimaryComponentTick.SetTickFunctionEnable(false);
		if (emitter) {
			emitter->Deactivate();
			this->Remove();
		}
		return;
	}


	// Tick Component -- only care if we are burning
	if (!burning) {
		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}

	// Remove fuel
	fuel--;

	// Go out?
	if (fuel <= 0) {

		/*

		UDecalComponent * decal = UGameplayStatics::SpawnDecalAttached(decalMaterial,
		this->GetUnscaledBoxExtent() * FVector(0.5f, 1.6f, 1.6f),
		this->GetAttachmentRoot(),
		FName("Decal"),
		GetComponentRotation().GetInverse().RotateVector(decalLocation) + GetComponentLocation(),  // unit
		GetComponentRotation().GetInverse().RotateVector(decalDirection).Rotation(),
		EAttachLocation::KeepWorldPosition,
		10000.0f);

		*/

		//FBoxSphereBounds bounds = decal->CalcBounds(decal->GetComponentToWorld());


		//	DrawDebugBox(GetWorld(), bounds.Origin, bounds.BoxExtent, GetComponentRotation().GetInverse().RotateVector(decalDirection).ToOrientationQuat() ,FColor(255,0,0), true, -1.0f, 0, 1);

		/*
		DrawDebugLine(
		GetWorld(),
		(FVector(i, j, k) * FireBoxSize) + box->GetComponentLocation(),
		box->GetComponentLocation(),
		FColor(255, 0, 0),
		true, -1, 0,
		1
		);*/

		burning = false;
		emitter->Deactivate();
		return;
	}


	TArray<UPrimitiveComponent *> neighbours;

	{

		SCOPE_CYCLE_COUNTER(STAT_GetNeighbours);
		// Damage neighbours

		this->GetOverlappingComponents(neighbours);

	}

	{
		SCOPE_CYCLE_COUNTER(STAT_Damage);
		for (UPrimitiveComponent * potNeighbour : neighbours) {
			if (potNeighbour->IsA(UFireBox::StaticClass())) {
				// Convert to Firebox
				UFireBox * neighbour = ((UFireBox *)potNeighbour);
				// Damage
				neighbour->Damage(10);

				{
					//	SCOPE_CYCLE_COUNTER(STAT_Consolidate);

					// Run Comsumption Algorithm
					if (!neighbour->IsConsumed() && // Don't consume those already consumed
						neighbour->GetAttachmentRootActor() == this->GetAttachmentRootActor() &&    // Only merge with boxes on this actor
						neighbour->IsBurning() && // If it's on fire
						neighbour->GetFuel() < consumeFuel  /*  // It has expended some of it's fuel
															this->GetUnscaledBoxExtent().X < MAXIMUM_SIZE &&
															this->GetUnscaledBoxExtent().X >= neighbour->GetUnscaledBoxExtent().X &&
															this->GetUnscaledBoxExtent().Y >= neighbour->GetUnscaledBoxExtent().Y &&
															this->GetUnscaledBoxExtent().Z >= neighbour->GetUnscaledBoxExtent().Z*/
						) {// And we haven't just consumed everything


						   // There is an assumption that boxes are only consumed on a grid
						   // i.e. the sides of each box line up completely, hence only one extent needs be adjusted

						FRotator actorRot = this->GetComponentRotation();


						// Get centres of boxes relative to actor, a eats b.
						FVector aLocLocal = actorRot.GetInverse().RotateVector(
							this->GetComponentLocation() - this->GetAttachmentRootActor()->GetActorLocation());
						FVector bLocLocal = actorRot.GetInverse().RotateVector(
							neighbour->GetComponentLocation() - neighbour->GetAttachmentRootActor()->GetActorLocation());


						bool xMidSame, yMidSame, zMidSame;
						bool xDimSame, yDimSame, zDimSame;
						FVector neighbourDim = neighbour->GetScaledBoxExtent();
						FVector thisDim = this->GetScaledBoxExtent();

						xDimSame = neighbourDim.X == thisDim.X;
						yDimSame = neighbourDim.Y == thisDim.Y;
						zDimSame = neighbourDim.Z == thisDim.Z;

						float tol = 0.01f;
						xMidSame = aLocLocal.X - tol < bLocLocal.X && bLocLocal.X < aLocLocal.X + tol;
						yMidSame = aLocLocal.Y - tol < bLocLocal.Y && bLocLocal.Y < aLocLocal.Y + tol;
						zMidSame = aLocLocal.Z - tol < bLocLocal.Z && bLocLocal.Z < aLocLocal.Z + tol;

						bool xSim = xDimSame && xMidSame;
						bool ySim = yDimSame && yMidSame;
						bool zSim = zDimSame && zMidSame;

						bool modified = false;
						FVector thisRatio = thisDim / (thisDim + neighbourDim);
						FVector neighbourRatio = neighbourDim / (thisDim + neighbourDim);
						FVector offset(0, 0, 0);
						FVector extent = this->GetScaledBoxExtent();

						if (xSim && ySim && !zSim) {
							// XY Face
							modified = true;
							extent.Z += neighbourDim.Z;


						}
						else if (xSim && !ySim && zSim) {
							// XZ Face
							modified = true;
							extent.Y += neighbourDim.Y;

						}
						else if (!xSim && ySim && zSim) {
							// YZ Face
							modified = true;
							extent.X += neighbourDim.X;
						}

						if (modified) {
							/*
							UE_LOG(LogTemp, Warning, TEXT("aGlobal: %s, bGlobal: %s"), *this->GetComponentLocation().ToString(), *neighbour->GetComponentLocation().ToString());
							UE_LOG(LogTemp, Warning, TEXT("aLocLocal: %s, bLocLocal: %s"), *aLocLocal.ToString(), *bLocLocal.ToString());
							UE_LOG(LogTemp, Warning, TEXT("aExtent: %s, bExtent: %s"), *thisDim.ToString(), *neighbourDim.ToString());
							UE_LOG(LogTemp, Warning, TEXT("newExtent %s"), *extent.ToString());
							UE_LOG(LogTemp, Warning, TEXT("thisRatio: %s, neighbourRatio: %s"), *thisRatio.ToString(), *neighbourRatio.ToString());*/
							this->SetBoxExtent(extent);
							this->fuel += neighbour->GetFuel();
							this->SetWorldLocation(this->GetComponentLocation() * thisRatio + neighbour->GetComponentLocation() * neighbourRatio);


							((AFlammableActor *) this->GetAttachmentRootActor())->decrementBoxes();
							((AFlammableActor *) this->GetAttachmentRootActor())->printNumberOfBoxes();
							// Set consumed on target
							neighbour->SetConsumed(true);
							neighbour->Remove();

							// Remove from burning boxes on parent

							//	FVector globalOffset = this->GetComponentRotation().RotateVector((-0.5f * ratio * difference));

							return;


							//	UE_LOG(LogTemp, Warning, TEXT("New Location: %s"), *this->GetComponentLocation().ToString());
							//		UE_LOG(LogTemp, Warning, TEXT("   "));

						}

					}
				}
			}

		}
	}













	//			// Here we go agggaaainnnn

	//			// Get the world locations
	//			FVector aWorldLocation = this->GetComponentLocation();
	//			FVector bWorldLocation = neighbour->GetComponentLocation();

	//			// Get the "origin", i.e the location of the actor
	//			FVector localOrigin = this->GetAttachmentRootActor()->GetActorLocation();
	//			
	//			FRotator worldRotation = this->GetAttachmentRootActor()->GetActorRotation();


	//			// Get two vectors which point to where we are in relation to this object
	//			FVector aLocalLocation = worldRotation.GetInverse().RotateVector(aWorldLocation - localOrigin);
	//			FVector bLocalLocation = worldRotation.GetInverse().RotateVector(bWorldLocation - localOrigin);

	//			







	//			 // Get centres of boxes, a eats b.
	//			FVector aLoc = this->GetComponentLocation();
	//			FVector bLoc = neighbour->GetComponentLocation();
	//	//		UE_LOG(LogTemp, Warning, TEXT("A Location at %f %f %f"), aLoc.X, aLoc.Y, aLoc.Z);
	//	//		UE_LOG(LogTemp, Warning, TEXT("B Location at %f %f %f"), bLoc.X, bLoc.Y, bLoc.Z);

	//			// You can make the assumption that every consumable box is rotated to the same orientation.
	//			// Find Midpoint
	//			FVector midpoint = (this->GetComponentLocation() + neighbour->GetComponentLocation()) / 2;

	//	//		UE_LOG(LogTemp, Warning, TEXT("Midpoint at %f %f %f"), midpoint.X, midpoint.Y, midpoint.Z);

	//			FRotator rotation = GetComponentRotation();

	//			// Previous two were world locations
	//			FVector na = (aLoc - midpoint);
	//			na = rotation.GetInverse().RotateVector(na);

	//			FVector nb = (bLoc - midpoint);
	//			nb = rotation.GetInverse().RotateVector(nb);

	//	//		UE_LOG(LogTemp, Warning, TEXT("NA at %f %f %f"), na.X, na.Y, na.Z);
	//	//		UE_LOG(LogTemp, Warning, TEXT("NB at %f %f %f"), nb.X, nb.Y, nb.Z);

	//			// Find maximum positive extent in X
	//			FVector exaPos = na + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).X;
	//			FVector exbPos = nb + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).X;
	//			float xExtentPos = ((exaPos.X > exbPos.X) ? exaPos.X : exbPos.X);

	//			// Find maximum negative extent
	//			FVector exaNeg = (na + (rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).X * -1.0f)) * -1.0f;
	//			FVector exbNeg = (nb + (rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).X * -1.0f)) * -1.0f;
	//			float xExtentNeg = ((exaNeg.X > exbNeg.X) ? exaNeg.X : exbNeg.X);

	//			float xExtent = ((xExtentNeg > xExtentPos) ? xExtentNeg : xExtentPos);



	//			// Find maximum positive extent in Y
	//			FVector eyaPos = na + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Y;
	//			FVector eybPos = nb + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Y;
	//			float yExtentPos = ((eyaPos.Y > eybPos.Y) ? eyaPos.Y : eybPos.Y);

	//	//		UE_LOG(LogTemp, Warning, TEXT("Positive Y Extents %f %f "), eyaPos.Y, eybPos.Y);

	//			// Find maximum negative extent
	//			FVector eyaNeg = (na - (rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Y)) * -1.0f;
	//			FVector eybNeg = (nb - (rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Y)) * -1.0f;
	//			float yExtentNeg = ((eyaNeg.Y > eybNeg.Y) ? eyaNeg.Y : eybNeg.Y);

	////			UE_LOG(LogTemp, Warning, TEXT("Negative Y Extents %f %f "), eyaNeg.Y, eybNeg.Y);


	//			float yExtent = ((yExtentNeg > yExtentPos) ? yExtentNeg : yExtentPos);



	//			// Find maximum positive extent in Z
	//			FVector ezaPos = na + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Z;
	//			FVector ezbPos = nb + rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Z;
	//			float zExtentPos = ((ezaPos.Z > ezbPos.Z) ? ezaPos.Z : ezbPos.Z);

	//			// Find maximum negative extent
	//			FVector ezaNeg = (na + (rotation.GetInverse().RotateVector(this->GetScaledBoxExtent()).Z * -1.0f)) * -1.0f;
	//			FVector ezbNeg = (nb + (rotation.RotateVector(this->GetScaledBoxExtent()).Z * -1.0f)) * -1.0f;
	//			float zExtentNeg = ((ezaNeg.Z > ezbNeg.Z) ? ezaNeg.Z : ezbNeg.Z);

	//			float zExtent = ((zExtentNeg > zExtentPos) ? zExtentNeg : zExtentPos);



	//		//	UE_LOG(LogTemp, Warning, TEXT("New Extends %f %f %f"), xExtent, yExtent, zExtent);

	//			// Find maximum negative extent in X

	//			// Find new extents
	//		//	float extent = this->GetUnscaledBoxExtent().X * 2;
	//		//	float nExtent = neighbour->GetUnscaledBoxExtent().X * 2;

	//	//		float newExtent = FMath::Pow((FMath::Pow(extent, 3.0f) + FMath::Pow(nExtent, 3.0f)),  0.333333f) / 2;

	//		//	UE_LOG(LogTemp, Warning, TEXT("New Extent %f"), newExtent);

	//			// Adjust Location
	//			this->SetWorldLocation(midpoint);
	//			this->SetBoxExtent(FVector(xExtent, yExtent, zExtent));

	//			this->health += neighbour->GetHealth();

	//			// Set consumed on target
	//			neighbour->SetConsumed(true);
	//			
	//			if (emitter) {
	//				emitter->SetRelativeScale3D(FVector(xExtent / 40.0f, yExtent / 40.0f, zExtent / 40.0f));
	//			}

	//		}

	//	}
	//}

}


void UFireBox::Damage(int damage) {
	// Take damage from some source
	if (burning || fuel <= 0) {
		// We dont care
		return;
	}

	else {
		health -= damage;
		if (health <= 0) {
			PrimaryComponentTick.SetTickFunctionEnable(true);

			burning = true;

			// Increment the number of burning boxes
			if (this->GetAttachmentRootActor()->IsA(AFlammableActor::StaticClass())) {
				((AFlammableActor *) this->GetAttachmentRootActor())->incrementBoxesOnFire();
				((AFlammableActor *) this->GetAttachmentRootActor())->AddBurningBox(this);
				((AFlammableActor *) this->GetAttachmentRootActor())->RegisterBoxWithShader(this);
			}

			//	UE_LOG(LogTemp, Warning, TEXT("Catching Fire"));
			if (!emitter) {
				//			UE_LOG(LogTemp, Warning, TEXT("No Emitter Found"));
				return;
			}
			emitter->Activate();
		}
	}
}

void UFireBox::Wet(int value)
{
	fuel -= value;
}



void UFireBox::Remove() {
	deactivated = true;
	// Remove this object
	// If we have an emitter, remove it.
	if (emitter) {
		emitter->UnregisterComponent();
	}
	if (burning) {
		((AFlammableActor *) this->GetAttachmentRootActor())->RemoveBurningBox(this);
	}
	//	this->MarkPendingKill();
	this->UnregisterComponent();
	this->DestroyComponent();
}
