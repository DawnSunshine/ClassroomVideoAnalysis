// ClassroomAnalysis.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include <stdlib.h>
#include "json/json.h"
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
#include <boost/date_time/posix_time/posix_time.hpp>  

#include "StudentStaticAnalysis.h"
#include "TeacherTrack.h"

//帧分辨率
#define VIDEO_WIDTH 480 
#define VIDEO_HEIGHT 270

using namespace std;
cv::Mat student_img;
cv::Mat teacher_img;
boost::recursive_mutex cs;
cv::Rect teacher_track_rect;
cv::Rect student_track_rect;

//学生检测结果
detection_student_t student_static_result;
detection_teacher_t teacher_acitve_result;


void classroom_analysis();
bool init_client_socket();
std::string talk_to_server(std::string &msg);
bool send_mask_to_server(MASK &mask);
bool send_video_to_server(cv::Mat &img);

Json::Value student_json_root;
Json::Value student_json_value;
Json::Value teacher_json_root;
Json::Value teacher_json_value;
Json::Value time_json_root;
Json::Value time_json_value;

std::string getCurrentTime()
{
	std::string strTime = boost::posix_time::to_iso_string(\
		boost::posix_time::second_clock::local_time());

	// 这时候strTime里存放时间的格式是YYYYMMDDTHHMMSS，日期和时间用大写字母T隔开了  
	int pos = strTime.find('T');
	strTime.replace(pos, 1, std::string("-"));
	strTime.replace(pos + 3, 0, std::string(":"));
	strTime.replace(pos + 6, 0, std::string(":"));
	return strTime;
}

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
	cv::Rect t4 = cv::Rect(1280*0.8, 720*0.5, 1280*0.2, 720*0.5);
	mask_.push_back(t4);
	detector.setMask(mask_);

	cv::Mat deal_student_img;
	cv::Mat deal_teacher_img;
	cv::Rect student_rect;
	cv::Rect teacher_rect;
	detection_student_t stu_result;
	stu_result.score = 0;
	stu_result.warning_flag = false;
	detection_teacher_t tch_result;
	tch_result.active_score = 0;
	tch_result.down_platform_num = 0;
	tch_result.moving_num = 0;
	tch_result.stand_num = 0;
	tch_result.up_platform_num = 0;
	tch_result.status = TCH_NULL;

	long json_index = 0;
	while (true)
	{
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			deal_student_img = student_img.clone();
			deal_teacher_img = teacher_img.clone();
		}

		//取消注释可以打开老师检测功能
		detector2.compute(deal_teacher_img, teacher_rect, tch_result);

		//取消注释可以打开学生检测功能
		detector.compute(deal_student_img, student_rect, stu_result);

		student_json_value[json_index] = stu_result.score;
		teacher_json_value[json_index] = tch_result.active_score;
		time_json_value[json_index] = getCurrentTime();
		json_index++;
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			student_static_result = stu_result;
			teacher_acitve_result = tch_result;
			student_track_rect = student_rect;
			teacher_track_rect = teacher_rect;
		}
		//if (cv::waitKey(300) == 27)
		//	break;
		Sleep(1000);
	}

}

void video_read_student(std::string path, bool resize_flag, cv::Size resize)
{
	cv::VideoCapture capture_student;
	capture_student.open(path);
	if (!capture_student.isOpened())
	{
		cout << "capture_student is null" << endl;
		getchar();
		return ;
	}
	int frame_count = capture_student.get(7);
	//capture_student.set(CV_CAP_PROP_POS_FRAMES, (int)frame_count*0.9);
	capture_student.set(CV_CAP_PROP_POS_FRAMES, 600*25 );
	cv::Mat frame_student;
	cv::Mat frame_roi;
	cv::Mat gray_frame;
	cv::namedWindow("student_video", 1);

	cv::Mat resize_frame;
	int during_emergency_num = 0;

	while (true)
	{
		if (!capture_student.read(frame_student))
		{
			printf("video capture_student fail...\n");
			break;
		}
		//cv::resize(frame_student, resize_frame, cv::Size(480, 270));
		resize_frame = frame_student;
		//frame_roi = frame_student(cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.9, VIDEO_HEIGHT*0.4));
		//frame_roi = resize_frame;
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			cv::cvtColor(resize_frame, gray_frame, CV_RGB2GRAY);
			student_img = gray_frame.clone();
			//cv::rectangle(resize_frame, cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.45), cv::Scalar(0, 0, 255), 3);
			if (student_track_rect.area() > (resize_frame.size().area()*0.02))
				cv::rectangle(resize_frame, student_track_rect, cv::Scalar(255, 0, 0), 2);
			/////////////////////////////////////////
			// 在图片上绘制识别学生识别结果
			std::stringstream Oss;
			Oss << "Static Score : " << student_static_result.score;
			if (student_static_result.warning_flag)
			{
				during_emergency_num = 0;
				Oss << " Emergency Warning !";
			}
			else
			{
				if (during_emergency_num < 100)
				{
					Oss << " Emergency Warning !";
					during_emergency_num++;
				}
			}
			int fontFace = CV_FONT_HERSHEY_DUPLEX;
			double fontScale = 0.8;
			int fontThickness = 2;
			cv::Size fontSize = cv::getTextSize("T[]", fontFace, fontScale, fontThickness, 0);
			cv::Point org;
			org.x = 1;
			org.y = fontSize.height;
			cv::putText(resize_frame, Oss.str(), org, CV_FONT_HERSHEY_DUPLEX, fontScale, CV_RGB(255, 0, 0), fontThickness, 16);
			//////////////////////////////////////////

		}
		cv::imshow("student_video", resize_frame);

		if (cv::waitKey(30) == 27)
			break;
	}
	capture_student.release();
}

