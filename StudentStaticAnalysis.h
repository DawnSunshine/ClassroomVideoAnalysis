////////////////////////////////////////////////////////////////////////
// Name: ѧ����������(�����̶�)�����
// Autor: ������
// Date: 2018/8/6
// Brief: �������ѧ����ͼƬ���������򣬽���ѧ���������ȵļ���ͻ���¼�Ԥ����


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
	//Ԥ����
	void preprocess(cv::Mat &src, cv::Mat &dst);
	//��һת��
	void convert(cv::Mat &src, cv::Mat &dst);
	//����������ȡROI
	void roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask);
	//��ȡ��ǰʱ���
	//int64_t getCurrentStamp64();
	//ѧ�������ȷ�������
	void computeScore(std::deque<long> &contours);

private:
	//���ͺ˴�С�������˳�ʼ��
	cv::Size dilate_size;
	//��ʴ�˴�С
	cv::Size erode_size;
	//��ʴ��
	cv::Mat erode_element;
	//���ͺ�
	cv::Mat dilate_element;

	cv::Rect track_area_;
	MASK mask_;


	long image_size;//ͼƬ���
	float static_score;//ѧ��������
	std::deque<long> contours_size_v;//��������֡���������ȶ��У���������ѧ�������ȵļ���
};

#endif //TEACHER_TRACK