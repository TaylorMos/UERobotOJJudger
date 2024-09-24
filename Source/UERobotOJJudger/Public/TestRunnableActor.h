// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestRunnableActor.generated.h"

UCLASS()
class UEROBOTOJJUDGER_API ATestRunnableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestRunnableActor();
	int32 TestCount;
	UPROPERTY(EditAnywhere)
	int32 TestTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

class FTestRunnable : public FRunnable
{
public:
	FTestRunnable(FString ThreadName,class ATestRunnableActor* TestActor): MyThreadName(ThreadName),TestActor(TestActor)
	{
		
	}
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	FString MyThreadName;
	class ATestRunnableActor* TestActor;
private:
	int32 WorkCount=0;
	//线程临界区，用于线程加锁
	static FWindowsCriticalSection CriticalSection;
};

