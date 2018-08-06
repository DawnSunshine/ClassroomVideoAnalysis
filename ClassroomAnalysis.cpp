// ClassroomAnalysis.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <WinSock2.h>
#include <Windows.h>
//#include "json/json.h"
#include <string>
#include <sstream>
#include "opencv2/video/tracking.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "StudentStaticAnalysis.h"
#include "TeacherTrack.h"

#define VIDEO_WIDTH 1280
#define VIDEO_HEIGHT 720

using namespace std;
cv::Mat gray_frame;
cv::Mat track_roi;
boost::recursive_mutex cs;
cv::Rect track_rect;

//TCP客户端
WORD wVersion = MAKEWORD(2, 2);
WSADATA WSAData;
SOCKET s ;

void classroom_analysis();
bool init_client_socket();
std::string talk_to_server(std::string &msg);
bool send_mask_to_server(MASK &mask);
bool send_video_to_server(cv::Mat &img);

void classroom_analysis()
{
	cv::Mat diff;
	StudentStaticAnalysis detector;
	TeacherTrack detector2;
	std::vector<cv::Rect> mask;
	cv::Rect t1 = cv::Rect(0, VIDEO_HEIGHT*0.68, VIDEO_WIDTH, VIDEO_HEIGHT*0.32);
	cv::Rect t2 = cv::Rect(VIDEO_WIDTH*0.57, VIDEO_HEIGHT*0.2, VIDEO_WIDTH*0.3, VIDEO_HEIGHT*0.4);
	mask.push_back(t1);
	mask.push_back(t2);
	detector2.setMask(mask);
	cv::Rect t3 = cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.45);
	detector2.setTrackArea(t3);
	//cvNamedWindow("diff_video", 1);
	//cvNamedWindow("pre_img", 1);
	//detector2.setTrackROI(cv::Rect(0, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH, VIDEO_HEIGHT*0.5));
	std::vector<cv::Rect> mask_;
	cv::Rect t4 = cv::Rect(VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.5, VIDEO_WIDTH*0.2, VIDEO_HEIGHT*0.5);
	mask_.push_back(t4);
	detector.setMask(mask_);

	cv::Mat deal_img;
	cv::Rect rect;
	while (true)
	{
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			deal_img = track_roi.clone();			
		}

		//取消注释可以打开老师检测功能
		//detector2.compute(deal_img, rect);

		//取消注释可以打开学生检测功能
		detector.compute(deal_img, rect);

		{
			boost::recursive_mutex::scoped_lock lk(cs);
			track_rect = rect;
		}
		if (cv::waitKey(300) == 27)
			break;
		//Sleep(500);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	cv::VideoCapture capture;

	capture.open("D:\\FFOutput\\student_panorama.avi");
	if (!capture.isOpened())
	{
		cout << "capture is null" << endl;
		getchar();
		return 0;
	}
	int frame_count = capture.get(7);
	capture.set(CV_CAP_PROP_POS_FRAMES, (int)frame_count*0.2);
	cv::Mat frame;
	cv::Mat frame_roi;
	cv::namedWindow("origin_video", 1);

	boost::thread thread(classroom_analysis);
	cv::Mat resize_frame;
	while (true)
	{
		if (!capture.read(frame))
		{
			printf("video capture fail...\n");
			break;
		}
		//cv::resize(frame, resize_frame, cv::Size(480, 270));
		resize_frame = frame;
		//frame_roi = frame(cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.9, VIDEO_HEIGHT*0.4));
		frame_roi = resize_frame;
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			cv::cvtColor(resize_frame, gray_frame, CV_RGB2GRAY);
			track_roi = gray_frame;
			//cv::rectangle(resize_frame, cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.45), cv::Scalar(0, 0, 255), 3);
			if (track_rect.area() > (resize_frame.size().area()*0.005))
				cv::rectangle(frame_roi, track_rect, cv::Scalar(255, 0, 0), 2);
		}		

		cv::imshow("origin_video", resize_frame);

		if (cv::waitKey(30) == 27)
			break;
	}
	capture.release();


	return 0;
}

