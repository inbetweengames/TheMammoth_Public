// copyright 2015 inbetweengames GBR

#include "LudumGame.h"
#include "AnimatedBillboardComponent.h"

UAnimatedBillboardComponent::UAnimatedBillboardComponent()
{
	static ConstructorHelpers::FObjectFinder<UMaterial> DefaultSpriteMat(TEXT("/Game/SpriteMaterial/Materials/SpriteGlobal_Mat"));
	if (DefaultSpriteMat.Succeeded())
	{
		FMaterialSpriteElement defaultMat;
		defaultMat.Material = DefaultSpriteMat.Object;
		m_sourceSprites.Add(defaultMat);
	}

	m_animSpeedName = FName("AnimSpeed");
	m_textureSetupName = FName("TextureSetup");
	m_framesTotalName = FName("framesTotal");
	m_animStartName = FName("AnimStart");
	m_sourceTextureName = FName("SourceTexture");

	m_animSpeed = 16.0f;
	m_numFramesX = 4;
	m_numFramesY = 4;
	m_texResolution = FVector2D(1024.0f, 1024.0f);
	m_numFrames = 16;
	m_animStart = 0;
	m_sourceTexture = nullptr;
}


void UAnimatedBillboardComponent::OnRegister()
{
	Super::OnRegister();

	SetupMaterials();
}

void UAnimatedBillboardComponent::SetupMaterials()
{
	m_MIDs.Reset();
	Elements = m_sourceSprites;

	for (int i = 0; i < Elements.Num(); i++)
	{
		FMaterialSpriteElement &element = Elements[i];
		UMaterialInstanceDynamic *MID = CreateDynamicMaterialInstance(i, element.Material);

		if (MID)
		{
			MID->SetScalarParameterValue(m_animSpeedName, m_animSpeed);
			MID->SetVectorParameterValue(m_textureSetupName, FLinearColor(m_numFramesX, m_numFramesY, m_texResolution.X, m_texResolution.Y));
			MID->SetScalarParameterValue(m_framesTotalName, m_numFrames);
			MID->SetScalarParameterValue(m_animStartName, m_animStart);
			MID->SetTextureParameterValue(m_sourceTextureName, m_sourceTexture);

			m_MIDs.Add(MID);
		}
	}
}