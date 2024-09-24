#include "UERobotOJJudger/Public/OpencvTest.h"


void AOpencvTest::ProcessONNX(cv::dnn::Net& net1, cv::Mat& frame)
{
	cv::Mat resized_frame;

	{
		std::lock_guard<std::mutex> Imagelock(ponxx);
		if (frame.empty()) {
			return;
		}
	}
	{
		std::lock_guard<std::mutex> Imagelock(ponxx);
		cv::resize(frame, resized_frame, cv::Size(640, 640));
	}
	cv::Mat blob = cv::dnn::blobFromImage(resized_frame, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(), true);
	net1.setInput(blob);
	std::vector<cv::Mat> netoutput;
	std::vector<std::string> out_name = { "output0" };
	net1.forward(netoutput, out_name);

	if (netoutput.empty()) {
		std::cerr << "Error: netoutput is empty." << std::endl;
		return;
	}

	cv::Mat result = netoutput[0];
	std::vector<std::vector<float>> infos = get_info(result);
	info_to_split_info(infos);
	draw_rect(resized_frame, infos);
	resize(resized_frame, resized_frame, cv::Size(640, 360));
	imshow("car", resized_frame);
	{
		std::lock_guard<std::mutex> lock1(car);
		if (!infos.empty() && infos[0].size() >= 4) {
			car_center.x = (infos[0][0] + infos[0][2]) / 2 * 3;
			car_center.y = (infos[0][1] + infos[0][3]) / 2 * 1.69;
		}
		else {
			std::cerr << "Error: info vector is empty or does not have enough elements." << std::endl;
		}
	}
    
}

void AOpencvTest::info_to_split_info(std::vector<std::vector<float>>& infos)
{
	for (auto i = 0; i < infos.size(); i++)
	{
		float x1 = infos[i][0];
		float y1 = infos[i][1];
		float x2 = infos[i][2];
		float y2 = infos[i][3];
		infos[i][0] = x1 - x2 / 2;
		infos[i][1] = y1 - y2 / 2;
		infos[i][2] = x1 + x2 / 2;
		infos[i][3] = y1 + y2 / 2;
	}
}

std::vector<std::vector<float>> AOpencvTest::get_info(const cv::Mat& retuslt, int len_data)
{
	float* pdata = (float*)retuslt.data;
	std::vector<std::vector<float>> s;
	for (int i = 0; i < retuslt.total() / len_data; i++)
	{
		if (pdata[4] > 0.5) {
			std::vector<float> info_line;
			for (auto j = 0; j < len_data; j++)
			{
				info_line.push_back(pdata[j]);
			}
			s.push_back(info_line);
		}
		pdata += len_data;
	}
	return s;
}

