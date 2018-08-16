#include "TeacherTrack.h"
#include <boost/timer.hpp>

TeacherTrack::TeacherTrack() :
erode_size(2, 2), dilate_size(5, 5),
active_score(0.0), reduction_factor(5.1)
{

	erode_element = cv::getStructuringElement(cv::MORPH_RECT, erode_size);
	dilate_element = cv::getStructuringElement(cv::MORPH_RECT, dilate_size);
	track_area_ = cv::Rect(0, 0, 0, 0);
	current_tch_status[2] = TCH_STAND;
	current_tch_status[1] = TCH_NULL;
	current_tch_status[0] = TCH_DOWN_PLATFORM;

}

TeacherTrack::~TeacherTrack()
{

}

void TeacherTrack::compute(cv::Mat &frame, cv::Rect &rect, detection_teacher_t &status)
{
	if (0 == frame.rows || 0 == frame.cols)
		return;

	cv::Mat gray;
	convert(frame, gray);

	cv::Mat roi_img;
	roiExtract(gray, roi_img, mask_);
    //cv::imshow("ROI", roi_img);

	boost::timer t;
	cv::Mat diff;
	updataFrame(roi_img, diff);

	//cv::imshow("diff_video", diff);

	cv::Mat pre_img;
	preprocess(diff, pre_img);

	//cv::imshow("pre_img", pre_img);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(pre_img, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	/*
	std::vector<std::vector<cv::Point>> contours2;
	std::vector<cv::Vec4i> hierarchy2;
	cv::findContours(pre_img, contours2, hierarchy2, cv::RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	*/

	// 寻找最大连通域
	double maxArea = 0;
	std::vector<cv::Point> maxContour;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxContour = contours[i];
		}
	}
	// 将轮廓转为矩形框
	if (maxContour.size() != 0)
		rect = cv::boundingRect(maxContour);
	else
		rect = cv::Rect(0, 0, 0, 0);

	cv::Mat dstImage = cv::Mat::zeros(frame.size(), CV_8UC3);
	for (int i = 0; i < hierarchy.size(); i++)
	{
		cv::Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
		drawContours(dstImage, contours, i, color, 3, 8, hierarchy);
	}
    //cv::imshow("仅外轮廓图", dstImage);
	/*
	cv::Mat dstImage2 = cv::Mat::zeros(frame.size(), CV_8UC3);
	for (int i = 0; i < hierarchy2.size(); i++)
	{
	cv::Scalar color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
	drawContours(dstImage2, contours2, i, color, 3, 8, hierarchy2);
	}
	cv::imshow("内外轮廓图", dstImage2);*/

	if (track_area_.area() == 0)//判断老师跟踪区域是否存在
		return;
	////***每30ms检测一帧时执行该函数***////
	/*if (maxContour.size() != 0)
	{
		switch (current_tch_status)
		{
		case TCH_DOWN_PLATFORM:
		{
			if (rect.area() >= (frame.size().area()*0.01))
			{
				if (((rect.br().y + rect.tl().y) / 2) >= track_area_.br().y)
				//if (rect.tl().y >= track_area_.br().y)
				{
					current_tch_status = TCH_KEEP_DOWN_PLATFORM;
					std::cout << "keep down platform" << std::endl;
				}
				else
				{
					current_tch_status = TCH_UP_PLATFORM;
					std::cout << "up platform" << std::endl;
				}
			}
			else
			{
				current_tch_status = TCH_KEEP_DOWN_PLATFORM;
				std::cout << "keep down platform" << std::endl;
			}
			break;
		}
		case TCH_UP_PLATFORM:
		case TCH_MOVING:
		case TCH_STAND:
		{
			if (rect.area() >= (frame.size().area()*0.01))
			{
				if (((rect.br().y + rect.tl().y) / 2) >= track_area_.br().y)
				//if (rect.tl().y >= track_area_.br().y)
				{
					current_tch_status = TCH_DOWN_PLATFORM;

					std::cout << "down platform" << std::endl;
				}
				else
				{
					current_tch_status = TCH_MOVING;

					std::cout << "keep moving" << std::endl;
				}
			}
			else
			{
				current_tch_status = TCH_STAND;
				std::cout << "keep standing" << std::endl;
			}
			break;
		}
		case TCH_KEEP_DOWN_PLATFORM:
		{
			if (rect.area() >= (frame.size().area()*0.01))
			{
				if (((rect.br().y + rect.tl().y) / 2) >= track_area_.br().y)
				//if (rect.tl().y >= track_area_.br().y)
				{
					current_tch_status = TCH_KEEP_DOWN_PLATFORM;
					std::cout << "keep down platform" << std::endl;
				}
				else
				{
					current_tch_status = TCH_UP_PLATFORM;
					std::cout << "up platform" << std::endl;
				}
			}
			else
			{
				current_tch_status = TCH_KEEP_DOWN_PLATFORM;
				std::cout << "keep down platform" << std::endl;
			}
			break;
		}
		default:
			break;
		}
	}*/

	////***每1s检测一帧时执行该函数***///
	/// 建议采用每1s检测一帧，这样可以实时跟踪老师所在区域，且减少了计算量
	if (rect.area() > 0)
	{
		if (rect.area() >= (frame.size().area()*0.01))
		{
			current_tch_status[1] = TCH_OCCUR;
			double distance;
			cv::Point last_center((last_active_area.br().x + last_active_area.tl().x) / 2, (last_active_area.br().y + last_active_area.tl().y) / 2);
			cv::Point current_center((rect.br().x + rect.tl().x) / 2, (rect.br().y + rect.tl().y) / 2);

			if ((track_area_.br().y - current_center.y ) < (frame.cols*0.03))
            {
				current_tch_status[0] = TCH_DOWN_PLATFORM;
                //status.down_platform_num++;
            }
			else
            {
				current_tch_status[0] = TCH_UP_PLATFORM;
               // status.up_platform_num++;
            }

			distance = sqrt((last_center.x - current_center.x)*(last_center.x - current_center.x) +
							(last_center.y - current_center.y)*(last_center.y - current_center.y));
			if (distance > (frame.rows*0.1))
			{
				last_active_area = rect;
				current_tch_status[2] = TCH_MOVING;
                //status.moving_num++;
			}
			else
            {
                //status.stand_num++;
				current_tch_status[2] = TCH_STAND;
            }

		}
		else
		{
			current_tch_status[1] = TCH_NULL;
		}

	}
	else
	{
		current_tch_status[1] = TCH_NULL;
	}

    statusAnalysis(rect, status);
	memcpy(last_tch_status, current_tch_status, 3 * sizeof(TEACHER_STATUS));
	//std::cout << t.elapsed() * 1000 << "ms" << std::endl;

	
}

