#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <QImage>
#include <QPixmap>

namespace fp
{
    QImage cvMatToQImage(const cv::Mat &inMat);
    QPixmap cvMatToQPixmap( const cv::Mat &inMat );
}

#endif // UTILS_H