void AOpencvTest::draw_rect(cv::Mat& imgs, std::vector<std::vector<float>>& infos)
{
	for (auto i = 0; i < infos.size(); i += 10)
	{
		cv::rectangle(imgs, cv::Point((int)infos[i][0], (int)infos[i][1]), cv::Point(infos[i][2], infos[i][3]), cv::Scalar(0, 0, 255), 1);
		cv::circle(imgs, cv::Point(((int)infos[i][0] + (int)infos[i][2]) / 2 * 3, ((int)infos[i][1] + (int)infos[i][3]) / 2 * 1.69), 1, cv::Scalar(0, 0, 255), -1);
		//标注中心点
		cv::putText(imgs, cv::format("(%.1f,%.1f)", (infos[i][0] + infos[i][2]) / 2 * 3, (infos[i][1] + infos[i][3]) / 2 * 1.69), cv::Point((infos[i][0] + infos[i][2]) / 2, (infos[i][1] + infos[i][3]) / 2), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
	}
}

void AOpencvTest::print_info(const std::vector<std::vector<float>>& infos)
{
	for (auto i = 0; i < infos.size(); i++)
	{
		for (auto j = 0; j < infos[i].size(); j++)
		{
			std::cout << infos[i][j] << " ";
		}
		printf("\n");
	}
}

void AOpencvTest::applyPerspectiveTransform(const cv::Mat& src, cv::Mat& dst, const cv::Point2f vertices[4])
{
	// 修正透视变换后的目标顶点，使其符合新的要求
	float width = static_cast<float>(src.cols);
	float height = static_cast<float>(src.rows);
	cv::Point2f dst_vertices[4] = {
		cv::Point2f(0, height),    // 左下角
		cv::Point2f(0, 0),         // 左上角
		cv::Point2f(width, 0),     // 右上角
		cv::Point2f(width, height) // 右下角
	};
	cv::Mat perspective_matrix = cv::getPerspectiveTransform(vertices, dst_vertices);
	cv::warpPerspective(src, dst, perspective_matrix, cv::Size(static_cast<int>(width), static_cast<int>(height)));
}

void AOpencvTest::detectRectangle(const cv::Mat& src, cv::Mat& dst, cv::RotatedRect& maxBoxs)
{
	cv::Mat gray, edges, dilated;
	// 转换为灰度图像
	cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	// 高斯模糊
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);
	// 边缘检测
	cv::Canny(gray, edges, 50, 150);

	// 形态学操作，连接断开的边缘
	cv::dilate(edges, dilated, cv::Mat(), cv::Point(-1, -1), 2);
	cv::erode(dilated, dilated, cv::Mat(), cv::Point(-1, -1), 1);


	std::vector<std::vector<cv::Point>> contours;
	// 轮廓检测
	cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


	// 霍夫曼直线检测
	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(dilated, lines, 1, CV_PI / 180, 50, 50, 10);
	// 保留长度大于500的直线
	std::vector<cv::Vec4i> lines2;
	for (size_t i = 0; i < lines.size(); i++) {
		cv::Vec4i l = lines[i];
		double length = sqrt(pow(l[0] - l[2], 2) + pow(l[1] - l[3], 2));
		if (length > 500) {
			lines2.push_back(l);
		}
	}

	// 将直线端点加入点集
	std::vector<cv::Point> points;
	for (size_t i = 0; i < lines2.size(); i++) {
		points.push_back(cv::Point(lines2[i][0], lines2[i][1]));
		points.push_back(cv::Point(lines2[i][2], lines2[i][3]));
	}

	// 如果检测到的点不足以形成矩形，输出错误信息
	if (points.size() < 4) {
		std::cerr << "Not enough points detected to form a rectangle." << std::endl;
		return;
	}

	// 使用最小面积矩形包围所有点
	cv::RotatedRect box = cv::minAreaRect(points);
	double area = box.size.width * box.size.height;

	// 只保留面积在指定范围内的矩形，并更新历史最大矩形
	if (area >= 1180000 && area <= 1190000) {
		if (area > maxBoxs.size.width * maxBoxs.size.height) {
			maxBoxs = box;
		}
	}
}

void AOpencvTest::ContourDetection(cv::Mat& src, cv::Mat& output_images, std::vector<std::vector<cv::Point>>& contours, std::vector<cv::Vec4i>& hierarchy, std::vector<std::vector<cv::Point>>& contours_dst)
{
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::Mat dst = cv::Mat::zeros(output_images.size(), CV_8UC1);
	for (int i = 0; i < contours.size(); i++) {
		fillPoly(dst, contours[i], 255);
	}
	cv::findContours(dst, contours_dst, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
}

void AOpencvTest::Clear_MicroConnected_Areas(cv::Mat src, cv::Mat& dst, double min_area)
{
	dst = src.clone();
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());

	if (!contours.empty() && !hierarchy.empty())
	{
		std::vector<std::vector<cv::Point> >::const_iterator itc = contours.begin();
		while (itc != contours.end())
		{
			cv::Rect rect = cv::boundingRect(cv::Mat(*itc));
			double area = contourArea(*itc);
			if (area < min_area)
			{
				for (int i = rect.y; i < rect.y + rect.height; i++)
				{
					uchar* output_data = dst.ptr<uchar>(i);
					for (int j = rect.x; j < rect.x + rect.width; j++)
					{
						if (output_data[j] == 255)
						{
							output_data[j] = 0;
						}
					}
				}
			}
			itc++;
		}
	}
}

