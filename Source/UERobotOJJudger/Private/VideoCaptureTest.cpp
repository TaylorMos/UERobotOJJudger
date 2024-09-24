// Fill out your copyright notice in the Description page of Project Settings.



#include "UERobotOJJudger/Public/VideoCaptureTest.h"

// Sets default values
AVideoCaptureTest::AVideoCaptureTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVideoCaptureTest::BeginPlay()
{
	Super::BeginPlay();
	FVideoCapture* VideoCapture = new FVideoCapture(TEXT("视频流获取"), this);
	FVideoCorrection* VideoCorrection = new FVideoCorrection(TEXT("视频流矫正"), this);
	FRunnableThread* RunnableThread1=FRunnableThread::Create(VideoCapture,*VideoCapture->MyThreadName);
	FRunnableThread* RunnableThread2=FRunnableThread::Create(VideoCorrection,*VideoCorrection->MyThreadName);
}

// Called every frame
void AVideoCaptureTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void FVideoCapture::OpenVideo()
{
	
}

// 定义鼠标点击事件的回调函数
void mouse_callback(int event, int x, int y, int flags, void* param) {
	if (event == cv::EVENT_LBUTTONDOWN) {
		auto* data = reinterpret_cast<std::pair<std::vector<cv::Point2f>*, cv::Mat*>*>(param);
		if (!data || !data->first || !data->second) {
			std::cerr << "Invalid parameter passed to mouse_callback." << std::endl;
			return;
		}

		std::vector<cv::Point2f>* points = data->first;
		cv::Mat* img = data->second;

		points->push_back(cv::Point2f(x, y));
		// 在点击的点上画一个小圆圈
		circle(*img, cv::Point(x, y), 5, cv::Scalar(0, 255, 0), -1);
		imshow("Image", *img);
	}
}

// 纯函数：获取用户点击的四个点的坐标
void AVideoCaptureTest::SetPoints() {
	cv::Mat img_points;
	{
		FScopeLock SetLock(&FrameLock);
		img_points = VideoStream.clone();
	}
	cv::namedWindow("Image");
	std::pair<std::vector<cv::Point2f>*, cv::Mat*> callbackData = {&points, &img_points};
	setMouseCallback("Image", mouse_callback, &callbackData);
	// 显示图片并等待用户点击四个点
	while (true) {
		cv::imshow("Image", img_points);
		if (points.size() == 4)
			break;
		if (cv::waitKey(1) == 27) {  // 按下ESC键退出
			break;
		}
	}
	cv::destroyAllWindows();
}

bool FVideoCapture::Init()
{
	std::cout<<"Thread "<<&MyThreadName<<" is starting"<<std::endl;
	//字符串拼接VideoCapture->ProjectDir+"ThirdParty/OpenCV/Source/1.mp4"
	VideoDir=VideoCapture->ProjectDir+"ThirdParty/OpenCV/Source/1.mp4";
	//打印路径
	std::cout<<TCHAR_TO_UTF8(*VideoDir)<<std::endl;
	return true;
}

uint32 FVideoCapture::Run()
{
	std::cout<<"Thread "<<&MyThreadName<<" is running"<<std::endl;
	//获取当前项目路径
	cap= cv::VideoCapture(TCHAR_TO_UTF8(*VideoDir));
	if (!cap.isOpened())
	{
		std::cout<<"Error: Can't open the video file."<<std::endl;
		return -1;
	}
	while (true)
	{
		{
			FScopeLock SetLock(&VideoCapture->FrameLock);
			cap >> VideoCapture->VideoStream;
			if (VideoCapture->VideoStream.empty())
			{
				std::cout<<"1."<<std::endl;
				break;
			}
			//640*360
			cv::resize(VideoCapture->VideoStream,VideoCapture->VideoStream,cv::Size(1280,720));
		}
		FPlatformProcess::Sleep(0.001f);
	}
	cap.release();//释放视频流
	cv::destroyAllWindows();//关闭所有窗口
	return 0;
}

void FVideoCapture::Stop()
{
	cap.release();//释放视频流
	cv::destroyAllWindows();
}

void FVideoCapture::Exit()
{
	cap.release();//释放视频流
	cv::destroyAllWindows();
	std::cout<<"Thread "<<&MyThreadName<<" is exiting"<<std::endl;
}

void FVideoCorrection::applyPerspectiveTransform(const cv::Mat& src, cv::Mat& dst, const cv::Point2f vertices[4]) {
	// 修正透视变换后的目标顶点，使其符合新的要求
	float width = static_cast<float>(src.cols);
	float height = static_cast<float>(src.rows);
	cv::Point2f dst_vertices[4] = {
		cv::Point2f(0, 0),         // 左上角
		cv::Point2f(0, height),    // 左下角
		cv::Point2f(width, height), // 右下角
		cv::Point2f(width, 0)      // 右上角
	};
	cv::Mat perspective_matrix = cv::getPerspectiveTransform(vertices, dst_vertices);
	cv::warpPerspective(src, dst, perspective_matrix, cv::Size(static_cast<int>(width), static_cast<int>(height)));
}


bool FVideoCorrection::Init()
{
	std::cout<<"Thread "<<&MyThreadName<<" is starting"<<std::endl;
	OnnxDir=VideoCapture->ProjectDir+"ThirdParty/OpenCV/Source/best.onnx";
	return true;
}

uint32 FVideoCorrection::Run()
{
	FPlatformProcess::Sleep(0.1f);
	std::cout<<"Thread "<<&MyThreadName<<" is running"<<std::endl;
	cv::Mat Input_Frame;
	cv::Mat Output_Frame;
	while (true)
	{
		{
			FScopeLock SetLock(&VideoCapture->FrameLock);
			Input_Frame = VideoCapture->VideoStream;
		}
		if (Input_Frame.empty())
		{
			std::cout<<"Error: Can't open the video file."<<std::endl;
			break;
		}
		if(VideoCapture->points.size()==4)
		{
			{
				FScopeLock SetLock(&VideoCapture->PointsLock);
				applyPerspectiveTransform(Input_Frame,Output_Frame,VideoCapture->points.data());
			}
			VideoCapture->VideoStreamCorrection=Output_Frame;
			cv::imshow("Correction",Output_Frame);
			if(cv::waitKey(1)==27)
				break;
		}
		//获取本地毫秒时间戳
		//std::chrono::milliseconds ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		//std::cout<<"Time:"<<ms2.count()-ms.count()<<std::endl;
	}
	return 0;
}

void FVideoCorrection::Stop()
{
	cv::destroyAllWindows();
}

void FVideoCorrection::Exit()
{
	cv::destroyAllWindows();
	std::cout<<"Thread "<<&MyThreadName<<" is exiting"<<std::endl;
}

FWindowsCriticalSection AVideoCaptureTest::FrameLock;//定义静态变量
FWindowsCriticalSection AVideoCaptureTest::PointsLock;//定义静态变量
