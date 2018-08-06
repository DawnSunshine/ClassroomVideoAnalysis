#include "StudentStaticAnalysis.h"
#include <boost/timer.hpp>

StudentStaticAnalysis::StudentStaticAnalysis() :
erode_size(5, 5), dilate_size(15, 15),
static_score(100), image_size(1), contours_size_v(MAX_FRAMES_TO_COMPUTE, -1)
{

	erode_element = cv::getStructuringElement(cv::MORPH_RECT, erode_size);
	dilate_element = cv::getStructuringElement(cv::MORPH_RECT, dilate_size);
	track_area_ = cv::Rect(0, 0, 0, 0);

}

StudentStaticAnalysis::~StudentStaticAnalysis()
{

}

void StudentStaticAnalysis::compute(cv::Mat &frame, cv::Rect &rect)
{
	if (0 == frame.rows || 0 == frame.cols)
		return;
	image_size = frame.rows*frame.cols;

	boost::timer t;
	cv::Mat gray;
	convert(frame, gray);

	cv::Mat roi_img;
	roiExtract(gray, roi_img, mask_);

	cv::imshow("roi_img", roi_img);

	cv::Mat diff;
	updataFrame(roi_img, diff);

	cv::imshow("diff_video", diff);

	cv::Mat pre_img;
	preprocess(diff, pre_img);

	cv::imshow("pre_img", pre_img);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(pre_img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	/*
	std::vector<std::vector<cv::Point>> contours2;
	std::vector<cv::Vec4i> hierarchy2;
	cv::findContours(pre_img, contours2, hierarchy2, cv::RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	*/

	// 寻找最大连通域
	long contours_size = 0;
	double maxArea = 0;
	std::vector<cv::Point> maxContour;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxContour = contours[i];
		}
		contours_size = contours_size + contours[i].size();
	}
	contours_size_v.pop_front();
	contours_size_v.push_back(contours_size);
	computeScore(contours_size_v);
	//printf("Student Static Score is %d \n", static_score);
	// 将轮廓转为矩形框
	if (maxContour.size() != 0)
		rect = cv::boundingRect(maxContour);
	else
		rect = cv::Rect(0, 0, 0, 0);

	cv::Mat dstImage = cv::Mat::zeros(frame.size(), CV_8UC3);
	for (int i = 0; i < hierarchy.size(); i++)
	{
		cv::Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
		drawContours(dstImage, contours, i, color, 3, 8, hierarchy);
	}
	cv::imshow("仅外轮廓图", dstImage);
	/*
	cv::Mat dstImage2 = cv::Mat::zeros(frame.size(), CV_8UC3);
	for (int i = 0; i < hierarchy2.size(); i++)
	{
	cv::Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
	drawContours(dstImage2, contours2, i, color, 3, 8, hierarchy2);
	}
	cv::imshow("内外轮廓图", dstImage2);*/

	std::cout << t.elapsed() * 1000 << "ms" << std::endl;

	if (track_area_.area() == 0)//判断跟踪区域是否存在
		return;


}

void StudentStaticAnalysis::preprocess(cv::Mat &src, cv::Mat &dst)
{
	cv::Mat temp;
	//先腐蚀
	erode(src, temp, erode_element);
	//后膨胀
	dilate(temp, dst, dilate_element);
	
	
}

void StudentStaticAnalysis::convert(cv::Mat &src, cv::Mat &dst)
{
	if (src.channels() == 3)
	{
		cv::cvtColor(src, dst, CV_BGR2GRAY);
	}
	else
	{
		dst = src;
	}
}

void StudentStaticAnalysis::roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask)
{
	if (src.size() != dst.size())
	{
		dst.create(src.size(), src.type());
	}
	cv::Mat mask_mat(src.size(), CV_8UC1, 1);
	for (int i = 0; i < mask.size(); i++)
	{
		cv::rectangle(mask_mat, mask[i], cv::Scalar(0), CV_FILLED);
	}
	dst = src.mul(mask_mat);
}

//int64_t StudentStaticAnalysis::getCurrentStamp64()
//{
//	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
//	boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time() - epoch;
//	return time_from_epoch.total_milliseconds();
//}

void StudentStaticAnalysis::computeScore(std::deque<long> &contours)
{
	int num = contours.size();
	int effective_num = 0;
	long sum = 0;
	for (int i = 0; i < num; i++)
	{
		if (contours[i] > -1)
		{
			sum += contours[i];
			effective_num++;
		}
	}
	if (effective_num > 0)
	{
		sum = sum / effective_num;
	}

	if (sum < (image_size*0.00001))
		static_score = 0;
	else
		static_score = (float)sum / (image_size * 0.0005) * 100;

	if (static_score > 100)
		static_score = 0;
	else
		static_score = 100 - static_score;

	printf("Static Score is %f\n", static_score);

	return;
}
