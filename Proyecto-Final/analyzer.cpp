#include "analyzer.h"
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <QDebug>

namespace fp
{

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

/*!
 * \brief crosses calcula si el el pixel corresponde a una minucia
 * \param bin imagen binaria a procesar
 * \param col columna del pixel
 * \param row fila del pixel
 * \return devuelve 1 si es un ridge ending o 3 si es un crossing o calquier otro valor
 * si no es una minucia
 */
int crosses(cv::Mat &bin, int col, int row)
{
    int cn = 0;
    //el pixel del medio es blanco (cresta, esta invertido)
    if(bin.at<uchar>(row,col) == 1)
    {
        int p1 = bin.at<uchar>(row-1,col-1);
        int p2 = bin.at<uchar>(row-1,col);
        int p3 = bin.at<uchar>(row-1,col+1);
        int p4 = bin.at<uchar>(row,col+1);
        int p5 = bin.at<uchar>(row+1,col+1);
        int p6 = bin.at<uchar>(row+1,col);
        int p7 = bin.at<uchar>(row+1,col-1);
        int p8 = bin.at<uchar>(row,col-1);
        int p0 = p8;
        int p_vals[9] = {p0,p1,p2,p3,p4,p5,p6,p7,p8};
        for(int i = 1; i < 9;i++)
        {
            cn = cn + abs(p_vals[i] - p_vals[i-1]);
        }
        cn = cn/2;
    }
    return cn;
}

//encuentra las minutiae en una huella digital
//src se supone que ya es esqueletizada,invertida y en rango 0-255
std::vector<cv::KeyPoint> kp_cn(cv::Mat &src)
{
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat bin = src/255; //pasamos la imagen a binario

    //iteramos en toda la imagen menos un borde de 1 pixel por el kernel de 3x3
    for(int col = 1; col < bin.cols - 1; col++)
    {
        for(int row = 1; row < bin.rows - 1; row++)
        {
            int cn = crosses(bin, col, row);
            //1 representa terminacion, 3 representa bifurcacion
            if(cn == 1 || cn == 3)
            {
                //guardamos el valor de cn como tamanio, para poder discernir luego si era terminacion o bifurcacion
                cv::KeyPoint kp(col,row,cn);
                keypoints.push_back(kp);
            }
        }
    }
    return keypoints;
}

std::vector<cv::KeyPoint> Analyzer::find_l2_features(cv::Mat &src)
{
    std::vector<cv::KeyPoint> l2_features;
    switch(l2_features_method)
    {
    case HARRIS:
    {
        l2_features = kp_harris(src, keypoint_threshold);
        break;
    }
    case SHITOMASI:
    {
        l2_features = kp_shitomasi(src);
        break;
    }
    case SURF:
    {
        l2_features = kp_surf(src);
        break;
    }
    case CN:
    {
        l2_features = kp_cn(src);
        break;
    }
    default:
        break;
    }
    return l2_features;
}

std::vector<cv::KeyPoint> poincare(cv::Mat src, cv::Mat orient, cv::Mat mask, float tol, int blk_sze)
{

}

std::vector<cv::KeyPoint> Analyzer::find_l1_features(cv::Mat &src)
{
    std::vector<cv::KeyPoint> l1_features;
    switch(l1_features_method)
    {
    case POINCARE:
    {
        l1_features = poincare(src, orient,  mask,  poincare_tol, blk_sze);
        break;
    }
    default:
        break;

    }
    return l1_features;
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
    analysis.l2_features = find_l2_features(analysis.fingerprint);
    //calculamos sus descriptores
    analysis.descriptors = calcular_descriptors(analysis.fingerprint, analysis.l2_features);
    return analysis;
}
}
