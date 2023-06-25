// Copyright Epic Games, Inc. All Rights Reserved.

#include "AfterimageCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Afterimage_.h"


//////////////////////////////////////////////////////////////////////////
// AAfterimageCharacter

AAfterimageCharacter::AAfterimageCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//커브 설정
	static ConstructorHelpers::FObjectFinder<UCurveFloat> CURVE_FLOAT(TEXT("/Game/Curve/FloatCurve.FloatCurve"));
	if (CURVE_FLOAT.Succeeded()) CurveFloat = CURVE_FLOAT.Object;
}

void AAfterimageCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//커브float가 있다면
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;									//타임라인 생성
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));	//해당 함수를 이어준다
		FOnTimelineEvent FinishTimeLine;									//타임라인 이벤트 생성
		FinishTimeLine.BindUFunction(this, FName("FinishTimeLine"));		//해당 함수를 이어준다.
		CurveTimeLine.AddInterpFloat(CurveFloat, TimelineProgress);			//CurveFloat값을 매개변수로 해서 받아낸다.
		CurveTimeLine.SetTimelineFinishedFunc(FinishTimeLine);	//타임라인이 끝나면 FinishTimeLine에 연결된 FinishTimeLine함수 실행
		CurveTimeLine.SetLooping(false);									//루프 하지 말고 끝났을때 리셋시키게
	}
}

void AAfterimageCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OnAfterimage)	//잔상 키를 눌렀을 때
	{
		CurveTimeLine.TickTimeline(DeltaTime);		//타임라인을 틱단위로 실행 시킨다
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAfterimageCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAfterimageCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAfterimageCharacter::Look);
	}

	//잔상 키를 눌렀을 때
	PlayerInputComponent->BindAction(TEXT("Afterimage"), EInputEvent::IE_Pressed, this, &AAfterimageCharacter::PressAfterimageKey);
}

void AAfterimageCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAfterimageCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAfterimageCharacter::PressAfterimageKey()
{
	if (OnAfterimage) OnAfterimage = false;
	else OnAfterimage = true;

	if (OnAfterimage)
	{
		CurveTimeLine.PlayFromStart();		//타임라인 시작
		UE_LOG(LogTemp, Warning, TEXT("On"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Off"));
	}
}

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