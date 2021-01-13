#ifndef COMMON_H
#define COMMON_H

#include <opencv2/opencv.hpp>

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
    SIFT,
    BRUTEFORCE,
    FLANN,
    CN,
    ENDING,
    BIFURCATION,
    POINCARE,
    LOOP,
    DELTA,
    WHORL,

};

struct Preprocessed
{
    cv::Mat original;
    cv::Mat normalized;
    cv::Mat filtered;
    cv::Mat thinned;
    cv::Mat orientation;
    cv::Mat roi;
    cv::Mat result;
};

struct Analysis
{
    cv::Mat fingerprint;
    std::vector<cv::KeyPoint> l1_features;
    std::vector<cv::KeyPoint> l2_features;
    cv::Mat descriptors;
};

}

#endif // COMMON_H
