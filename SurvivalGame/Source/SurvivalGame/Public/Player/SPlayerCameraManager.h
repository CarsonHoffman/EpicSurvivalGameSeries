// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "SPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

	ASPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	/* Update the FOV */
	virtual void UpdateCamera(float DeltaTime) override;

	virtual void BeginPlay() override;

public:

	// Function to call for looking at an actor.
	UFUNCTION(BlueprintCallable, Category = "Focus")
	void LookAtActor(AActor* Actor, float TimeInSecondsToFocusOnActor);

	bool IsFocusing();

private:

	void PerformZooming(float TargetFOV, float DeltaTime, float Speed);

	void StopLookingAtActor();

	void CompleteUnFocus();

	/* Value overridden to true if MyPawn->IsTargeting() is true, be aware */
	bool bWantsToZoom;

	bool bFocusing;

	float CurrentCrouchOffset;

	AActor* TargetActor;

	FRotator OriginalRotation;

	/* Maximum camera offset applied when crouch is initiated. Always lerps back to zero */
	float MaxCrouchOffsetZ;

	float CrouchLerpVelocity;

	bool bWasCrouched;

	/* Default relative Z offset of the player camera */
	float DefaultCameraOffsetZ;

	/* default, hip fire FOV */
	float NormalFOV;

	/* aiming down sight / zoomed FOV */
	float TargetingFOV;

	/* FOV to transition to while at max speed. When not at max speed, FOV is somewhere between NormalFOV and this value */
	float MaxSpeedFOV;

public:

	/* Zooming on POI FOV */
	float ZoomingFOV;
};
