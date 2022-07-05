// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalToProcedural.h"

// Sets default values for this component's properties
USkeletalToProcedural::USkeletalToProcedural()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}



void USkeletalToProcedural::SkeletalMeshToProcedural(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProceduralMeshComponent, bool bCreateCollision) {


	double start = FPlatformTime::Seconds();

	FSkeletalMeshRenderData* SkMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
	const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[LODIndex];
	FSkinWeightVertexBuffer& SkinWeights = *SkeletalMeshComponent->GetSkinWeightBuffer(LODIndex);

    TMap<int32, int32> MeshToSectionVertMap;
	TArray<FVector> VerticesArray;
	TArray<FVector> Normals;
	TArray<FVector2D> UV;
	TArray<int32> Tris;
	TArray<FColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	int32 SectionCount = DataArray.RenderSections.Num();
	int32 NumSourceVertices = 0;

	for (int32 j = 0; j < SectionCount; j++) {
		//get num vertices
		NumSourceVertices = DataArray.RenderSections[j].NumVertices;

		const FSkelMeshRenderSection& ThisSection = DataArray.RenderSections[j];
		
		uint32 LastIndex = ThisSection.BaseIndex + ThisSection.NumTriangles * 3;

		for (uint32 i = ThisSection.BaseIndex; i < LastIndex; i++) {
			uint32 MeshVertexIndex = DataArray.MultiSizeIndexContainer.GetIndexBuffer()->Get(i);
			int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertexIndex);
			int32 NewIndex = -1;
			if (NewIndexPtr) {
				NewIndex = *NewIndexPtr;
				Tris.Add(NewIndex);
			}
			else {
				FVector3f SkinnedVectorPos = USkeletalMeshComponent::GetSkinnedVertexPosition(SkeletalMeshComponent, MeshVertexIndex, DataArray, SkinWeights);
				int SectVertIdx = VerticesArray.Add(FVector(SkinnedVectorPos));

				//Calculate normals and tanges from the static version instead of the skeletal mesh
				FVector4f ZTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(MeshVertexIndex);
				FVector4f XTangentStatic = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(MeshVertexIndex);

				//add normals from static mesh
				Normals.Add(FVector(ZTangentStatic));

				Tangents.Add(FProcMeshTangent(FVector(XTangentStatic), false));

				FVector2f uvs = DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(MeshVertexIndex, 0);
				UV.Add(FVector2D(uvs));

				Colors.Add(FColor(0, 0, 0, 255));

				MeshToSectionVertMap.Add(MeshVertexIndex, SectVertIdx);
				Tris.Add(SectVertIdx);

			}
			

		}
	}


	//Create proc mesh
	ProceduralMeshComponent->CreateMeshSection(0, VerticesArray, Tris, Normals, UV, Colors, Tangents, bCreateCollision);

    for (int32 MatIndex = 0; MatIndex < SkeletalMeshComponent->GetNumMaterials(); MatIndex++)
    {
        ProceduralMeshComponent->SetMaterial(MatIndex, SkeletalMeshComponent->GetMaterial(MatIndex));
    }



    ProceduralMeshComponent->ClearCollisionConvexMeshes();

    if (SkeletalMeshComponent->BodySetup != nullptr)
    {
        // Iterate over all convex hulls on static mesh..
        const int32 NumConvex = SkeletalMeshComponent->BodySetup->AggGeom.ConvexElems.Num();
        for (int ConvexIndex = 0; ConvexIndex < NumConvex; ConvexIndex++)
        {
            // Copy convex verts to ProcMesh
            FKConvexElem& MeshConvex = SkeletalMeshComponent->BodySetup->AggGeom.ConvexElems[ConvexIndex];
            ProceduralMeshComponent->AddCollisionConvexMesh(MeshConvex.VertexData);
        }
    }

	double end = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("code executed in %f seconds."), end - start);



}

