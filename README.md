# Afterimage
언리얼 엔진에서 잔상 시스템을 구현한 C++ 프로젝트.
 
## Preview
![Afterimage](https://github.com/poi001/Afterimage/assets/107660181/38d3e683-f52e-46dc-badb-ac403cee0682)

## 기능 구현
### 언리얼 엔진
1. 
### 코드
특수한 키를 누르면 잔상효과가 발동되는 기능으로 구현하였다. ( 이 프로젝트에서는 숫자 1번 키로 설정해놓음 )

아래 코드는 잔상 구현을 담을 Actor클래스의 cpp파일에서 Init함수의 코드이다. ( AAfterimage_.cpp )

```
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
```

* `PoseableMesh` : 메쉬의 포즈정보만 복사해오는 컴포넌트이다. 이 변수에 플레이어 캐릭터의 SkeletalMesh 포즈를 복제한다.
* `TArray<UMaterialInterface*> Mats` : 이 배열은 for문을 알맞게 쓰기 위해 임의로 만든 UMaterialInterface자료형의 배열이다. `PoseableMesh`의 머티리얼을 담았다.
* `Materials` : 잔상 머티리얼 인스턴스를 담을 배열이다. 각 원소마다 AfterImageMaterials를 할당하였다.
* `AfterImageMaterials` : 효과를 주기위한 머티리얼이다. 림라이트이다. ( 림라이트:피사체 뒤에서 강한 조명을 주는 것 )
* `PoseableMesh->SetMaterial(i, Materials[i])` : `PoseableMesh`의 머티리얼을 `Materials`로 설정한다.
* `FadeOutTime` : 잔상이 사라질 시간을 담을 Float형 변수이다.
* `FadeCountDown` : 잔상이 몇 초동안 남아있는지 재는 Float형 변수이다.
* `IsSpawned` : 잔상을 소환할 것인지에 대한 Bool형 변수이다.

위에 Init함수는 잔상의 처음 초기화를 시킬 때 호출하는 함수로 쓰일 것이다.

아래 코드는 잔상의 코드의 Tick부분이다. ( AAfterimage_.cpp )

```
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
```

* `if (IsSpawned)` : IsSpawned이 true일 때만 실행되도록 하였다.
* `FadeCountDown -= DeltaTime` : FadeCountDown를 DeltaTime만큼 Tick마다 감소시켜준다. ( 0.5f로 설정하였다 )
* `for (int i = 0; i < Materials.Num(); i++)` : `Materials`의 배열 원소만큼(머티리얼 인터페이스) 반복문을 돌린다.
* `Materials[i]->SetScalarParameterValue("Opacity", FadeCountDown / FadeOutTime)` : 
