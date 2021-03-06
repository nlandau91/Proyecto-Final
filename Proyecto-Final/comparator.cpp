#include "comparator.h"
#include "appsettings.h"
#include <opencv2/calib3d.hpp>
#include <QDebug>

using namespace fp;
Comparator::Comparator()
{

}

//busca matches entre descriptores de consulta y descriptores de entrenamiento
std::vector<cv::DMatch> find_matches(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, int norm_type)
{
    cv::BFMatcher matcher = cv::BFMatcher(norm_type, true);
    std::vector<cv::DMatch> matches;
    matcher.match(query_descriptors,train_descriptors,matches);
    return matches;
}

//busca matches entre descriptores de una imagen y descriptores de entrenamiento
//utiliza el test del ratio de lowe
std::vector<cv::DMatch> find_matches(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, int norm_type, float lowe_ratio)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(norm_type);
    // Now loop over the database and start the matching
    std::vector<cv::DMatch> good_matches;
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(query_descriptors, train_descriptors,matches,2);
    for(std::vector<cv::DMatch> &match_pair : matches)
    {
        //test de ratio de lowe
        if(match_pair[0].distance < lowe_ratio*match_pair[1].distance)
        {
            good_matches.push_back(match_pair[0]);
        }
    }

    return good_matches;
}

bool compare_singularities(const std::vector<cv::KeyPoint> &sing1, const std::vector<cv::KeyPoint> &sing2)
{
    int tipo1 = -1;
    int deltas1 = 0;
    int cores1 = 0;
    int whorls1 = 0;
    for(const cv::KeyPoint &kp : sing1)
    {
        if((int)kp.class_id == fp::DELTA)
        {
            deltas1++;
        }
        if((int)kp.class_id == fp::LOOP)
        {
            cores1++;
        }
        if((int)kp.class_id == fp::WHORL)
        {
            whorls1++;
        }
    }
    deltas1 = round(deltas1 / 4.0);
    cores1 = round(cores1 / 4.0);
    if(deltas1 == 0 && cores1 == 0)
    {
        tipo1 = ARCH;
    }
    if(cores1 >= 2 || whorls1 >= 1)
    {
        tipo1 = WHORL;
    }
    if(cores1 == 1)
    {
        tipo1 = LOOP;
    }

    int tipo2 = -1;
    int deltas2 = 0;
    int cores2 = 0;
    int whorls2 = 0;
    for(const cv::KeyPoint &kp : sing2)
    {
        if((int)kp.class_id == fp::DELTA)
        {
            deltas2++;
        }
        if((int)kp.class_id == fp::LOOP)
        {
            cores2++;
        }
        if((int)kp.class_id == fp::WHORL)
        {
            whorls2++;
        }
    }
    deltas2 = round(deltas2 / 4.0);
    cores2 = round(cores2 / 4.0);
    if(deltas2 == 0 && cores2 == 0)
    {
        tipo2 = ARCH;
    }
    if(cores2 >= 2 || whorls2 >= 1)
    {
        tipo2 = WHORL;
    }
    if(cores2 == 1)
    {
        tipo2 = LOOP;
    }
    bool iguales = false;
    //iguales = (tipo1 == tipo2) || tipo1 == -1 || tipo2 == -1;
    iguales = !(tipo1 == ARCH && (tipo2 == WHORL || tipo2 == LOOP)) ||
            !((tipo1 == WHORL || tipo2 == LOOP) && tipo2 == ARCH);

    return iguales;
}

std::vector<cv::DMatch> remove_outliers_median(const std::vector<cv::DMatch> &matches, double ratio = 2.0)
{
    //nos quedamos con los que tengan una distancia menor a un multiplo de la media
    double median = 0.0;
    for(const cv::DMatch &m : matches)
    {
        median += m.distance;
    }
    median /= (double)matches.size();
    std::vector<cv::DMatch> good_matches;
    for(const cv::DMatch &m : matches)
    {
        if(m.distance <= ratio * median)
        {
            good_matches.push_back(m);
        }
    }
    return good_matches;
}

