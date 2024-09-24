// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "opencv2/opencv.hpp"
#include "vector"
#include "VideoCaptureTest.generated.h"

UCLASS()
class UEROBOTOJJUDGER_API AVideoCaptureTest : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AVideoCaptureTest();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	cv::Mat VideoStream;
	cv::Mat VideoStreamCorrection;
	std::vector<cv::Point2f> points;
	static FWindowsCriticalSection FrameLock;
	static FWindowsCriticalSection PointsLock;
	FString ProjectDir=FPaths::ProjectDir();
	//蓝图函数
	UFUNCTION(BlueprintCallable)
	void SetPoints();
};

class FVideoCapture:public FRunnable
{
public:
	FVideoCapture(FString ThreadName,class AVideoCaptureTest* VideoCapture): MyThreadName(ThreadName),VideoCapture(VideoCapture)
	{
		
	}
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	
	void OpenVideo();
	FString MyThreadName;
	class AVideoCaptureTest* VideoCapture;//指向主线程的指针
private:
	cv::VideoCapture cap;
	FString VideoDir;
};

class FVideoCorrection:public FRunnable
{
public:
	FVideoCorrection(FString ThreadName,class AVideoCaptureTest* VideoCapture): MyThreadName(ThreadName),VideoCapture(VideoCapture)
	{
		
	}
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	void OpenVideo();
	void applyPerspectiveTransform(const cv::Mat& src, cv::Mat& dst, const cv::Point2f vertices[4]);
	FString MyThreadName;
	cv::RotatedRect maxBox; // 历史最大矩形
	class AVideoCaptureTest* VideoCapture;//指向主线程的指针
private:
	FString OnnxDir;
};
