#ifndef COMMON_H
#define COMMON_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/ximgproc.hpp>
#include <iostream>

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
    cv::Mat keypoints;
};

}

#endif // COMMON_H
