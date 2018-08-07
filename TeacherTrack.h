/////////////////////////////////////////////////////////////////////
// Name: ��ʦ�ڿ���������
// Autor: ������
// Date: 2018/7/20
// Brief: ���������ʦ��ͼƬ����������ͽ�̨���򣬽�����ʦ�ڿ�����ļ��㡣


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
	//ִ�м���㷨
	void compute(cv::Mat &frame, cv::Rect &rect);
	//������Ƶ����
	inline void setMask(MASK mask){ mask_ = mask; }
	//���ø�������
	inline void setTrackArea(cv::Rect area){ track_area_ = area; }

private:
	//Ԥ����
	void preprocess(cv::Mat &src, cv::Mat &dst);
	//��һת��
	void convert(cv::Mat &src, cv::Mat &dst);
	//����������ȡROI
	void roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask);
	//״̬����
	void statusAnalysis(cv::Rect &active_area);
	//��ȡ��ǰʱ���
	int64_t getCurrentStamp64();

private:
	//���ͺ˴�С�������Ժ˳�ʼ��
	cv::Size dilate_size;
	//��ʴ�˴�С�������Ժ˳�ʼ��
	cv::Size erode_size;
	//��ʴ��
	cv::Mat erode_element;
	//���ͺ�
	cv::Mat dilate_element;

	cv::Rect track_area_;
	MASK mask_;


	TEACHER_STATUS current_tch_status[3];

	TEACHER_STATUS last_tch_status[3];
	
	cv::Rect last_active_area;//��һ֡��⵽����ʦ�����
	double active_score;//��ʦ��ѧ�����
	double reduction_factor;//˥��ϵ��
	int64_t last_time_stamp_;//ʱ���(ms��)
};

#endif //TEACHER_TRACK
