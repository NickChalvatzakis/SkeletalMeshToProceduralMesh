// Nick Chalvatzakis


#include "SkeletalToProcedural.h"
#include "ProceduralMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/ConvexElem.h"

// Sets default values for this component's properties
USkeletalToProcedural::USkeletalToProcedural()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

static int32 GetNewIndexForOldVertIndex(FVector Vertex,int32 MeshVertexIndex, TMap<int32, int32>& MeshToSectionVertMap, const FStaticMeshVertexBuffers& VertexBuffers, TArray<FVector>& Vertices, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents){
	int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertexIndex);
	if (NewIndexPtr != nullptr) {

		return *NewIndexPtr;
	}
	else {
		//int32 SectionVertIndex = Vertices.Add((FVector)VertexBuffers.PositionVertexBuffer.VertexPosition(MeshVertexIndex));
		int32 SectionVertIndex = Vertices.Add(Vertex);

		Normals.Add(FVector4(VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(MeshVertexIndex)));
		check(Normals.Num() == Vertices.Num());

		UVs.Add(FVector2D(VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(MeshVertexIndex, 0)));
		check(UVs.Num() == Vertices.Num());


		FVector4 TangentX = (FVector4)VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(MeshVertexIndex);
		FProcMeshTangent NewTangent(TangentX, TangentX.W < 0.f);
		Tangents.Add(NewTangent);
		check(Tangents.Num() == Vertices.Num());

		MeshToSectionVertMap.Add(MeshVertexIndex, SectionVertIndex);

		return SectionVertIndex;
	}
}
void USkeletalToProcedural::GetSectionFromSkeletalMesh(USkeletalMeshComponent* SkeletalMeshComponent,FSkinWeightVertexBuffer& SkinWeights,FSkeletalMeshRenderData* RenderData, int32 LODIndex, int32 SectionIndex, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals, TArray<FVector2D>& UVs, TArray<FProcMeshTangent>& Tangents){

	if (RenderData != nullptr && RenderData->LODRenderData.IsValidIndex(LODIndex)) {
		const FSkeletalMeshLODRenderData& LOD = RenderData->LODRenderData[LODIndex];
		

		if (LOD.RenderSections.IsValidIndex(SectionIndex)) {
			Vertices.Reset();
			Triangles.Reset();
			Normals.Reset();
			UVs.Reset();
			Tangents.Reset();


			TMap<int32, int32> MeshToSectionVertMap;

			const FSkelMeshRenderSection& Section = LOD.RenderSections[SectionIndex];
			const uint32 OnePastLastIndex = Section.BaseIndex + Section.NumTriangles * 3;
			for (uint32 i = Section.BaseIndex; i < OnePastLastIndex; i++) {
				uint32 MeshVertexIndex = LOD.MultiSizeIndexContainer.GetIndexBuffer()->Get(i);
				FVector SkinnedVectorPosition = (FVector)USkeletalMeshComponent::GetSkinnedVertexPosition(SkeletalMeshComponent, MeshVertexIndex, LOD, SkinWeights);
				int32 SectionVertIndex = GetNewIndexForOldVertIndex(SkinnedVectorPosition, MeshVertexIndex, MeshToSectionVertMap, LOD.StaticVertexBuffers, Vertices, Normals, UVs, Tangents);
				Triangles.Add(SectionVertIndex);
			}
		}
	}

}

void USkeletalToProcedural::NewCopyFromSkel(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProcMeshComponent, bool bCreateCollision, bool bAverageConvex, int AverageModifier) {

	double start = FPlatformTime::Seconds();
	FSkeletalMeshRenderData* SkMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
	int32 NumSections = SkMeshRenderData->LODRenderData[LODIndex].RenderSections.Num();
	FSkinWeightVertexBuffer& SkinWeights = *SkeletalMeshComponent->GetSkinWeightBuffer(LODIndex);
	TArray<FVector> ConvexArray;
	ProcMeshComponent->ClearCollisionConvexMeshes();
	ProcMeshComponent->SetSimulatePhysics(true);

	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++) {
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;
		TArray<FVector2D> UVs1;
		TArray<FVector2D> UVs2;
		TArray<FVector2D> UVs3;
		TArray<FProcMeshTangent> Tangents;

		GetSectionFromSkeletalMesh(SkeletalMeshComponent, SkinWeights, SkMeshRenderData, LODIndex, SectionIndex, Vertices, Triangles, Normals, UVs, Tangents);
		if (bCreateCollision) {
			if (!bAverageConvex)
				ConvexArray = Vertices;
			else {
				FVector Point = FVector::Zero();
				for (int i = 0; i < Vertices.Num(); i++) {
					Point += Vertices[i]/AverageModifier;
					if (i % AverageModifier == 0) {
						ConvexArray.Add(Point);
						Point = FVector::Zero();
					}

				}
			}
		}
		
		TArray<FLinearColor> DummyColors;
		ProcMeshComponent->CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, Normals, UVs, UVs1, UVs2, UVs3, DummyColors, Tangents, false);
	
	}

	if (bCreateCollision)
		ProcMeshComponent->AddCollisionConvexMesh(ConvexArray);

	

	for (int32 MatIndex = 0; MatIndex < SkeletalMeshComponent->GetNumMaterials(); MatIndex++)
	{
		ProcMeshComponent->SetMaterial(MatIndex, SkeletalMeshComponent->GetMaterial(MatIndex));
	}

	double end = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("code executed in %f seconds."), end - start);

}