void TeacherTrack::preprocess(cv::Mat &src, cv::Mat &dst)
{
	cv::Mat temp;
	//先腐蚀
	//erode(src, temp, erode_element);
	//后膨胀
	dilate(src, dst, dilate_element);
}

void TeacherTrack::convert(cv::Mat &src, cv::Mat &dst)
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

void TeacherTrack::roiExtract(cv::Mat &src, cv::Mat &dst, std::vector<cv::Rect> &mask)
{
	if (src.size() != dst.size())
	{
		dst.create(src.size(), src.type());
	}
	cv::Mat mask_mat(src.size(), CV_8UC1, 1);
	for (int i = 0; i < mask.size(); i++)
	{
		cv::rectangle(mask_mat, mask[i], cv::Scalar(0), CV_FILLED);
	}
	dst = src.mul(mask_mat);
}

int64_t TeacherTrack::getCurrentStamp64()
{
	boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
	boost::posix_time::time_duration time_from_epoch = boost::posix_time::second_clock::universal_time() - epoch;
	return time_from_epoch.total_milliseconds();
}

void TeacherTrack::statusAnalysis(cv::Rect &active_area, detection_teacher_t &result)
{
	if (last_tch_status[0] == TCH_UP_PLATFORM)
	{
		if (current_tch_status[0] == TCH_UP_PLATFORM)
		{
			if (last_tch_status[1] == TCH_OCCUR)
			{
				if (current_tch_status[1] == TCH_OCCUR)
				{
					if (last_tch_status[2] == TCH_STAND)
					{
						if (current_tch_status[2] == TCH_MOVING)
						{
                            active_score += (3 * reduction_factor);//从站立变成移动，则+(3 * reduction_factor)热情度
							COUT("TCH_MOVING");
							result.status = TCH_MOVING;
							result.moving_num++;
							//std::cout << "TCH_MOVING" << std::endl;
						}
						else
						{
							COUT("KEEP_STAND")
							result.status = KEEP_STAND;
							result.stand_num++;
                            active_score += (reduction_factor+0.2 );//如果保持站立，则+(reduction_factor + 0.2)热情度
						}
					}
					else
					{
						if (current_tch_status[2] == TCH_STAND)
						{
                            active_score += (reduction_factor+0.2 );//从移动变成站立，则+(reduction_factor + 0.2)热情度
							COUT("TCH_STAND");
							result.status = TCH_STAND;
							result.stand_num++;
							//std::cout << "TCH_STAND" << std::endl;
						}
						else
						{
							COUT("KEEP_MOVING");
							result.status = KEEP_MOVING;
							result.moving_num++;
                            active_score += (2 * reduction_factor);//如果保持移动，则+(2 * reduction_factor)热情度
						}
					}
				}
				else
				{		
					COUT("TCH_NULL")
					result.status = TCH_NULL;
					result.stand_num++;
					//std::cout << "TCH_DOWN_PLATFORM" << std::endl;
				}
			}
			else
			{
				if (current_tch_status[1] == TCH_OCCUR)
				{
					active_score += (reduction_factor + 1);//从消失变成出现，则+ (reduction_factor + 1)热度
					COUT("TCH_OCCUR");
					result.status = TCH_OCCUR;
				}
			}
		}
		else
		{
			active_score += (2 * reduction_factor);//从上讲台变成下讲台，则+(2 * reduction_factor)热情度
            //status.down_platform_num++;
			COUT("TCH_DOWN_PLATFORM");
			result.status = TCH_DOWN_PLATFORM;
			result.down_platform_num++;
			//std::cout << "TCH_NULL" << std::endl;
		}
	}         
	else
	{
		if (current_tch_status[0] == TCH_UP_PLATFORM)
		{
            active_score += (reduction_factor);//从下讲台变成上讲台，则+(reduction_factor)热情度
			COUT("TCH_UP_PLATFORM");
			result.status = TCH_UP_PLATFORM;
			result.moving_num++;
			//std::cout << "TCH_UP_PLATFORM" << std::endl;
		}
		else
		{
            active_score += (reduction_factor-0.2);//如果保持下讲台状态，+reduction_factor - 0.2热情度
			result.status = KEEP_DOWN_PLATFORM;
			result.down_platform_num++;
			COUT("KEEP_DOWN_PLATFORM");
		}
	}

	active_score -= reduction_factor;//固定每帧-reduction_factor热情度
	if (active_score > 100)
		active_score = 100;
	if (active_score < 0)
		active_score = 0;
	result.active_score = active_score;

	//std::cout << "active_score:" << active_score << std::endl;

}
