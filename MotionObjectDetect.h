#ifndef MOTION_OBJECT_DETECT
#define MOTION_OBJECT_DETECT

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class MotionObjectDetect
{
public:
	MotionObjectDetect();
	~MotionObjectDetect();

public:
	
	inline void setParam(double threshold){ THRESHOLD = threshold; }

	void updataFrame(cv::Mat &frame, cv::Mat &diff);

private:
	void convert(cv::Mat &src, cv::Mat &dst);

private:
	cv::Mat last_frame;
	double THRESHOLD;
};


#endif //MOTION_OBJECT_DETECT
