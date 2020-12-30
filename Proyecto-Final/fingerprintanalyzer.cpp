#include "fingerprintanalyzer.h"

FingerprintAnalyzer::FingerprintAnalyzer()
{

}

//buscamos los keypoints en una imagen con el detector de esquinas de Harris
//la imagen ya debe estar preprocesada
std::vector<cv::KeyPoint> obtenerKeyPoints(cv::Mat preprocesado, float threshold = 130.0)
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
            if((int)harris_normalised.at<float>(y,x) > threshold)
            {
                //guardamos el keypoint
                keypoints.push_back((cv::KeyPoint(x,y,1)));
            }
        }
    }
    return keypoints;
}

cv::Mat calcular_descriptores(cv::Mat &src)
{
    //buscamos los puntos minuciosos (minutae)
    std::vector<cv::KeyPoint> keypoints;
    keypoints = obtenerKeyPoints(src);
    //obtenemos los descriptores alrededor de esos puntos
    cv::Ptr<cv::Feature2D> orb_descriptor = cv::ORB::create();
    cv::Mat descriptors;
    orb_descriptor->compute(src,keypoints,descriptors);
    return descriptors;
}

FingerprintAnalyzer::FingerprintAnalysis FingerprintAnalyzer::analize(cv::Mat &src, FingerprintAnalyzer::KeypointMethod keypoint_method, FingerprintAnalyzer::DescriptorMethod descriptor_method)
{
    FingerprintAnalysis analysis;
    analysis.fingerprint = src.clone();
    //buscamos los puntos minuciosos (minutae)
    switch(keypoint_method)
    {
    case HARRIS:
        analysis.keypoints = obtenerKeyPoints(analysis.fingerprint);
        break;
    default:
        break;
    }
    //obtenemos los descriptores alrededor de esos puntos
    switch(descriptor_method)
    {
    case ORB:
    {
        cv::Ptr<cv::Feature2D> orb_descriptor = cv::ORB::create();
        orb_descriptor->compute(analysis.fingerprint,analysis.keypoints,analysis.descriptors);
        break;
    }
    default:
        break;
    }
    return analysis;
}
