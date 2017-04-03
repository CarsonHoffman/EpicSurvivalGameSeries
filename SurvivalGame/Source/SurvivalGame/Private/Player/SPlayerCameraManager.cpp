// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SurvivalGame.h"
#include "SPlayerCameraManager.h"
#include "SCharacter.h"
#include "Kismet/KismetMathLibrary.h"

ASPlayerCameraManager::ASPlayerCameraManager(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	TargetingFOV = 65.0f;
	MaxSpeedFOV = 115.f;

	ViewPitchMin = -80.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;

	/* The camera delta between standing and crouched */
	MaxCrouchOffsetZ = 46.0f;

	/* HACK. Mirrored from the 3rd person camera offset from SCharacter */
	DefaultCameraOffsetZ = 20.0f;

	/* Ideally matches the transition speed of the character animation (crouch to stand and vice versa) */
	CrouchLerpVelocity = 12.0f;
}


void ASPlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();

// 	ASCharacter* MyPawn = PCOwner ? Cast<ASCharacter>(PCOwner->GetPawn()) : nullptr;
// 	if (MyPawn)
// 	{
// 		/* Cache the offset Z value of the camera to maintain a correct offset while transition during crouches */
// 		DefaultCameraOffsetZ = MyPawn->GetCameraComponent()->GetRelativeTransform().GetLocation().Z;
// 	}
}

void ASPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	// Zooming block
	ASCharacter* MyPawn = PCOwner ? Cast<ASCharacter>(PCOwner->GetPawn()) : nullptr;
	if (MyPawn)
	{
		if (bWantsToZoom)
		{
			PerformZooming(ZoomingFOV, DeltaTime, 5.f);
		}
		else if (MyPawn->IsTargeting())
		{
			PerformZooming(TargetingFOV, DeltaTime, 10.f);
		}
		else
		{
			float NewFOV = FMath::Lerp(NormalFOV, MaxSpeedFOV, MyPawn->GetVelocity().Size() / 1000.f);
			PerformZooming(NewFOV, DeltaTime, 10.f);
		}
	}

	// Focusing block
	if (MyPawn && bFocusing)
	{
		FRotator RotateTo = TargetActor ? UKismetMathLibrary::FindLookAtRotation(MyPawn->GetCameraComponent()->GetComponentLocation(), TargetActor->GetActorLocation()) : OriginalRotation;
		FRotator NewRotation = FMath::RInterpTo(MyPawn->GetCameraComponent()->GetComponentRotation(), RotateTo, DeltaTime, 5.f);
		MyPawn->GetCameraComponent()->SetWorldRotation(NewRotation);

		// Once we're back to our original position, stop focusing
		if (NewRotation.Equals(OriginalRotation, 0.05f))
		{
			MyPawn->GetCameraComponent()->SetWorldRotation(OriginalRotation);
			CompleteUnFocus();
		}
	}

	/* Apply smooth camera lerp between crouch toggling */
	if (MyPawn)
	{
		if (MyPawn->bIsCrouched && !bWasCrouched)
		{
			CurrentCrouchOffset = MaxCrouchOffsetZ;
		}
		else if (!MyPawn->bIsCrouched && bWasCrouched)
		{
			CurrentCrouchOffset = -MaxCrouchOffsetZ;
		}

		bWasCrouched = MyPawn->bIsCrouched;
		/* Clamp the lerp to 0-1.0 range and interpolate to our new crouch offset */
		CurrentCrouchOffset = FMath::Lerp(CurrentCrouchOffset, 0.0f, FMath::Clamp(CrouchLerpVelocity * DeltaTime, 0.0f, 1.0f));

		FVector CurrentCameraOffset = MyPawn->GetCameraComponent()->GetRelativeTransform().GetLocation();
		FVector NewCameraOffset = FVector(CurrentCameraOffset.X, CurrentCameraOffset.Y, DefaultCameraOffsetZ + CurrentCrouchOffset);
		MyPawn->GetCameraComponent()->SetRelativeLocation(NewCameraOffset);
	}

	Super::UpdateCamera(DeltaTime);
}

void ASPlayerCameraManager::PerformZooming(float TargetFOV, float DeltaTime, float Speed)
{
	DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, Speed);
	SetFOV(DefaultFOV);
}

void ASPlayerCameraManager::LookAtActor(AActor* Actor, float TimeInSecondsToFocusOnActor)
{
	bWantsToZoom = true;
	bFocusing = true;
	TargetActor = Actor;

	ASCharacter* MyPawn = PCOwner ? Cast<ASCharacter>(PCOwner->GetPawn()) : nullptr;
	if (MyPawn)
	{
		OriginalRotation = MyPawn->GetCameraComponent()->GetComponentRotation();
	}

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &ASPlayerCameraManager::StopLookingAtActor, TimeInSecondsToFocusOnActor, false);
}

bool ASPlayerCameraManager::IsFocusing()
{
	return bFocusing;
}

void ASPlayerCameraManager::StopLookingAtActor()
{
	TargetActor = nullptr;
	bWantsToZoom = false;
}

void ASPlayerCameraManager::CompleteUnFocus()
{
	bFocusing = false;
}