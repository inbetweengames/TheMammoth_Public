// copyright 2015 inbetweengames GBR

#pragma once

#include "Components/MaterialBillboardComponent.h"
#include "AnimatedPlaneComponent.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FAnimationSetup 
{
	GENERATED_BODY();

	FAnimationSetup()
	{
		m_animSpeed = 16.0f;
		m_numFramesX = 4;
		m_numFramesY = 4;
		m_texResolution = FVector2D(1024.0f, 1024.0f);
		m_numFrames = 16;
		m_animStartLeft = 0;
		m_sourceTexture = nullptr;
	}
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Animation)
	float m_animSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	int32 m_numFramesX;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	int32 m_numFramesY;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	FVector2D m_texResolution;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	int32 m_numFrames;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	int32 m_animStartLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	int32 m_animStartRight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	UTexture2D *m_sourceTexture;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Animation)
	FLinearColor m_colorMod;

	friend bool operator==(const FAnimationSetup& LHS, const FAnimationSetup& RHS);
};

/**
 * 
 */
UCLASS(ClassGroup=(Rendering, Common), hidecategories=(Object,Activation,"Components|Activation"), ShowCategories=(Mobility), editinlinenew, meta=(BlueprintSpawnableComponent))
class UAnimatedPlaneComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UAnimatedPlaneComponent();

	virtual void OnRegister() override;

	virtual void InitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category=Animation)
	void SetNewAnimation(const FAnimationSetup &newSetup, bool playLeft = true);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	bool m_doDepthSort;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

protected:
	

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	UMaterial *m_sourceMaterial;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Animation)
	FAnimationSetup m_animSetup;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Animation)
	bool m_isPlayingLeftAnim;

	void SetupAnimation();

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic *> m_MIDs;

	FName m_textureSetupName;
	FName m_sourceTextureName;
	FName m_colourModName;
	FName m_currFrameName;


	float m_currentTimePerFrame;
	float m_currentFrameTime;
	int32 m_currentFrame;

	void PushCurrentFrame();
};
