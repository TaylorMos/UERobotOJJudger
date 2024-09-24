// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include "OpencvTest.generated.h"

UCLASS()
class UEROBOTOJJUDGER_API AOpencvTest : public AActor
{
	GENERATED_BODY()

private:
	int flag = 0;
	std::mutex obs;
	std::mutex ponxx;
	std::mutex car;
	std::mutex obslocation;
	std::mutex out;
	std::mutex imglock;
	std::mutex flaglock;
	std::mutex videopieclock;
	std::mutex socketpie;
	//小车中心坐标
	cv::Point2f car_center;
	//障碍物坐标数组
	std::vector<cv::Point2f> obstacle_center;
	cv::Mat img;
	cv::Mat output_image;
	std::vector<std::vector<float>> info;
	cv::dnn::Net net;
	cv::VideoCapture capture;
	cv::RotatedRect maxBox; // 历史最大矩形
	cv::Mat obsimg;
	cv::Mat Ponxximg;
	cv::Mat outImg;
	cv::Mat NormalImg;
	
public:	
	// Sets default values for this actor's properties
	AOpencvTest();
	void ProcessONNX(cv::dnn::Net& net1, cv::Mat& frame);
	void ObstacleDetection(const cv::Mat& oldframe, double min_area);
	void ContourDetection(cv::Mat& src, cv::Mat& output_images, std::vector<std::vector<cv::Point>>& contours, std::vector<cv::Vec4i>& hierarchy, std::vector<std::vector<cv::Point>>& contours_dst);
	void Clear_MicroConnected_Areas(cv::Mat src, cv::Mat& dst, double min_area);
	void info_to_split_info(std::vector<std::vector<float>>& infos);
	std::vector<std::vector<float>> get_info(const cv::Mat& retuslt, int len_data = 6);
	void applyPerspectiveTransform(const cv::Mat& src, cv::Mat& dst, const cv::Point2f vertices[4]);
	void detectRectangle(const cv::Mat& src, cv::Mat& dst, cv::RotatedRect& maxBoxs);
	void draw_rect(cv::Mat& imgs, std::vector<std::vector<float>>& infos);
	void print_info(const std::vector<std::vector<float>>& infos);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
