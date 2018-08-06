========================================================================
    控制台应用程序：ClassroomAnalysis 项目概述
========================================================================

应用程序向导已为您创建了此 ClassroomAnalysis 应用程序。

本文件概要介绍组成 ClassroomAnalysis 应用程序的每个文件的内容。

MotionObjectDetect.h MotionObjectDetect.cpp
    运动物体检测类，继承该类可以拓展出基于运动检测的不同功能的子类。在这里作为StudentStaticAnalysis类和TeacherTrack类的父类。
    
StudentStaticAnalysis.h StudentStaticAnalysis.cpp 
    学生运动幅度检测和突发事件预警类。继承于MotionObjectDetect类。
    
TeacherTrack.h TeacherTrack.cpp
    老师运动跟踪检测类。继承于MotionObjectDetect类。

ClassroomAnalysis.cpp
    这是主应用程序源文件。用来读取视频文件，并进行图像识别检测。


/////////////////////////////////////////////////////////////////////////////
其他注释:


/////////////////////////////////////////////////////////////////////////////
