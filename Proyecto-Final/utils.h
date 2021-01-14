#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <QImage>
#include <QPixmap>
#include "common.h"

namespace fp
{
QImage cvMatToQImage(const cv::Mat &inMat);
QPixmap cvMatToQPixmap( const cv::Mat &inMat );
cv::Mat draw_singularities( const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities);
cv::Mat draw_minutiae( const cv::Mat &src, const std::vector<cv::KeyPoint> &mintiaes);
}

#endif // UTILS_H
