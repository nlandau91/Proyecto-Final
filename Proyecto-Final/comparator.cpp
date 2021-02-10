#include "comparator.h"
#include <QDebug>

namespace fp
{
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

bool Comparator::compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, double threshold)
{
    bool comparation = false;
    std::vector<cv::DMatch> matches;
    switch(matcher_method)
    {
    case ORB:
    {
        matches = find_matches(query_descriptors,train_descriptors,cv::NORM_HAMMING,0.75);
        break;
    }
    case SURF:
    {
        //matches = find_matches(query_descriptors,train_descriptors,cv::NORM_L2,0.75);
        matches = find_matches(query_descriptors,train_descriptors,cv::NORM_L2);
        break;
    }
    }
    if(matches.size() > 0)
    {
        double score = (double)matches.size()/std::max(query_descriptors.rows,train_descriptors.rows);
        comparation = score > threshold;
        if(comparation)
        {
            qDebug() << score;
        }
    }
    return comparation;
}

bool Comparator::compare_edges(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors)
{
    bool comparation = false;
    Q_UNUSED(query_descriptors);
    Q_UNUSED(train_descriptors);
    return comparation;
}

bool Comparator::compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, double threshold)
{
    bool comparation = false;
    //qDebug() << "Comparator: obteniendo matches...";
    for(const cv::Mat &td : train_descriptors_list)
    {
        comparation = comparation || compare(query_descriptors, td, threshold);
    }
    return comparation;
}

bool Comparator::compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, const cv::Mat &query_keypoints, const std::vector<cv::Mat> &train_keypoints_list, double threshold)
{
    bool comparation = false;
    //qDebug() << "Comparator: obteniendo matches...";
    int i = 0;
    for(const cv::Mat &td : train_descriptors_list)
    {
        comparation = comparation || compare(query_descriptors, td, query_keypoints, train_keypoints_list[i], threshold);
        i++;
    }
    return comparation;
}

bool Comparator::compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, const cv::Mat &query_keypoints, const cv::Mat &train_keypoints, double threshold)
{
    bool comparation = false;
    std::vector<cv::DMatch> matches;
    switch(matcher_method)
    {
    case ORB:
    {
        matches = find_matches(query_descriptors,train_descriptors,cv::NORM_HAMMING,0.75);
        break;
    }
    case SURF:
    {
        //matches = find_matches(query_descriptors,train_descriptors,cv::NORM_L2,0.75);
        matches = find_matches(query_descriptors,train_descriptors,cv::NORM_L2);
        break;
    }
    }
    if(matches.size() > 1)
    {
        //armamos las raices
        int q_root_x = query_keypoints.at<double>(matches[0].queryIdx,0);
        int q_root_y = query_keypoints.at<double>(matches[0].queryIdx,1);
        cv::Point q_root(q_root_x,q_root_y);
        int t_root_x = train_keypoints.at<double>(matches[0].trainIdx,0);
        int t_root_y = train_keypoints.at<double>(matches[0].trainIdx,1);
        cv::Point t_root(t_root_x,t_root_y);

    }

    if(matches.size() > 0)
    {
        double score = (double)matches.size()/std::max(query_descriptors.rows,train_descriptors.rows);
        comparation = score > threshold;
        if(comparation)
        {
            qDebug() << score;
        }
    }
    return comparation;
}

}
