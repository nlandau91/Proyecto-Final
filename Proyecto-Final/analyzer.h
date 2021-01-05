#ifndef FINGERPRINTANALYZER_H
#define FINGERPRINTANALYZER_H

#include <opencv2/opencv.hpp>
namespace fp
{
//clase para realizar el analisis y extraccion de caracteristicas de una huella dactilar
//se asume que las imagenes de huellas ya estan preprocesadas
class Analyzer
{
public:
    enum KeypointMethod
    {
        HARRIS,
    };
    enum DescriptorMethod
    {
        ORB,
    };
    enum MatcherMethod
    {
        BRUTEFORCE,
    };

    struct Analysis
    {
        cv::Mat fingerprint;
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
    };

    Analyzer();
    Analyzer(KeypointMethod keypoint_method, int keypoint_threshold, DescriptorMethod descriptor_method, MatcherMethod matcher_method, int max_match_dist);

    //analiza la huella digital
    Analysis analize(cv::Mat &src);
    std::vector<cv::KeyPoint> calcular_keypoints(cv::Mat &src);

private:
    int keypoint_threshold;
    int max_match_dist;
    KeypointMethod keypoint_method;
    DescriptorMethod descriptor_method;
    MatcherMethod matcher_method;
};
}
#endif // FINGERPRINTANALYZER_H
