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
	//�ٲ��� ��Ƽ���� ȿ���� �ҷ��´�.
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
			//�ð��� �������� Opacity�� ��ġ�� ���������Ѵ�.
			//Opacity�� ���͸��� ������ ScalarParameter�� �̸��̴�.
			Materials[i]->SetScalarParameterValue("Opacity", FadeCountDown / FadeOutTime);
		}
		// ������ 0�� �Ǿ������ ����
		if (FadeCountDown < 0)
		{
			Destroy();
		}
	}
}

void AAfterimage_::Init(USkeletalMeshComponent* Pawn)
{
	//������ ��û�� ��ü�� SkeletalMesh ��� �����Ѵ�.
	PoseableMesh->CopyPoseFromSkeletalComponent(Pawn);

	//��Ƽ���� ������ �������
	TArray<UMaterialInterface*> Mats = PoseableMesh->GetMaterials();

	for (int i = 0; i < Mats.Num(); i++)
	{
		//������ŭ ���Ƿ� �߰��� ��Ƽ������ �����Ѵ�.
		Materials.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), AfterImageMaterials));
		//������ ��Ƽ������ �޽��� ����
		PoseableMesh->SetMaterial(i, Materials[i]);
	}
	//��ġ�� �������� ������ ������
	FadeOutTime = 0.5f;
	FadeCountDown = FadeOutTime;
	IsSpawned = true;
}