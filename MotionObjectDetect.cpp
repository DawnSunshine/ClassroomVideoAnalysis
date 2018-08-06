#include "MotionObjectDetect.h"


MotionObjectDetect::MotionObjectDetect():
	THRESHOLD(30)
{

}


MotionObjectDetect::~MotionObjectDetect()
{
}

void MotionObjectDetect::updataFrame(cv::Mat &frame, cv::Mat &diff)
{
	if (last_frame.cols != frame.cols || last_frame.rows != frame.rows)
	{
		last_frame.create(frame.rows, frame.cols, CV_8UC1);
	}

	cv::Mat gray;
	cv::Mat diff_image;
	convert(frame, gray);
	cv::absdiff(gray, last_frame, diff_image);
	cv::threshold(diff_image, diff, THRESHOLD, 255, CV_THRESH_BINARY);

	last_frame = gray.clone();

}

void MotionObjectDetect::convert(cv::Mat &src, cv::Mat &dst)
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