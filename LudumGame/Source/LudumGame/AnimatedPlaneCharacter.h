// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
// copyright 2015 inbetweengames GBR

#pragma once

#include "GameFramework/Character.h"
#include "AnimatedPlaneComponent.h"
#include "AnimatedPlaneCharacter.generated.h"

// AAnimatedPlaneCharacter behaves like ACharacter, but uses a UAnimatedPlaneComponent instead of a USkeletalMeshComponent as a visual representation
// Note: The variable named Mesh will not be set up on this actor!
UCLASS()
class AAnimatedPlaneCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()
	
protected:

	// Name of the Sprite component
	static FName SpriteComponentName;

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimatedPlaneComponent* m_animatedSprite;

	/** Called for side to side input */
	void MoveRight(float Val);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_idleAnim;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_walkAnimR;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_walkAnimL;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	FAnimationSetup m_fallingAnim;

	virtual void Tick(float DeltaSeconds) override;

	void UpdateAnimation();

public:

	virtual void PostInitializeComponents() override;

	/** Returns Sprite subobject **/
	FORCEINLINE class UAnimatedPlaneComponent* GetSprite() const { return m_animatedSprite; }
};
