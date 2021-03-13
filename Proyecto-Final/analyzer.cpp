#include "analyzer.h"
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <QDebug>

namespace fp
{

Analyzer::Analyzer()
{

}

//buscamos los keypoints en una imagen con el detector de esquinas de Harris
//la imagen ya debe estar preprocesada
std::vector<cv::KeyPoint> kp_harris(const cv::Mat preprocesado, float keypoint_threshold = -1)
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
            if( keypoint_threshold == -1 || (int)harris_normalised.at<float>(y,x) > keypoint_threshold)
            {
                //guardamos el keypoint
                keypoints.push_back(cv::KeyPoint(x,y,5));
            }
        }
    }

    return keypoints;
}

std::vector<cv::KeyPoint> kp_shitomasi(const cv::Mat &src)
{
    cv::Mat kp_positions;
    cv::goodFeaturesToTrack(src,kp_positions,25,0.01,10);
    std::vector<cv::KeyPoint> keypoints;
    for(int y = 0; y < kp_positions.rows; y++)
    {
        cv::Point2f pt = kp_positions.at<cv::Point2f>(y,0);
        cv::KeyPoint keypoint = cv::KeyPoint(pt,5);
        keypoints.push_back(keypoint);
    }
    return keypoints;
}

std::vector<cv::KeyPoint> kp_sift(const cv::Mat &src)
{

    cv::Ptr<cv::SIFT> siftPtr = cv::SIFT::create();
    std::vector<cv::KeyPoint> keypoints;
    siftPtr->detect(src, keypoints);

    return keypoints;
}

