/////////////////////////////////////////////////////////////////////
// Name: 老师授课热情检测类
// Autor: 林晓生
// Date: 2018/7/20
// Brief: 输入教室老师的图片、掩码区域和讲台区域，进行老师授课热情的计算。


/////////////////////////////////////////////////////////////////////

#ifndef TEACHER_TRACK
#define TEACHER_TRACK
#include <iostream>
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <boost/date_time.hpp>

//#define DEBUG_COUT true
#define DEBUG_COUT false

#define COUT(x) {if(DEBUG_COUT) std::cout<<x<<std::endl;}

typedef enum
{
	TCH_DOWN_PLATFORM,
	TCH_UP_PLATFORM,
	TCH_OCCUR,
	TCH_NULL,
	TCH_MOVING,
	TCH_STAND
}TEACHER_STATUS;

typedef std::vector<cv::Rect> MASK;

#include "MotionObjectDetect.h"
class TeacherTrack :
	public MotionObjectDetect
{
public:
	TeacherTrack();
	~TeacherTrack();

public:
	//执行检测算法
	void compute(cv::Mat &frame, cv::Rect &rect);
	//设置视频掩码
	inline void setMask(MASK mask){ mask_ = mask; }
	//设置跟踪区域
	inline void setTrackArea(cv::Rect area){ track_area_ = area; }

private:
	//预处理
	void preprocess(cv::Mat &src, cv::Mat &dst);
	//归一转换
	void convert(cv::Mat &src, cv::Mat &dst);
	//根据掩码提取ROI
	void roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask);
	//状态分析
	void statusAnalysis(cv::Rect &active_area);
	//获取当前时间戳
	int64_t getCurrentStamp64();

private:
	//膨胀核大小，用来对核初始化
	cv::Size dilate_size;
	//腐蚀核大小，用来对核初始化
	cv::Size erode_size;
	//腐蚀核
	cv::Mat erode_element;
	//膨胀核
	cv::Mat dilate_element;

	cv::Rect track_area_;
	MASK mask_;


	TEACHER_STATUS current_tch_status[3];

	TEACHER_STATUS last_tch_status[3];
	
	cv::Rect last_active_area;//上一帧检测到的老师活动区域
	double active_score;//老师教学热情度
	double reduction_factor;//衰减系数
	int64_t last_time_stamp_;//时间戳(ms级)
};

#endif //TEACHER_TRACK
