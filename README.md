# Afterimage
언리얼 엔진에서 잔상 시스템을 구현한 언리얼 C++ 프로젝트.
 
## Preview
![Afterimage](https://github.com/poi001/Afterimage/assets/107660181/38d3e683-f52e-46dc-badb-ac403cee0682)

## 기능 구현
### 언리얼 엔진
우선 코드에 들어가기 앞서 언리얼 엔진에서 두 가지를 만들고 가야 한다.

1. 머티리얼 생성
2. 커브 생성

#### 머티리얼 생성
![머티리얼_만들기](https://github.com/poi001/Afterimage/assets/107660181/ffcbdb92-35e1-4e52-ab92-821d579f58ff)

컨텐츠 브라우저에서 우클릭하여 머티리얼을 생성한다.

![머티리얼_전체샷](https://github.com/poi001/Afterimage/assets/107660181/c96cb210-affb-4ae8-be36-f3e5aad7a70b)

머티리얼 기본 베이스 색은 파랑색(Constant4Vector)으로 설정하였고 림라이트(림라이트:피사체 뒤에서 강한 조명을 주는 것)로 설정하여 외각선부분이 더 진하게 빛나도록 설정했다. Opacity(Scalar Parameter)를 사용하여 잔상의 투먕도를 설정하였다.

여기서 Fresnel노드로 머티리얼의 설정을 조정했는데 Fresnel이란 관찰자가 바라보는 각도에 따라 반사되는 빛의 세기가 달라지는 현상을 설명하는 데 사용된다. 예를 들어 물 웅덩이 위에 서서 수직으로 내려다보는 경우, 반사되는 수면이 많이 보이지 않을 것이다. 머리를 움직여 물 웅덩이의 수면이 시선과 평행이 되어갈수록, 수면의 반사면이 많아지는 것이 보일 것이다.

* `ExponentIn` : 프레넬 이펙트 감쇠를 제어합니다.
* `BaseReflectFrctionIn` : 표면을 직접 봤을 때의 스페큘러 리플렉션의 굴절율을 나타냅니다. 이 값을 1 로 설정하면 사실상 프레넬이 꺼집니다.

![image](https://github.com/poi001/Afterimage/assets/107660181/103f12c2-3d2a-4bd4-bc77-43075d15ab61)
디테일창에서 해당 체크리스트를 체크해놔야지 오류가 나지 않는다.

위 설정대로 머티리얼을 구현해놓는다.
#### 커브 생성
Timeline코드에서 사용할 커브를 생성한다.

![커브_만들기](https://github.com/poi001/Afterimage/assets/107660181/3e889a5f-5e2f-4008-bb5a-745612ffb059)

컨텐츠 브라우저에서 우클릭하여 커브를 생성한다.

![커브_키만들기](https://github.com/poi001/Afterimage/assets/107660181/9890141c-fdb0-4a85-a477-d0b8f05a49cd)

![image](https://github.com/poi001/Afterimage/assets/107660181/c3f1aa82-59bb-4df7-823c-0041478c37be)

키를 두개를 만들어 하나는 0,0 또 하나는 0.1, 0.1로 만든다. ( 틱마다 0.1씩 늘어나고 0.1에 다다르면 잔상이 소환되는 코드를 구현할 것이다 )

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

다음은 잔상이 소환됐을 시, 구현을 할 Tick함수이다.( AAfterimage_.cpp )

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

다음은 플레이어 캐릭터 BeginPlay에 넣을 타임라인 관련 코드이다. ( AfterimageCharacter.cpp )

```
//커브float가 있다면
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;					//타임라인 생성
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));	//해당 함수를 이어준다
		FOnTimelineEvent FinishTimeLine;					//타임라인 이벤트 생성
		FinishTimeLine.BindUFunction(this, FName("FinishTimeLine"));		//해당 함수를 이어준다.
		CurveTimeLine.AddInterpFloat(CurveFloat, TimelineProgress);		//CurveFloat값을 매개변수로 해서 받아낸다.
		CurveTimeLine.SetTimelineFinishedFunc(FinishTimeLine);	//타임라인이 끝나면 FinishTimeLine에 연결된 FinishTimeLine함수 실행
		CurveTimeLine.SetLooping(false);					//루프 하지 말고 끝났을때 리셋시키게
	}
```

다음은 플레이어 캐릭터 코드에서 타임라인에 관련된 사용자 정의함수이다. ( AfterimageCharacter.cpp )

```
void AAfterimageCharacter::TimelineProgress(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("%f"), Value);
}

void AAfterimageCharacter::FinishTimeLine()
{
	FActorSpawnParameters SpawnParams;			//SpawnActor 함수에 전달되는 선택적 매개 변수의 구조체
	SpawnParams.Owner = this;					//소환될 액터의 소유를 플레이어 캐릭터로 설정

	FTransform transform = GetMesh()->GetComponentTransform();	//플레이어 캐릭터의 트랜스폼

	//월드상에서 액터 스폰을 요청한다.(액터 종류,액터 방향,회전,액터의 주인)
	auto GTrail = Cast<AAfterimage_>(GetWorld()->SpawnActor<AActor>(AAfterimage_::StaticClass(), transform, SpawnParams));
	if (GTrail)
	{
		GTrail->Init(GetMesh());	//소환될 잔상의 설정 초기화
	}

	//처음이라면
	if (CurveTimeLine.GetPlaybackPosition() == 0.0f)
		CurveTimeLine.Play();		//타임라인 시작
	//끝났을때
	else if (CurveTimeLine.GetPlaybackPosition() == CurveTimeLine.GetTimelineLength())
		CurveTimeLine.Reverse();	//타임라인 역재생
}
```

타임라인 사용 방법은 타임라인이 끝날 때, 실행되는 함수를 델리게이트한다. 그 델리게이트된 함수의 내용은 잔상을 소환하고 타임라인이 끝났을 때는 다시 재생하는 코드이다.
