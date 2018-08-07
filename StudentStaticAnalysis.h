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

//存放每一帧初次识别结果的容器大小
//该值越大，检测结果抖动越平缓，
//该值越小，结果抖动越大。
#define MAX_FRAMES_TO_COMPUTE 20

//定义掩码类型
typedef std::vector<cv::Rect> MASK;

//定义识别结果结构体
typedef struct
{
	double score;//识别结果
	bool warning_flag;//突发事件警告
}student_static_t;

class StudentStaticAnalysis :
	public MotionObjectDetect
{
public:
	StudentStaticAnalysis();
	~StudentStaticAnalysis();

public:
	//执行检测算法
	void compute(cv::Mat &frame, cv::Rect &rect, student_static_t &result);
	//设置视频掩码
	inline void setMask(MASK mask){ mask_ = mask; }
	//设置突发事件警告灵敏度 warning_sensitivity范围0~1
	inline void setWarningSensitivity(float warning_sensitivity = 0.5){ warning_sensitivity_ = warning_sensitivity; }
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
	void computeScore(const std::deque<double> &contours);

private:
	//膨胀核大小，用来对核初始化
	cv::Size dilate_size;
	//腐蚀核大小，用来对核初始化
	cv::Size erode_size;
	//腐蚀核
	cv::Mat erode_element;
	//膨胀核
	cv::Mat dilate_element;

	//视频掩码
	MASK mask_;
	//突发事件警告灵敏度
	float warning_sensitivity_;
	//是否已经触发突发事件警告
	bool warning_flag;


	long image_size;//图片面积
	double last_static_score;//上一帧的学生安静度，未归一化
	double static_score;//学生安静度,已归一化
	std::deque<double> contours_size_v;//存放每一帧初次识别结果的队列，用来进行学生安静度的计算
};

#endif //TEACHER_TRACK