void video_read_teacher(std::string path, bool resize_flag, cv::Size resize)
{
	cv::VideoCapture capture_teacher;
	capture_teacher.open(path);
	if (!capture_teacher.isOpened())
	{
		cout << "capture_student is null" << endl;
		getchar();
		return ;
	}
	int frame_count = capture_teacher.get(7);
	capture_teacher.set(CV_CAP_PROP_POS_FRAMES, 600*25 );
	cv::Mat frame_teacher;
	cv::Mat frame_roi;
	cv::Mat gray_frame;
	cv::namedWindow("teacher_video", 1);

	cv::Mat resize_frame;
	int during_emergency_num = 0;
	while (true)
	{
		if (!capture_teacher.read(frame_teacher))
		{
			printf("video capture_student fail...\n");
			break;
		}
		if (resize_flag)
			cv::resize(frame_teacher, resize_frame, resize);
		else
			resize_frame = frame_teacher;
		
		//frame_roi = frame_teacher(cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.9, VIDEO_HEIGHT*0.4));
		//frame_roi = resize_frame;
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			cv::cvtColor(resize_frame, gray_frame, CV_RGB2GRAY);
			teacher_img = gray_frame;
			cv::rectangle(resize_frame, cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.45), cv::Scalar(0, 0, 255), 2);
			if (teacher_track_rect.area() > (resize_frame.size().area()*0.005))
				cv::rectangle(resize_frame, teacher_track_rect, cv::Scalar(255, 0, 0), 2);
			/////////////////////////////////////////
			// 在图片上绘制老师识别结果
			std::stringstream Oss;
			std::string t_status;
			switch (teacher_acitve_result.status)
			{
			case TCH_DOWN_PLATFORM:
				t_status = "TCH_DOWN_PLATFORM";
				break;
			case KEEP_DOWN_PLATFORM:
				t_status = "KEEP_DOWN_PLATFORM";
				break;
			case TCH_UP_PLATFORM:
				t_status = "TCH_UP_PLATFORM";
				break;
			case TCH_OCCUR:
				t_status = "TCH_OCCUR";
				break;
			case TCH_NULL:
				t_status = "TCH_NULL";
				break;
			case TCH_MOVING:
				t_status = "TCH_MOVING";
				break;
			case KEEP_MOVING:
				t_status = "KEEP_MOVING";
				break;
			case TCH_STAND:
				t_status = "TCH_STAND";
				break;
			case KEEP_STAND:
				t_status = "KEEP_STAND";
				break;
			default:
				break;
			}
			Oss << "Active Score: " << teacher_acitve_result.active_score;
			int fontFace = CV_FONT_HERSHEY_DUPLEX;
			double fontScale = 0.6;
			int fontThickness = 2;
			cv::Size fontSize = cv::getTextSize("T[]", fontFace, fontScale, fontThickness, 0);
			cv::Point org;
			org.x = 1;
			org.y = fontSize.height;
			cv::putText(resize_frame, Oss.str(), org, CV_FONT_HERSHEY_DUPLEX, fontScale, CV_RGB(255, 0, 0), fontThickness, 16);
			org.y = 2.5*fontSize.height;
			cv::putText(resize_frame, t_status, org, CV_FONT_HERSHEY_DUPLEX, fontScale, CV_RGB(255, 0, 0), fontThickness, 16);
			//////////////////////////////////////////

		}
		cv::imshow("teacher_video", resize_frame);

		if (cv::waitKey(30) == 27)
			break;
	}
	capture_teacher.release();
}