void AOpencvTest::ObstacleDetection(const cv::Mat& oldframe, double min_area)
{
	cv::Mat output_images;
   
	{
		std::lock_guard <std::mutex> Imagelock(obs);
		if (oldframe.empty())
		{
			//解锁mtx2
			return;
		}
	}
	{
		std::lock_guard <std::mutex> Imagelock(obs);
		if (!oldframe.empty())
		{
			output_images = oldframe.clone();
		}
		else
		{
			printf("Error: oldframe is empty.\n");
			return;
		}
	}
	cv::resize(output_images, output_images, cv::Size(output_images.cols / 2, output_images.rows / 2));
	cv::Mat frame_hsv;
	cv::cvtColor(output_images, frame_hsv, cv::COLOR_BGR2HSV);
	cv::Mat frame_mask;
	cv::inRange(frame_hsv, cv::Scalar(20, 35, 193), cv::Scalar(50, 55, 400), frame_mask);
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(frame_mask, frame_mask, cv::MORPH_OPEN, kernel);
	Clear_MicroConnected_Areas(frame_mask, frame_mask, min_area);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	std::vector<std::vector<cv::Point>> contours_dst;
	ContourDetection(frame_mask, output_images, contours, hierarchy, contours_dst);
    
	//清空障碍物中心坐标数组
	{
		std::lock_guard<std::mutex> lock(obslocation);
		obstacle_center.clear();
		for (int i = 0; i < contours_dst.size(); i++) {
			cv::Rect rect = cv::minAreaRect(contours_dst[i]).boundingRect();
			cv::Point center = cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
			cv::circle(output_images, center, 5, cv::Scalar(0, 0, 255), -1);
			cv::putText(output_images, cv::format("(%d,%d)", center.x * 2, center.y * 2), center, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
			cv::rectangle(output_images, rect, cv::Scalar(0, 255, 0), 2);
			center.x *= 2;
			center.y *= 2;
			obstacle_center.push_back(center);
		}
	}
	imshow("obs", output_images);
}



// Sets default values
AOpencvTest::AOpencvTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOpencvTest::BeginPlay()
{
	Super::BeginPlay();
	try {
		net = cv::dnn::readNetFromONNX("D:/UERobotOJJudger/ThirdParty/OpenCV/Source/best.onnx");
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	} catch (const cv::Exception& e) {
		std::cerr << "Error loading ONNX model: " << e.what() << std::endl;
		return;
	}

	// 打开视频文件
	capture.open("D:/UERobotOJJudger/ThirdParty/OpenCV/Source/test.mp4");
	if (!capture.isOpened()) {
		std::cerr << "Error opening video file." << std::endl;
		return;
	}
}

// Called every frame
void AOpencvTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 读取视频帧
	capture >> img;
	if (img.empty()) {
		std::cerr << "Error: frame is empty." << std::endl;
		return;
	}
	{
		std::lock_guard<std::mutex> Normallock(imglock);
		NormalImg = img.clone();
	}
	// 检测矩形并标出顶点
	detectRectangle(img, output_image, maxBox);
	cv::Point2f vertices[4];
	//将maxBox的四个顶点存入vertices
	maxBox.points(vertices);
	applyPerspectiveTransform(img, output_image, vertices);
	{
		std::lock_guard<std::mutex> obslock(obs);
		obsimg = output_image.clone();
	}
	{
		std::lock_guard<std::mutex> Ponxxlock(ponxx);
		Ponxximg = output_image.clone();
	}
	//判断如果flag==0，播放output_image，如果flag==1，播放NormalImg
	{
		std::lock_guard<std::mutex> lock(flaglock);
		if (flag == 0) {
			outImg = output_image.clone();
		}
		if (flag == 1) {
			{
				std::lock_guard<std::mutex> Norlock(imglock);
				outImg = NormalImg.clone();
			}
		}
	}
	ObstacleDetection(obsimg, 2000);
	ProcessONNX(net,Ponxximg);
	cv::resize(outImg, outImg, cv::Size(1920,1080));
	//imshow outImg
	cv::resize(outImg, outImg, cv::Size(640, 360));
	cv::imshow("Video", outImg);

	// 按下ESC键退出
	if (cv::waitKey(1) == 27) {
		
	}
}
