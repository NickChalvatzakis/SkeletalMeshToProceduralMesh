// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "VectorTypes.h"
#include "AnyProject.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTVRODUS_API USkeletalToProcedural : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkeletalToProcedural();


public:	

	UFUNCTION(BlueprintCallable)
	void SkeletalMeshToProcedural(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProceduralMeshComponent, bool bCreateCollision);

		
};
