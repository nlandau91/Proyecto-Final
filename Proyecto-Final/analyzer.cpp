#include "analyzer.h"
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <QDebug>

namespace fp
{
Analyzer::Analyzer()
{
    Analyzer(HARRIS, 130, ORB, BRUTEFORCE, 80);
}

Analyzer::Analyzer(int keypoint_method, int keypoint_threshold, int descriptor_method, int matcher_method, int max_match_dist)
{
    this->keypoint_threshold = keypoint_threshold;
    this->max_match_dist = max_match_dist;
    this->keypoint_method = keypoint_method;
    this->matcher_method = matcher_method;
    this->descriptor_method = descriptor_method;
}

//buscamos los keypoints en una imagen con el detector de esquinas de Harris
//la imagen ya debe estar preprocesada
std::vector<cv::KeyPoint> kp_harris(cv::Mat preprocesado, float keypoint_threshold)
{
    cv::Mat harris_corners, harris_normalised;
    harris_corners = cv::Mat::zeros(preprocesado.size(), CV_32FC1);
    cv::cornerHarris(preprocesado,harris_corners,2,3,0.04,cv::BORDER_DEFAULT);
    cv::normalize(harris_corners,harris_normalised,0,255,cv::NORM_MINMAX,CV_32FC1,cv::Mat());
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat rescaled;
    cv::convertScaleAbs(harris_normalised,rescaled);
    cv::Mat harris_c(rescaled.rows,rescaled.cols,CV_8UC3);
    cv::Mat in[] = {rescaled,rescaled,rescaled};
    int from_to[] = {0,0,1,1,2,2};
    cv::mixChannels(in,3,&harris_c,1,from_to,3);
    for(int x = 0; x < harris_normalised.cols; x++)
    {
        for(int y = 0; y < harris_normalised.rows; y++)
        {
            if((int)harris_normalised.at<float>(y,x) > keypoint_threshold)
            {
                //guardamos el keypoint
                keypoints.push_back(cv::KeyPoint(x,y,5));
            }
        }
    }

    return keypoints;
}

std::vector<cv::KeyPoint> kp_shitomasi(cv::Mat &src)
{
    cv::Mat kp_positions;
    cv::goodFeaturesToTrack(src,kp_positions,25,0.01,10);
    std::vector<cv::KeyPoint> keypoints;
    for(int x = 0; x < kp_positions.rows; x++)
    {
        cv::Point2f pt = kp_positions.at<cv::Point2f>(x,0);
        cv::KeyPoint keypoint = cv::KeyPoint(pt,5);
        keypoints.push_back(keypoint);
    }
    return keypoints;
}

std::vector<cv::KeyPoint> kp_sift(cv::Mat &src)
{

    cv::Ptr<cv::SIFT> siftPtr = cv::SIFT::create();
    std::vector<cv::KeyPoint> keypoints;
    siftPtr->detect(src, keypoints);

    return keypoints;
}

std::vector<cv::KeyPoint> kp_surf(cv::Mat &src, int hessian_threshold = 20000)
{

    cv::Ptr<cv::xfeatures2d::SURF> surf = cv::xfeatures2d::SURF::create(hessian_threshold);
    std::vector<cv::KeyPoint> keypoints;
    surf->detect(src,keypoints);

    return keypoints;
}

std::vector<cv::KeyPoint> Analyzer::calcular_keypoints(cv::Mat &src)
{
    std::vector<cv::KeyPoint> keypoints;
    switch(keypoint_method)
    {
    case HARRIS:
    {
        keypoints = kp_harris(src, keypoint_threshold);
        break;
    }
    case SHITOMASI:
    {
        keypoints = kp_shitomasi(src);
        break;
    }
    case SURF:
    {
        //keypoints = kp_sift(src);
        keypoints = kp_surf(src);
        break;
    }
    default:
        break;
    }
    return keypoints;
}

cv::Mat Analyzer::calcular_descriptors(cv::Mat &src, std::vector<cv::KeyPoint> keypoints)
{
    cv::Mat descriptors;
    switch(descriptor_method)
    {
    case ORB:
    {
        cv::Ptr<cv::Feature2D> orb_descriptor = cv::ORB::create();
        orb_descriptor->compute(src,keypoints,descriptors);
        break;
    }
    case SURF:
    {
        break;
    }
    case SIFT:
    {
        break;
    }
    default:
        break;
    }

    return descriptors;
}


Analyzer::Analysis Analyzer::analize(cv::Mat &src)
{

    Analysis analysis;
    analysis.fingerprint = src.clone();
    //buscamos los puntos minuciosos (minutae)
    analysis.keypoints = calcular_keypoints(analysis.fingerprint);
    //calculamos sus descriptores
    analysis.descriptors = calcular_descriptors(analysis.fingerprint, analysis.keypoints);
    return analysis;
}
}