std::vector<cv::KeyPoint> kp_surf(const cv::Mat &src, int hessian_threshold = 20000)
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
int crosses(const cv::Mat &bin, int col, int row)
{
    int cn = 0;
    //estamos sobre una cresta?
    if(bin.at<uchar>(row,col) == 0)
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
//src se supone que ya es esqueletizada y en rango 0-255
std::vector<cv::KeyPoint> kp_cn(const cv::Mat &src, const cv::Mat &mask)
{
    int blk_sze = 7;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat bin = src/255; //pasamos la imagen a binario

    //iteramos en toda la imagen menos un borde de 1 pixel por el kernel de 3x3
    //for(int col = 1; col < bin.cols - 1; col++)
    for(int col = blk_sze/2; col < bin.cols - blk_sze/2; col++)
    {
        //for(int row = 1; row < bin.rows - 1; row++)
        for(int row = blk_sze/2; row < bin.rows - blk_sze/2; row++)
        {
            //cv::Rect neighbours(col-1,row-1,3,3);
            cv::Rect neighbours(col-blk_sze/2,row-blk_sze/2,blk_sze,blk_sze);
            int valid_neighbours = cv::countNonZero(mask(neighbours));
            //if(valid_neighbours == 9)
            if(valid_neighbours == blk_sze*blk_sze)
            {
                int cn = crosses(bin, col, row);
                //1 representa terminacion, 3 representa bifurcacion
                if(cn == 1 || cn == 3)
                {
                    //armamos el keypoint
                    int tipo = (cn==1 ? ENDING : BIFURCATION);
                    cv::KeyPoint kp(col,row,1);
                    kp.class_id = tipo;
                    keypoints.push_back(kp);
                }
            }
        }
    }
    return keypoints;
}

/*!
 * \brief minutiae_angle calcula el angulo de una minucia a partir de su orientacion
 * \param im bloque binario de 3x3 centrado en la minucia
 * \param tipo tipo de minucia fp::ENDING o fp::BIFURCATION
 * \param o orientacion de la minucia, rango [0,pi]
 * \return devuelve angulo con respecto al horizontal, rango [0,2pi]
 */
float minutiae_angle(const cv::Mat &im, int tipo, float o)
{
    //calculamos los puntos donde hay cresta
    std::vector<cv::Point> points;
    points.push_back(cv::Point(0,0));
    points.push_back(cv::Point(1,0));
    points.push_back(cv::Point(2,0));
    points.push_back(cv::Point(2,1));
    points.push_back(cv::Point(2,2));
    points.push_back(cv::Point(1,2));
    points.push_back(cv::Point(0,2));
    points.push_back(cv::Point(0,1));
    points.push_back(cv::Point(0,0));
    points.push_back(cv::Point(1,0));

    cv::Point key_point;
    //si es terminacion, buscamos el unico punto negro que rodea la minucia
    if(tipo == fp::ENDING)
    {
        for(int i = 0; i < 8; i++)
        {
            cv::Point p = points[i];
            if(im.at<uchar>(p) == 0)
            {
                key_point = p;
            }

        }
    }
    //si es bifurcacion, buscamos el punto blanco que abre la bifurcacion
    if(tipo == fp::BIFURCATION)
    {
        for(int i = 1; i < 9; i++)
        {
            cv::Point p = points[i];
            if(im.at<uchar>(p) == 0)
            {
                if(im.at<uchar>(points[i-1]) > 0 && im.at<uchar>(points[i+1]) > 0)
                {
                    key_point = p;
                    //todo: podria tener dos puntos, en cuyo caso debo ver cual se alinea mejor con la orientacion
                    //otro metodo seria calcularlo a partir del esqueleto invertido
                }
            }

        }
    }
    int x = key_point.x;
    int y = key_point.y;
    float angle = o;
    if(y == 0)
    {
        angle += M_PI;
    }
    if(y == 1 && x == 0)
    {
        if(o > M_PI/2)
        {
            angle += M_PI;
        }
    }
    if(y == 1 && x == 2)
    {
        if(o < M_PI/2)
        {
            angle += M_PI;
        }
    }
    return angle;
}

//arma un mat a partir de una lista de keypoints
//cada row es un kp
//col 0 = pos x
//col 1 = pos y
//col 2 = tipo de minucia
//col 4 = angulo entre 0 y pi
cv::Mat get_minutiae(const Preprocessed &pre)
{
    std::vector<cv::KeyPoint> kps = kp_cn(pre.result,pre.roi);
    int blk_sze = trunc((double)pre.result.rows / pre.orientation.rows);
    cv::Mat descriptors(kps.size(),4,CV_32FC1);
    int index = 0;
    for(const cv::KeyPoint &kp : kps)
    {
        int x = kp.pt.x;
        int y = kp.pt.y;
        int tipo = kp.class_id;
        float orientation = pre.orientation.at<float>(trunc((double)kp.pt.y / blk_sze),trunc((double)kp.pt.x / blk_sze));
        float ang = minutiae_angle(pre.result(cv::Rect(x-1,y-1,3,3)),tipo,orientation);

        descriptors.at<float>(index,0) = x;
        descriptors.at<float>(index,1) = y;
        descriptors.at<float>(index,2) = tipo;
        descriptors.at<float>(index,3) = ang;
        index++;
    }

    return descriptors;
}

std::vector<cv::KeyPoint> Analyzer::get_keypoints(const Preprocessed &pre)
{
    std::vector<cv::KeyPoint> l2_features;
    switch(keypoint_method)
    {
    case HARRIS:
    {
        l2_features = kp_harris(pre.result, keypoint_threshold);

        break;
    }
    case SHITOMASI:
    {
        l2_features = kp_shitomasi(pre.result);
        break;
    }
    case SURF:
    {
        l2_features = kp_surf(pre.result);
        break;
    }
    case CN:
    {
        l2_features = kp_cn(pre.result,pre.roi);
        break;
    }
    default:
        break;
    }
    return l2_features;
}


//calcula el indice de poincare en la posicion elegida
//supone que orient esta dividido en bloques
int poincare_index_en(int row, int col, const cv::Mat orient, float tol)
{
    float to_degrees = 180.0/M_PI;

    float d1 = to_degrees * orient.at<float>(row-1,col-1);
    float d2 = to_degrees * orient.at<float>(row-1,col);
    float d3 = to_degrees * orient.at<float>(row-1,col+1);
    float d4 = to_degrees * orient.at<float>(row,col+1);
    float d5 = to_degrees * orient.at<float>(row+1,col+1);
    float d6 = to_degrees * orient.at<float>(row+1,col);
    float d7 = to_degrees * orient.at<float>(row+1,col-1);
    float d8 = to_degrees * orient.at<float>(row,col-1);

    float p_vals[8] = {d1,d2,d3,d4,d5,d6,d7,d8};
    float p_index = 0;
    for(int i = 0; i < 8;i++)
    {
        float dif = p_vals[(i+1) % 8] - p_vals[i];
        if(dif > 90)
        {
            dif -= 180;
        }
        else
        {
            if(dif < -90)
            {
                dif += 180;
            }
        }
        p_index += dif;
    }
    int result = -1;
    if(180-tol <= p_index && p_index <= 180 + tol) result = LOOP; //o core
    if(-180 - tol <= p_index && p_index <= -180+tol) result = DELTA;
    if(360-tol <= p_index && p_index <= 360+tol) result = WHORL;
    return result;

}

std::vector<cv::KeyPoint> poincare(const cv::Mat orient, const cv::Mat mask, float tol)
{

    int blk_sze = trunc((float)mask.cols / (float)orient.cols);
    int W = 16/blk_sze;
    std::vector<cv::KeyPoint> keypoints;

    for(int y = W; y < orient.rows - 1 - W; y++)
    {
        for(int x = W ; x < orient.cols - 1 - W; x++)
        {
            cv::Mat roi = mask(cv::Rect((x - W)*blk_sze, (y - W)*blk_sze, 2 * W * blk_sze, 2 * W * blk_sze));
            int valid_blocks = cv::countNonZero(roi);
            if(valid_blocks == 4*W*W*blk_sze*blk_sze)
            {
                float p_index = poincare_index_en(y,x,orient,tol);
                if(p_index != -1)
                {
                    cv::KeyPoint kp((x*blk_sze)+blk_sze/2, (y*blk_sze)+blk_sze/2, 1);
                    kp.class_id = p_index;
                    keypoints.push_back(kp);
                }
            }
        }
    }
    return keypoints;
}

std::vector<cv::KeyPoint> Analyzer::find_l1_features(const Preprocessed &pre)
{
    std::vector<cv::KeyPoint> l1_features;
    switch(l1_features_method)
    {
    case POINCARE:
    {
        l1_features = poincare(pre.orientation,  pre.roi, 1);
        break;
    }
    default:
        break;

    }
    return l1_features;
}

cv::Mat Analyzer::calcular_descriptors(const cv::Mat &src, std::vector<cv::KeyPoint> &keypoints)
{
    cv::Ptr<cv::Feature2D> extractor;
    cv::Mat descriptors;
    switch(descriptor_method)
    {
    case ORB:
    {
        extractor = cv::ORB::create();
        extractor->compute(src,keypoints,descriptors);
        break;
    }
    case BRIEF:
    {
        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
        extractor->compute(src,keypoints,descriptors);
        break;
    }
    case SURF:
    {
        extractor = cv::xfeatures2d::SURF::create();
        extractor->compute(src,keypoints,descriptors);
        break;
    }
    default:
        break;
    }

    return descriptors;
}


fp::Analysis Analyzer::analize(const fp::Preprocessed &preprocessed)
{

    Analysis analysis;
    cv::bitwise_not(preprocessed.result,analysis.fingerprint);
    //analysis.fingerprint = preprocessed.result.clone();
    //buscamos las features de nivel 1
    qDebug() << "Analyzer: buscando singularidades...";
    analysis.l1_features = find_l1_features(preprocessed);
    //buscamos los puntos minuciosos (minutae)
    qDebug() << "Analizer: buscando minutiae...";
    analysis.keypoints = get_keypoints(preprocessed);
    analysis.minutiae = get_minutiae(preprocessed);
    //calculamos sus descriptores
    qDebug() << "Analizer: calculando descriptores...";
    //analysis.descriptors = calcular_descriptors(analysis.fingerprint, analysis.l2_features);
    analysis.descriptors = calcular_descriptors(analysis.fingerprint, analysis.keypoints);
    qDebug() << "Analizer: listo.";
    return analysis;
}
}
