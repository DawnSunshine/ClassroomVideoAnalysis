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
	KEEP_DOWN_PLATFORM,
	TCH_UP_PLATFORM,
	TCH_OCCUR,
	TCH_NULL,
	TCH_MOVING,
	KEEP_MOVING,
	TCH_STAND,
	KEEP_STAND
}TEACHER_STATUS;

typedef struct
{
	double active_score;//讲课热情
	TEACHER_STATUS status;//老师状态
	int down_platform_num;//下讲台次数
	int up_platform_num;//上讲台次数
	int moving_num;//移动次数
	int stand_num;//站立次数

}detection_teacher_t;

typedef std::vector<cv::Rect> MASK;

#include "MotionObjectDetect.h"
class TeacherTrack :
	public MotionObjectDetect
{
public:
	TeacherTrack();
	~TeacherTrack();

public:
	void compute(cv::Mat &frame, cv::Rect &rect, detection_teacher_t &result);

	inline void setMask(MASK mask){ mask_ = mask; }

	inline void setTrackArea(cv::Rect area){ track_area_ = area; }

private:
	//预处理
	void preprocess(cv::Mat &src, cv::Mat &dst);
	//归一转换
	void convert(cv::Mat &src, cv::Mat &dst);
	//根据掩码提取ROI
	void roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask);
	//状态分析
	void statusAnalysis(cv::Rect &active_area, detection_teacher_t &status);
	//获取当前时间戳
	int64_t getCurrentStamp64();

private:
	cv::Size dilate_size;
	cv::Size erode_size;
	cv::Mat erode_element;
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
