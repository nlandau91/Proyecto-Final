#ifndef COMMON_H
#define COMMON_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/ximgproc.hpp>
#include <iostream>
#include <cmath>

namespace fp
{
enum Params
{
    NONE,
    GABOR,
    ZHANGSUEN,
    MORPH,
    GUOHALL,
    HARRIS,
    SHITOMASI,
    SURF,
    ORB,
    CUSTOM,
    SIFT,
    BRIEF,
    BRUTEFORCE,
    FLANN,
    CN,
    ENDING,
    BIFURCATION,
    POINCARE,
    LOOP,
    DELTA,
    WHORL,
    ARCH,
    HOMOGRAPHY,
    PARTIALAFFINE,
    AFFINE,

};

struct Preprocessed
{
    cv::Mat original;
    cv::Mat normalized;
    cv::Mat roi;
    cv::Mat orientation;
    cv::Mat frequency;
    cv::Mat filtered;
    cv::Mat thinned;
    cv::Mat grayscale;
    cv::Mat result;
};


}

#endif // COMMON_H
