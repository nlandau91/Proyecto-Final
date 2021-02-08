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
    for(std::vector<cv::DMatch> match_pair : matches)
    {
        //test de ratio de lowe
        if(match_pair[0].distance < lowe_ratio*match_pair[1].distance)
        {
            good_matches.push_back(match_pair[0]);
        }
    }

    return good_matches;
}

double Comparator::compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors)
{
    double score = 999;
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
        score = 0;
        for(cv::DMatch m : matches)
        {
            score += m.distance;
        }
        score/=matches.size();
        qDebug() << score;
    }
    return score;
}

double Comparator::compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list)
{
    qDebug() << "Comparator: obteniendo matches...";
    double lowest_score = 999;
    for(cv::Mat td : train_descriptors_list)
    {
        double current = compare(query_descriptors, td);
        if(current < lowest_score)
        {
            lowest_score = current;
        }
    }
    return lowest_score;
}

}