int main()
{

	boost::thread thread(classroom_analysis);
	boost::thread thread1(boost::bind(video_read_student,
		"D:\\FFOutput\\student_panorama.avi", false, cv::Size(100, 100)));
	boost::thread thread2(boost::bind(video_read_teacher,
		"D:\\FFOutput\\teacher_panorama~3.avi", true, cv::Size(480, 270)));

	while (1)
	{
		std::string commond;
		std::cin >> commond;
		if (commond == "exit")
			break;
	}
	boost::recursive_mutex::scoped_lock lk(cs);
	student_json_root["student_static_score"] = student_json_value;
	Json::Value moving;
	moving["name"] = "Moving On Platform";
	moving["value"] = teacher_acitve_result.moving_num;
	Json::Value down_platform;
	down_platform["name"] = "Down Platform";
	down_platform["value"] = teacher_acitve_result.down_platform_num;
	/*Json::Value up_platform;
	up_platform["name"] = "Up Platform";
	up_platform["value"] = teacher_acitve_result.up_platform_num;*/
	Json::Value stand;
	stand["name"] = "Standing On Platform";
	stand["value"] = teacher_acitve_result.stand_num;

	teacher_json_root["teacher_behavior"].append(moving);
	teacher_json_root["teacher_behavior"].append(down_platform);
	//teacher_json_root["teacher_behavior"].append(up_platform);
	teacher_json_root["teacher_behavior"].append(stand);

	teacher_json_root["teacher_score_active"] = teacher_json_value;
	time_json_root["time"] = time_json_value;
	ofstream ofs;
	ofs.open("D:\\WebProject\\combine_analysis\\echarts3.0\\student.json");
	ofs << student_json_root.toStyledString();
	ofs.close();
	ofs.open("D:\\WebProject\\combine_analysis\\echarts3.0\\teacher.json");
	ofs << teacher_json_root.toStyledString();
	ofs.close();
	ofs.open("D:\\WebProject\\combine_analysis\\echarts3.0\\time.json");
	ofs << time_json_root.toStyledString();
	ofs.close();

	/*
	cv::VideoCapture capture_student;

	capture_student.open("D:\\FFOutput\\student_panorama~6.avi");
	if (!capture_student.isOpened())
	{
		cout << "capture_student is null" << endl;
		getchar();
		return 0;
	}
	int frame_count = capture_student.get(7);
	//capture_student.set(CV_CAP_PROP_POS_FRAMES, (int)frame_count*0.9);
	cv::Mat frame_student;
	cv::Mat frame_teacher;
	cv::Mat frame_roi;
	cv::Mat gray_frame;
	cv::namedWindow("origin_video", 1);

	cv::Mat resize_frame;
	int during_emergency_num = 0;
	while (true)
	{
		if (!capture_student.read(frame_student))
		{
			printf("video capture_student fail...\n");
			break;
		}
		//cv::resize(frame_student, resize_frame, cv::Size(480, 270));
		resize_frame = frame_student;
		//frame_roi = frame_student(cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.9, VIDEO_HEIGHT*0.4));
		frame_roi = resize_frame;
		{
			boost::recursive_mutex::scoped_lock lk(cs);
			cv::cvtColor(resize_frame, gray_frame, CV_RGB2GRAY);
			student_img = gray_frame;
			//cv::rectangle(resize_frame, cv::Rect(VIDEO_WIDTH*0.1, VIDEO_HEIGHT * 0.2, VIDEO_WIDTH*0.8, VIDEO_HEIGHT*0.45), cv::Scalar(0, 0, 255), 3);
			if (student_track_rect.area() > (resize_frame.size().area()*0.005))
				cv::rectangle(frame_roi, student_track_rect, cv::Scalar(255, 0, 0), 2);
			/////////////////////////////////////////
			// 在图片上绘制识别学生识别结果
			std::stringstream Oss;
			Oss << "Static score : " << student_static_result.score;
			if (student_static_result.warning_flag)
			{
				during_emergency_num = 0;
				Oss << " Emergency Warning !";
			}
			else
			{
				if (during_emergency_num < 100)
				{
					Oss << " Emergency Warning !";
					during_emergency_num++;
				}
			}
			int fontFace = CV_FONT_HERSHEY_DUPLEX;
			double fontScale = 0.8;
			int fontThickness = 2;
			cv::Size fontSize = cv::getTextSize("T[]", fontFace, fontScale, fontThickness, 0);
			cv::Point org;
			org.x = 1;
			org.y = fontSize.height;
			cv::putText(frame_roi, Oss.str(), org, CV_FONT_HERSHEY_DUPLEX, fontScale, CV_RGB(255, 0, 0), fontThickness, 16);
			//////////////////////////////////////////

		}		

		cv::imshow("origin_video", resize_frame);

		if (cv::waitKey(30) == 27)
			break;
	}
	capture_student.release();*/

	

	return 0;
}

