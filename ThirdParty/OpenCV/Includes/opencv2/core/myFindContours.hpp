

#include "opencv2/opencv.hpp"
#include <opencv2/core/types_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#pragma once

#include <vector>
using namespace std;
using namespace cv;
/*************************
@author:  Lyu Shuai
@Time:  21/08/2021
@Source:   https://blog.csdn.net/amani_liu/article/details/87932141
@Func:  解决使用OpenCV的findContours函数时，会出现崩溃问题
	some changes have been made to fit opencv4.5

********************************/


void myFindContours(const Mat& src, vector<vector<Point>>& contours, vector<Vec4i>& hierarchy,int retr, int method , Point offset);
void myFindContours(const Mat& src, vector<vector<Point>>& contours, vector<Vec4i>& hierarchy,
	int retr, int method , Point offset)
{
	CvMat c_image = cvMat(src);
	//Mat c_image = src;

	MemStorage storage(cvCreateMemStorage());
	CvSeq* _ccontours = 0;

	//cvFindContours(&c_image, storage, &_ccontours, sizeof(CvContour), retr, method, CvPoint(offset));

	cvFindContours(&c_image, storage, &_ccontours, sizeof(CvContour), retr, method, cvPoint(offset.x, offset.y));

	if (!_ccontours)
	{
		contours.clear();
		return;
	}
	Seq<CvSeq*> all_contours(cvTreeToNodeSeq(_ccontours, sizeof(CvSeq), storage));
	int total = (int)all_contours.size();
	contours.resize(total);

	SeqIterator<CvSeq*> it = all_contours.begin();
	for (int i = 0; i < total; i++, ++it)
	{
		CvSeq* c = *it;
		((CvContour*)c)->color = (int)i;
		int count = (int)c->total;
		int* data = new int[count * 2];
		cvCvtSeqToArray(c, data);
		for (int j = 0; j < count; j++)
		{
			contours[i].push_back(Point(data[j * 2], data[j * 2 + 1]));
		}
		delete[] data;
	}

	hierarchy.resize(total);
	it = all_contours.begin();
	for (int i = 0; i < total; i++, ++it)
	{
		CvSeq* c = *it;
		int h_next = c->h_next ? ((CvContour*)c->h_next)->color : -1;
		int h_prev = c->h_prev ? ((CvContour*)c->h_prev)->color : -1;
		int v_next = c->v_next ? ((CvContour*)c->v_next)->color : -1;
		int v_prev = c->v_prev ? ((CvContour*)c->v_prev)->color : -1;
		hierarchy[i] = Vec4i(h_next, h_prev, v_next, v_prev);
	}
	storage.release();
}

