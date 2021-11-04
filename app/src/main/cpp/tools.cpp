#include "tools.h"
#include <android/log.h>
using namespace cv;


void draw_box(cv::Mat &img, SeetaRect& box) {
    // 绘制单个人脸框
    cv::rectangle(img, cv::Point2i{box.x, box.y}, cv::Point2i{box.width+box.x, box.height+box.y}, cv::Scalar(0, 255, 255), 3, 8, 0);
}

void draw_points(cv::Mat &img, std::vector<SeetaPointF> &pts) {
    // 绘制特征点
    cv::Scalar color=cv::Scalar(225, 0, 225);
    if(pts.size()==68){
        color=cv::Scalar(225, 0, 0);
    }
    for (int j = 0; j < pts.size(); ++j) {
        cv::circle(img, cv::Point2d(pts[j].x, pts[j].y), 2, color, -1, 8,0);
    }
}