// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Afterimage_.generated.h"

UCLASS()
class AFTERIMAGE_API AAfterimage_ : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAfterimage_();

protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Init(USkeletalMeshComponent* Pawn);

private:
	class UPoseableMeshComponent* PoseableMesh;			//�޽��� ���������� �����ؿ��� ������Ʈ
	class UMaterial* AfterImageMaterials;				//ȿ���� �ֱ����� ��Ƽ����(������Ʈ�� ������ߴ�) ������Ʈ:�ǻ�ü ���� ��
	TArray<class UMaterialInstanceDynamic*> Materials;	//�ܻ� ��Ƽ���� �ν��Ͻ� �迭

	bool IsSpawned = false;			//�ܻ��� �����ƴ��� ����
	float FadeCountDown = 0.0f;		//�ð��� �� ����
	float FadeOutTime = 0.0f;		//����� �ð�
};
