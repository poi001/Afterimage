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
	class UPoseableMeshComponent* PoseableMesh;			//메쉬의 포즈정보만 복사해오는 컴포넌트
	class UMaterial* AfterImageMaterials;				//효과를 주기위한 머티리얼(림라이트를 쓰기로했다) 림라이트:피사체 뒤의 빛
	TArray<class UMaterialInstanceDynamic*> Materials;	//잔상 머티리얼 인스턴스 배열

	bool IsSpawned = false;			//잔상이 스폰됐는지 변수
	float FadeCountDown = 0.0f;		//시간을 잴 변수
	float FadeOutTime = 0.0f;		//사라질 시간
};
