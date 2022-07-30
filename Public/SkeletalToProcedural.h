// Nick Chalvatzakis


#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshLODRenderData.h"

#include "MySkeletalMeshComponent.generated.h"


UCLASS()
class ANYPROJECT_API UMySkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

		void SliceSkeletalMeshComponent(int32 LODIndex, FVector PlanePosition, FVector PlaneNormal, bool bCreateOtherHalf, USkeletalMeshComponent*& OutOtherHalfSkelMesh, UMaterialInterface* CapMaterial);
	
};

