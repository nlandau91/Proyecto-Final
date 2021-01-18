#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QPixmap>
#include "common.h"

namespace fp
{

QImage cvMatToQImage(const cv::Mat &inMat);
QPixmap cvMatToQPixmap( const cv::Mat &inMat );
cv::Mat draw_singularities( const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities);
cv::Mat draw_minutiae( const cv::Mat &src, const std::vector<cv::KeyPoint> &mintiaes);
cv::Mat mat_cos( const cv::Mat &src);
cv::Mat mat_sin( const cv::Mat &src);
cv::Mat mat_atan2( const cv::Mat &src1, const cv::Mat &src2);
std::vector<float> unique(const cv::Mat &input, bool sort = false);
cv::Mat visualize_angles( const cv::Mat &im, const cv::Mat &angles, int W);
cv::Mat apply_mask(const cv::Mat &src, const cv::Mat &mask);
cv::Mat translate_mat(const cv::Mat &img, int offsetx, int offsety);

}

#endif // UTILS_H
