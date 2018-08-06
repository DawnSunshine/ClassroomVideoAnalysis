////////////////////////////////////////////////////////////////////////
// Name: 学生动作幅度(安静程度)检测类
// Autor: 林晓生
// Date: 2018/8/6
// Brief: 输入教室学生的图片和掩码区域，进行学生动作幅度的检测和突发事件预警。


////////////////////////////////////////////////////////////////////////

#ifndef STUDENT_STATIC_ANALYSIS
#define STUDENT_STATIC_ANALYSIS

#include "MotionObjectDetect.h"
#include <iostream>
#include <vector>
#include <deque>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <boost/date_time.hpp>

#define MAX_FRAMES_TO_COMPUTE 20

typedef std::vector<cv::Rect> MASK;

class StudentStaticAnalysis :
	public MotionObjectDetect
{
public:
	StudentStaticAnalysis();
	~StudentStaticAnalysis();

public:
	void compute(cv::Mat &frame, cv::Rect &rect);

	inline void setMask(MASK mask){ mask_ = mask; }

	inline void setTrackArea(cv::Rect area){ track_area_ = area; }

private:
	//预处理
	void preprocess(cv::Mat &src, cv::Mat &dst);
	//归一转换
	void convert(cv::Mat &src, cv::Mat &dst);
	//根据掩码提取ROI
	void roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask);
	//获取当前时间戳
	//int64_t getCurrentStamp64();
	//学生安静度分数计算
	void computeScore(std::deque<long> &contours);

private:
	//膨胀核大小，用来核初始化
	cv::Size dilate_size;
	//腐蚀核大小
	cv::Size erode_size;
	//腐蚀核
	cv::Mat erode_element;
	//膨胀核
	cv::Mat dilate_element;

	cv::Rect track_area_;
	MASK mask_;


	long image_size;//图片面积
	float static_score;//学生安静度
	std::deque<long> contours_size_v;//连续若干帧的轮廓长度队列，用来进行学生安静度的计算
};

#endif //TEACHER_TRACK
