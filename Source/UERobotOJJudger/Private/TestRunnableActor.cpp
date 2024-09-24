// Fill out your copyright notice in the Description page of Project Settings.


#include "TestRunnableActor.h"

#include <iostream>

// Sets default values
ATestRunnableActor::ATestRunnableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestRunnableActor::BeginPlay()
{
	Super::BeginPlay();
	FTestRunnable* Runnable1=new FTestRunnable(TEXT("线程1"),this);
	FTestRunnable* Runnable2=new FTestRunnable(TEXT("线程2"),this);
	FRunnableThread* RunnableThread1=FRunnableThread::Create(Runnable1,*Runnable1->MyThreadName);
	FRunnableThread* RunnableThread2=FRunnableThread::Create(Runnable2,*Runnable2->MyThreadName);
}

// Called every frame
void ATestRunnableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

uint32 FTestRunnable::Run()
{
	while(IsValid(TestActor))
	{
#if true //thread sync 线程同步
		FScopeLock Lock(&CriticalSection);
#endif
		if(TestActor->TestCount<TestActor->TestTarget)
		{
			TestActor->TestCount++;
			std::cout<<"Thread "<<&MyThreadName<<" is running,TestCount="<<TestActor->TestCount<<std::endl;
		}
		else
		{
			break;
		}
	}
	return 0;
}
bool FTestRunnable::Init()
{
	//打印log
	std::cout<<"Thread "<<&MyThreadName<<" is starting"<<std::endl;
	TestActor->TestCount=0;
	return true;
}
void FTestRunnable::Exit()
{
	std::cout<<"Thread "<<&MyThreadName<<" is exiting"<<std::endl;
}
FWindowsCriticalSection FTestRunnable::CriticalSection;//定义静态变量