void USkeletalToProcedural::SkeletalMeshToProcedural(USkeletalMeshComponent* SkeletalMeshComponent, int32 LODIndex, UProceduralMeshComponent* ProceduralMeshComponent, bool bCreateCollision, bool bAverageConvex, int32 averageModifier) {


	double start = FPlatformTime::Seconds();

	FSkeletalMeshRenderData* SkMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();
	const FSkeletalMeshLODRenderData& DataArray = SkMeshRenderData->LODRenderData[LODIndex];
	FSkinWeightVertexBuffer& SkinWeights = *SkeletalMeshComponent->GetSkinWeightBuffer(LODIndex);

    TMap<int32, int32> MeshToSectionVertMap;


	int32 SectionCount = DataArray.RenderSections.Num();
	int32 NumSourceVertices = 0;

	SkeletalMeshComponent->bPauseAnims = true;


	ProceduralMeshComponent->ClearCollisionConvexMeshes();
	ProceduralMeshComponent->SetSimulatePhysics(true);
	TArray<FVector> AverageVerticesArray;
	FVector PointsInVertexSpace = FVector::Zero();


	for (int32 j = 0; j < SectionCount; j++) {


		TArray<FVector> VerticesArray;
		TArray<FVector> Normals;
		TArray<FVector2D> UV;
		TArray<FVector2D> UVs;
		TArray<FVector2D> UVs1;
		TArray<FVector2D> UVs2;
		TArray<FVector2D> UVs3;
		TArray<int32> Tris;
		TArray<FProcMeshTangent> Tangents;

		//get num vertices
		NumSourceVertices = DataArray.RenderSections[j].NumVertices;

		const FSkelMeshRenderSection& ThisSection = DataArray.RenderSections[j];
		
		uint32 LastIndex = ThisSection.BaseIndex + ThisSection.NumTriangles * 3;
	;


		for (uint32 i = ThisSection.BaseIndex; i < LastIndex; i++) {
			uint32 MeshVertexIndex = DataArray.MultiSizeIndexContainer.GetIndexBuffer()->Get(i);
			int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertexIndex);
			int32 NewIndex = -1;
			if (NewIndexPtr) {
				NewIndex = *NewIndexPtr;
				Tris.Add(NewIndex);
			}
			else {				
				int SectVertIdx = VerticesArray.Add(FVector(USkeletalMeshComponent::GetSkinnedVertexPosition(SkeletalMeshComponent, MeshVertexIndex, DataArray, SkinWeights)));

				

				if (bCreateCollision) {
					if (bAverageConvex)
						PointsInVertexSpace += VerticesArray[SectVertIdx];
						if (SectVertIdx % averageModifier == 0) {
							PointsInVertexSpace /= averageModifier;

							AverageVerticesArray.Add(PointsInVertexSpace);
							PointsInVertexSpace = FVector::Zero();
						}
						else {
							AverageVerticesArray.Add(VerticesArray[SectVertIdx]);
						}
					
				}
				

				//add normals from static mesh
				Normals.Add(FVector4(DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(MeshVertexIndex)));
				check(Normals.Num() == VerticesArray.Num());

				UV.Add(FVector2D(DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(MeshVertexIndex, 0)));
				check(UV.Num() == VerticesArray.Num());

				FVector4 TangentX = (FVector4)DataArray.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(MeshVertexIndex);
				FProcMeshTangent NewTangent(TangentX, TangentX.W < 0.f);

				Tangents.Add(NewTangent);
				check(Tangents.Num() == VerticesArray.Num());


				MeshToSectionVertMap.Add(MeshVertexIndex, SectVertIdx);
				Tris.Add(SectVertIdx);
			}
			
			
	
		}
		
		// Create proc mesh
		TArray<FLinearColor> DummyColors;
		ProceduralMeshComponent->CreateMeshSection_LinearColor(j, VerticesArray, Tris, Normals, UV, UVs1, UVs2, UVs3, DummyColors, Tangents, false);
	}
	if(bCreateCollision)
	ProceduralMeshComponent->AddCollisionConvexMesh(AverageVerticesArray);
	

    for (int32 MatIndex = 0; MatIndex < SkeletalMeshComponent->GetNumMaterials(); MatIndex++)
    {
        ProceduralMeshComponent->SetMaterial(MatIndex, SkeletalMeshComponent->GetMaterial(MatIndex));
    }

	

	double end = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("code executed in %f seconds."), end - start);



}
