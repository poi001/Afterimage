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

	//Ŀ�� ����
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

	//Ŀ��float�� �ִٸ�
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;									//Ÿ�Ӷ��� ����
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));	//�ش� �Լ��� �̾��ش�
		FOnTimelineEvent FinishTimeLine;									//Ÿ�Ӷ��� �̺�Ʈ ����
		FinishTimeLine.BindUFunction(this, FName("FinishTimeLine"));		//�ش� �Լ��� �̾��ش�.
		CurveTimeLine.AddInterpFloat(CurveFloat, TimelineProgress);			//CurveFloat���� �Ű������� �ؼ� �޾Ƴ���.
		CurveTimeLine.SetTimelineFinishedFunc(FinishTimeLine);	//Ÿ�Ӷ����� ������ FinishTimeLine�� ����� FinishTimeLine�Լ� ����
		CurveTimeLine.SetLooping(false);									//���� ���� ���� �������� ���½�Ű��
	}
}

void AAfterimageCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OnAfterimage)	//�ܻ� Ű�� ������ ��
	{
		CurveTimeLine.TickTimeline(DeltaTime);		//Ÿ�Ӷ����� ƽ������ ���� ��Ų��
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

	//�ܻ� Ű�� ������ ��
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
		CurveTimeLine.PlayFromStart();		//Ÿ�Ӷ��� ����
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
	FActorSpawnParameters SpawnParams;			//SpawnActor �Լ��� ���޵Ǵ� ������ �Ű� ������ ����ü
	SpawnParams.Owner = this;					//��ȯ�� ������ ������ �÷��̾� ĳ���ͷ� ����

	FTransform transform = GetMesh()->GetComponentTransform();	//�÷��̾� ĳ������ Ʈ������

	//����󿡼� ���� ������ ��û�Ѵ�.(���� ����,���� ����,ȸ��,������ ����)
	auto GTrail = Cast<AAfterimage_>(GetWorld()->SpawnActor<AActor>(AAfterimage_::StaticClass(), transform, SpawnParams));
	if (GTrail)
	{
		GTrail->Init(GetMesh());	//��ȯ�� �ܻ��� ���� �ʱ�ȭ
	}

	//ó���̶��
	if (CurveTimeLine.GetPlaybackPosition() == 0.0f)
		CurveTimeLine.Play();		//Ÿ�Ӷ��� ����
	//��������
	else if (CurveTimeLine.GetPlaybackPosition() == CurveTimeLine.GetTimelineLength())
		CurveTimeLine.Reverse();	//Ÿ�Ӷ��� �����
}