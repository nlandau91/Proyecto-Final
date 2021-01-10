#ifndef ANALYZER_H
#define ANALYZER_H

#include "common.h"
namespace fp
{
//clase para realizar el analisis y extraccion de caracteristicas de una huella dactilar
//se asume que las imagenes de huellas ya estan preprocesadas
class Analyzer
{
public:
    enum KeypointExtractor
    {
        HARRIS,
        SHITOMASI,
        KSURF,
    };
    enum FeatureExtractor
    {
        ORB,
        DSURF,
        DSIFT,

    };
    enum MatcherMethod
    {
        BRUTEFORCE,
        FLANN,
    };

    struct Analysis
    {
        cv::Mat fingerprint;
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
    };

    Analyzer();
    Analyzer(KeypointExtractor keypoint_method, int keypoint_threshold, FeatureExtractor descriptor_method, MatcherMethod matcher_method, int max_match_dist);

    //analiza la huella digital
    Analysis analize(cv::Mat &src);

private:
    int keypoint_threshold;
    int max_match_dist;
    KeypointExtractor keypoint_method;
    FeatureExtractor descriptor_method;
    MatcherMethod matcher_method;
    std::vector<cv::KeyPoint> calcular_keypoints(cv::Mat &src);
    cv::Mat calcular_descriptors(cv::Mat &src, std::vector<cv::KeyPoint> keypoints);
};
}
#endif // ANALYZER_H
