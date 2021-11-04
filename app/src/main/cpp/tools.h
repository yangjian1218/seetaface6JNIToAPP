//
// Created by jerry on 2021/9/17.
//

#ifndef SEETAFACE_TOOLS_H
#define SEETAFACE_TOOLS_H
#include <jni.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <seeta/Common/Struct.h>
using namespace std;
using namespace cv;


void draw_box(cv::Mat& img, SeetaRect &box);  //对单个脸画图
void draw_points(cv::Mat &img, vector<SeetaPointF> &pts);   //绘制特征点
#endif //SEETAFACE_TOOLS_H