// Fill out your copyright notice in the Description page of Project Settings.


#include "Afterimage_.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"

// Sets default values
AAfterimage_::AAfterimage_()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("POSEABLEMESH"));
	RootComponent = PoseableMesh;

	ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_PoseMesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
	if (SK_PoseMesh.Succeeded())
	{
		PoseableMesh->SetSkeletalMesh(SK_PoseMesh.Object);
	}
	//바꿔줄 머티리얼 효과를 불러온다.
	ConstructorHelpers::FObjectFinder<UMaterial> M_GhostTail(TEXT("/Game/Material/AfterimageMat.AfterimageMat"));
	if (M_GhostTail.Succeeded())
	{
		AfterImageMaterials = M_GhostTail.Object;
	}
}

// Called when the game starts or when spawned
/*void AAfterimage_::BeginPlay()
{
	Super::BeginPlay();
	
}*/

// Called every frame
void AAfterimage_::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsSpawned)
	{
		FadeCountDown -= DeltaTime;
		for (int i = 0; i < Materials.Num(); i++)
		{
			//시간이 지날수록 Opacity의 수치가 낮아져야한다.
			//Opacity는 머터리얼에 생성한 ScalarParameter의 이름이다.
			Materials[i]->SetScalarParameterValue("Opacity", FadeCountDown / FadeOutTime);
		}
		// 투명도가 0이 되어버리면 삭제
		if (FadeCountDown < 0)
		{
			Destroy();
		}
	}
}

void AAfterimage_::Init(USkeletalMeshComponent* Pawn)
{
	//생성을 요청한 객체의 SkeletalMesh 포즈를 복제한다.
	PoseableMesh->CopyPoseFromSkeletalComponent(Pawn);

	//머티리얼 개수를 얻기위해
	TArray<UMaterialInterface*> Mats = PoseableMesh->GetMaterials();

	for (int i = 0; i < Mats.Num(); i++)
	{
		//개수만큼 임의로 추가한 머티리얼을 복제한다.
		Materials.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), AfterImageMaterials));
		//복제한 머티리얼을 메쉬에 적용
		PoseableMesh->SetMaterial(i, Materials[i]);
	}
	//수치가 적을수록 빠르게 삭제됨
	FadeOutTime = 0.5f;
	FadeCountDown = FadeOutTime;
	IsSpawned = true;
}