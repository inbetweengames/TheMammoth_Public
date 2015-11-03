// copyright 2015 inbetweengames GBR

#pragma once

#include "Components/MaterialBillboardComponent.h"
#include "AnimatedBillboardComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=Rendering, collapsecategories, hidecategories=(Sprite,Object,Activation,"Components|Activation",Physics,Collision,Lighting,Mesh,PhysicsVolume), editinlinenew, meta=(BlueprintSpawnableComponent))
class UAnimatedBillboardComponent : public UMaterialBillboardComponent
{
	GENERATED_BODY()
	
public:
	UAnimatedBillboardComponent();


	virtual void OnRegister() override;

protected:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	float m_animSpeed;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	int32 m_numFramesX;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	int32 m_numFramesY;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	FVector2D m_texResolution;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	int32 m_numFrames;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	int32 m_animStart;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	UTexture2D *m_sourceTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animation)
	TArray<struct FMaterialSpriteElement> m_sourceSprites;

	void SetupMaterials();

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic *> m_MIDs;

	FName m_animSpeedName;
	FName m_textureSetupName;
	FName m_framesTotalName;
	FName m_animStartName;
	FName m_sourceTextureName;
};