std::vector<cv::DMatch> remove_outliers_ransac(const std::vector<cv::KeyPoint> &query_keypoints, const std::vector<cv::KeyPoint> &train_keypoints, const std::vector<cv::DMatch> &matches, int transform = HOMOGRAPHY, double ransac_threshold = 3.0, int max_iter = 2000, double conf = 0.995)
{
    std::vector<cv::DMatch> good_matches;
    std::vector<cv::Point> src_points;
    std::vector<cv::Point> dst_points;
    for(const cv::DMatch &m : matches)
    {
        src_points.push_back(query_keypoints[m.queryIdx].pt);
        dst_points.push_back(train_keypoints[m.trainIdx].pt);
    }
    //usamos findHomography o estimateAffine2D? ambos usan ransac
    cv::Mat mask;
    switch(transform)
    {
    case HOMOGRAPHY:
    {
        cv::findHomography(src_points,dst_points,cv::RANSAC, ransac_threshold ,mask, max_iter, conf);
        break;
    }
    case AFFINE:
    {
        cv::estimateAffine2D(src_points,dst_points,mask,cv::RANSAC,ransac_threshold,max_iter, conf);
        break;
    }
    case PARTIALAFFINE:
    {
        cv::estimateAffinePartial2D(src_points,dst_points,mask,cv::RANSAC,ransac_threshold,max_iter,conf);
        break;
    }
    }

    for(int i = 0; i < mask.rows; i++)
    {
        if(mask.at<uchar>(i,0) > 0)
        {
            good_matches.push_back(matches[i]);
        }
    }
    return good_matches;
}

double Comparator::compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template) const
{
    qDebug() << "Comparator: Realizando matches entre keypoints...";
    double score = 0.0;
    //testing: si son distinta singularidad, devolvemos falso de una
    //solo se hace si ambas tienen singularidad
    if(sing_compare && !compare_singularities(query_template.singularities, train_template.singularities))
    {
        return score;
    }
    std::vector<cv::DMatch> matches;

    switch(matcher_method)
    {
    case ORB:
    {
        //matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_HAMMING,0.75);
        matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_HAMMING);
        break;
    }
    case SURF:
    {
        //matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_L2,0.75);
        matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_L2);
        break;
    }
    case SIFT:
    {
        //matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_L2,0.75);
        matches = find_matches(query_template.descriptors,train_template.descriptors,cv::NORM_L2);
    }
    }
    if(matches.size() > 4)
    {
        qDebug() << "Comparator: Analizando matches entre keypoints... ";
        //limpiamos los matches malos
        //ordenamos de forma creciente por distancia
        //        std::sort(matches.begin(),matches.end(),[] (cv::DMatch const& m1, cv::DMatch const& m2) -> bool
        //        {
        //            return m1.distance < m2.distance;
        //        });
        //limpiamos los outliers
        std::vector<cv::DMatch> inliners_median;
        inliners_median = remove_outliers_median(matches,median_threshold);
        if(inliners_median.size() > 4)
        {
            std::vector<cv::DMatch> inliners_ransac;
            inliners_ransac = remove_outliers_ransac(query_template.keypoints,train_template.keypoints,inliners_median,ransac_model, ransac_threshold,ransac_iter,ransac_conf);
            std::vector<cv::DMatch> good_matches = inliners_ransac;

            //metodo basico de matching, utilizando simplemente la cantidad de matches encontrados entre minutiae
            score = (double)good_matches.size()/std::max(query_template.descriptors.rows,train_template.descriptors.rows);
            //score = (double)good_matches.size()/((double)(query_template.descriptors.rows+train_template.descriptors.rows)/2.0);
            qDebug() << "Comparator: score: " << score;
            //qDebug() << "Comparator: score2: " << score2;
        }

    }

    return score;
}
