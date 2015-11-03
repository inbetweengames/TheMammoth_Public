// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "AnimatedPlaneComponent.h"

UAnimatedPlaneComponent::UAnimatedPlaneComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = false;

	bWantsInitializeComponent = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneShape(TEXT("/Game/StaticMeshes/Shape_Plane.Shape_Plane"));
	if (PlaneShape.Succeeded())
	{
		SetStaticMesh(PlaneShape.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> DefaultSpriteMat(TEXT("/Game/SpriteMaterial/Materials/SpriteGlobal_Mat"));
	if (DefaultSpriteMat.Succeeded())
	{
		m_sourceMaterial = DefaultSpriteMat.Object;
	}

	m_textureSetupName = FName("TextureSetup");
	m_sourceTextureName = FName("SourceTexture");
	m_colourModName = FName("ColorMod");
	m_currFrameName = FName("CurrFrame");

	m_isPlayingLeftAnim = false;

	m_doDepthSort = true;

}

void UAnimatedPlaneComponent::OnRegister()
{
	Super::OnRegister();

	SetupAnimation();
}


void UAnimatedPlaneComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetupAnimation();
}

void UAnimatedPlaneComponent::SetNewAnimation(const FAnimationSetup &newSetup, bool playLeft)
{
	if (m_isPlayingLeftAnim == playLeft && newSetup == m_animSetup)
	{
		return;
	}

	m_isPlayingLeftAnim = playLeft;
	m_animSetup = newSetup;

	SetupAnimation();
}

void UAnimatedPlaneComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	m_currentFrameTime += DeltaTime;
	
	check(m_currentTimePerFrame > 0.0f);
	if (m_currentFrameTime >= m_currentTimePerFrame)
	{
		int32 startFrame = (m_isPlayingLeftAnim) ? m_animSetup.m_animStartLeft : m_animSetup.m_animStartRight;
		int32 lastFrame = startFrame + m_animSetup.m_numFrames-1;

		while (m_currentFrameTime >= m_currentTimePerFrame)
		{
			m_currentFrameTime -= m_currentTimePerFrame;
			m_currentFrame++;

			if (m_currentFrame > lastFrame)
			{
				m_currentFrame = startFrame;
			}
		}

		PushCurrentFrame();
	}
}

void UAnimatedPlaneComponent::SetupAnimation()
{
	m_MIDs.Reset();
		
	for (int i = 0; i < GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic *MID = CreateDynamicMaterialInstance(i, m_sourceMaterial);

		if (MID)
		{
			MID->SetVectorParameterValue(m_textureSetupName, FLinearColor(m_animSetup.m_numFramesX, m_animSetup.m_numFramesY, m_animSetup.m_texResolution.X, m_animSetup.m_texResolution.Y));
			MID->SetTextureParameterValue(m_sourceTextureName, m_animSetup.m_sourceTexture);
			MID->SetVectorParameterValue(m_colourModName, m_animSetup.m_colorMod);

			m_MIDs.Add(MID);
		}
	}

	if (m_animSetup.m_numFrames > 1 && m_animSetup.m_animSpeed > 0.0f)
	{
		// setup for animation
		m_currentTimePerFrame = 1.0f / m_animSetup.m_animSpeed;

		check(m_currentTimePerFrame > 0.0f);

		m_currentFrameTime = FMath::FRandRange(0.0f, m_currentTimePerFrame);
		int32 startFrame = (m_isPlayingLeftAnim) ? m_animSetup.m_animStartLeft : m_animSetup.m_animStartRight;
		m_currentFrame = FMath::RandRange(startFrame, startFrame + m_animSetup.m_numFrames-1);
		
		Activate();
	}
	else
	{
		// setup for single frame
		m_currentFrame = (m_isPlayingLeftAnim) ? m_animSetup.m_animStartLeft : m_animSetup.m_animStartRight;

		Deactivate();
	}

	PushCurrentFrame();
}

void UAnimatedPlaneComponent::PushCurrentFrame()
{
	for (int i = 0; i < m_MIDs.Num(); i++)
	{
		UMaterialInstanceDynamic *MID = m_MIDs[i];

		if (MID)
		{
			MID->SetScalarParameterValue(m_currFrameName, m_currentFrame);
		}
	}

}

bool operator==(const FAnimationSetup& LHS, const FAnimationSetup& RHS)
{
	return LHS.m_animSpeed == RHS.m_animSpeed &&
		LHS.m_animStartLeft == RHS.m_animStartLeft &&
		LHS.m_animStartRight == RHS.m_animStartRight &&
		LHS.m_numFrames == RHS.m_numFrames &&
		LHS.m_numFramesX == RHS.m_numFramesX &&
		LHS.m_numFramesY == RHS.m_numFramesY &&
		LHS.m_sourceTexture == RHS.m_sourceTexture &&
		LHS.m_texResolution == RHS.m_texResolution &&
		LHS.m_colorMod == RHS.m_colorMod;
}
