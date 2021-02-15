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

std::vector<Edge> get_edges(const cv::Mat &keypoints, double min_dist = 5.0)
{
    std::vector<Edge> edges;
    for(int i = 0; i < keypoints.rows - 1; i++)
    {
        //calculamos la raiz
        int root_x = keypoints.at<float>(i,0);
        int root_y = keypoints.at<float>(i,1);
        float root_angle = keypoints.at<float>(i,4);
        qDebug() << "r: " << i << ", " << root_angle;
        for(int j = i + 1; j < keypoints.rows; j++)
        {
            //armamos los arcos
            int neighbor_x = keypoints.at<float>(j,0);
            int neighbor_y = keypoints.at<float>(j,1);
            float neighbor_angle = keypoints.at<float>(j,4);
            qDebug() << "n: " << j << ", " << neighbor_angle;
            Edge edge(root_x, root_y, root_angle, neighbor_x, neighbor_y, neighbor_angle);
            if(edge.dist > min_dist)
            {
                edges.push_back(edge);
            }
        }
    }
    return edges;
}

bool Comparator::compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, const cv::Mat &query_keypoints, const cv::Mat &train_keypoints, double threshold)
{
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
    bool comparation = false;
    if(matches.size() > 1)
    {
        //armamos los arcos
        double min_dist = 10.0; // minima distancia que debe tener un arco
        std::vector<Edge> query_edges = get_edges(query_keypoints, min_dist);
        std::vector<Edge> train_edges = get_edges(train_keypoints, min_dist);

        //comparamos los arcos
        for(const Edge &e1 : query_edges)
        {
            //            qDebug() << "dist: " << e1.dist;
            //            qDebug() << "angle: " << e1.angle;
            //            qDebug() << "alpha: " << e1.alpha;
            //            qDebug() << "beta: " << e1.beta;
            for(const Edge &e2 : train_edges)
            {
                comparation = e1.compare(e2, edge_dist, edge_angle);

                //if(comparation)qDebug() << comparation;

            }
        }





        double score = (double)matches.size()/std::max(query_descriptors.rows,train_descriptors.rows);
        comparation = score > threshold;


    }

    return comparation;
}

bool Comparator::compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, const cv::Mat &query_keypoints, const std::vector<cv::Mat> &train_keypoints_list)
{
    bool comparation = false;
    //qDebug() << "Comparator: obteniendo matches...";
    int i = 0;
    for(const cv::Mat &td : train_descriptors_list)
    {
        comparation = comparation || compare(query_descriptors, td, query_keypoints, train_keypoints_list[i], match_threshold);
        i++;
    }

    return comparation;
}

}
