// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeBase.h"
#include "SnakeElementBase.h"
#include "Interactable.h"
#include "Food.h"

// Sets default values
ASnakeBase::ASnakeBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ElementSize = 100.f;
	MovementSpeed = 10.f;
	LastMoveDirection = EMovementDirection::UP;
	bCanChangeDirection = true;
}

// Called when the game starts or when spawned
void ASnakeBase::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickInterval(MovementSpeed);
	AddSnakeElement(2);
}

// Called every frame
void ASnakeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Move();
}

void ASnakeBase::AddSnakeElement(int ElementsNum)
{
	for (int i = 0; i < ElementsNum; ++i)
	{
		FVector NewLocation;

		if (SnakeElements.Num() > 0)
		{
			ASnakeElementBase* LastElement = SnakeElements.Last();
			FVector LastLocation = LastElement->GetActorLocation();

			switch (LastMoveDirection)
			{
			case EMovementDirection::UP:
				NewLocation = LastLocation - FVector(ElementSize, 0, 0);
				break;
			case EMovementDirection::DOWN:
				NewLocation = LastLocation + FVector(ElementSize, 0, 0);
				break;
			case EMovementDirection::LEFT:
				NewLocation = LastLocation - FVector(0, ElementSize, 0);
				break;
			case EMovementDirection::RIGHT:
				NewLocation = LastLocation + FVector(0, ElementSize, 0);
				break;
			default:
				NewLocation = LastLocation;
				break;
			}
		}
		else
		{
			NewLocation = FVector(0, 0, 0);
		}

		FTransform NewTransform(NewLocation);
		ASnakeElementBase* NewSnakeElem = GetWorld()->SpawnActor<ASnakeElementBase>(SnakeElementClass, NewTransform);
		NewSnakeElem->SnakeOwner = this;
		int32 ElementIndex = SnakeElements.Add(NewSnakeElem);
		if (ElementIndex == 0)
		{
			NewSnakeElem->SetFirstElementType();
		}
	}
}

void ASnakeBase::Move()
{
	FVector MovementVector(FVector::ZeroVector);

	switch (LastMoveDirection)
	{
	case EMovementDirection::UP:
		MovementVector.X += ElementSize;
		break;
	case EMovementDirection::DOWN:
		MovementVector.X -= ElementSize;
		break;
	case EMovementDirection::LEFT:
		MovementVector.Y += ElementSize;
		break;
	case EMovementDirection::RIGHT:
		MovementVector.Y -= ElementSize;
		break;
	}

	SnakeElements[0]->ToggleCollision();

	for (int i = SnakeElements.Num() - 1; i > 0; i--)
	{
		auto CurrentElement = SnakeElements[i];
		auto PrevElement = SnakeElements[i - 1];
		FVector PrevLocation = PrevElement->GetActorLocation();
		CurrentElement->SetActorLocation(PrevLocation);
	}

	SnakeElements[0]->AddActorWorldOffset(MovementVector);
	SnakeElements[0]->ToggleCollision();

	// Allow direction change for the next tick
	bCanChangeDirection = true;
}

void ASnakeBase::ChangeDirection(EMovementDirection NewDirection)
{
	if (bCanChangeDirection)
	{
		switch (NewDirection)
		{
		case EMovementDirection::UP:
			if (LastMoveDirection != EMovementDirection::DOWN)
			{
				LastMoveDirection = NewDirection;
				bCanChangeDirection = false;
			}
			break;
		case EMovementDirection::DOWN:
			if (LastMoveDirection != EMovementDirection::UP)
			{
				LastMoveDirection = NewDirection;
				bCanChangeDirection = false;
			}
			break;
		case EMovementDirection::LEFT:
			if (LastMoveDirection != EMovementDirection::RIGHT)
			{
				LastMoveDirection = NewDirection;
				bCanChangeDirection = false;
			}
			break;
		case EMovementDirection::RIGHT:
			if (LastMoveDirection != EMovementDirection::LEFT)
			{
				LastMoveDirection = NewDirection;
				bCanChangeDirection = false;
			}
			break;
		}
	}
}

void ASnakeBase::SnakeElementOverlap(ASnakeElementBase* OverlappedElement, AActor* Other)
{
	if (IsValid(OverlappedElement))
	{
		int32 ElemIndex;
		SnakeElements.Find(OverlappedElement, ElemIndex);
		bool bIsFirst = ElemIndex == 0;
		IInteractable* IInteractableInterface = Cast<IInteractable>(Other);
		if (IInteractableInterface)
		{
			IInteractableInterface->Interact(this, bIsFirst);
			if (Other->IsA<AFood>())
			{
				MovementSpeed /= FMath::FRandRange(1.3f, 1.6f);
				SetActorTickInterval(MovementSpeed);
			}
		}
	}
}
