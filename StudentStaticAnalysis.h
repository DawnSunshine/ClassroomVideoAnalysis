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

//���ÿһ֡����ʶ������������С
//��ֵԽ�󣬼��������Խƽ����
//��ֵԽС���������Խ��
#define MAX_FRAMES_TO_COMPUTE 20

//������������
typedef std::vector<cv::Rect> MASK;

//����ʶ�����ṹ��
typedef struct
{
	double score;//ʶ����
	bool warning_flag;//ͻ���¼�����
}student_static_t;

class StudentStaticAnalysis :
	public MotionObjectDetect
{
public:
	StudentStaticAnalysis();
	~StudentStaticAnalysis();

public:
	//ִ�м���㷨
	void compute(cv::Mat &frame, cv::Rect &rect, student_static_t &result);
	//������Ƶ����
	inline void setMask(MASK mask){ mask_ = mask; }
	//����ͻ���¼����������� warning_sensitivity��Χ0~1
	inline void setWarningSensitivity(float warning_sensitivity = 0.5){ warning_sensitivity_ = warning_sensitivity; }
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
	void computeScore(const std::deque<double> &contours);

private:
	//���ͺ˴�С�������Ժ˳�ʼ��
	cv::Size dilate_size;
	//��ʴ�˴�С�������Ժ˳�ʼ��
	cv::Size erode_size;
	//��ʴ��
	cv::Mat erode_element;
	//���ͺ�
	cv::Mat dilate_element;

	//��Ƶ����
	MASK mask_;
	//ͻ���¼�����������
	float warning_sensitivity_;
	//�Ƿ��Ѿ�����ͻ���¼�����
	bool warning_flag;


	long image_size;//ͼƬ���
	double last_static_score;//��һ֡��ѧ�������ȣ�δ��һ��
	double static_score;//ѧ��������,�ѹ�һ��
	std::deque<double> contours_size_v;//���ÿһ֡����ʶ�����Ķ��У���������ѧ�������ȵļ���
};

#endif //TEACHER_TRACK
