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

std::vector<Edge> get_edges()
{
    std::vector<Edge> edges;

    return edges;
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
        qDebug() << "Comparator: " << matches.size() << " matches encontrados.";
        double ratio = 0.8; //maxima diferencia entre edges
        double min_dist = 10.0; // minima distancia que debe tener un arco

        //buscamos el match con la minima distancia
        int root_query_index = 0;
        int root_train_index = 0;
        double best_dist = 999;
        for(cv::DMatch m : matches)
        {
            if(m.distance < best_dist)
            {
                best_dist = m.distance;
                root_query_index = m.queryIdx;
                root_train_index = m.trainIdx;
            }
        }

        //calculamos los puntos raiz
        int root_query_x = query_keypoints.at<double>(root_query_index,0);
        int root_query_y = query_keypoints.at<double>(root_query_index,1);
        double root_query_angle = query_keypoints.at<double>(root_query_index,4);
        int root_train_x = train_keypoints.at<double>(root_train_index,0);
        int root_train_y = train_keypoints.at<double>(root_train_index,1);
        double root_train_angle = train_keypoints.at<double>(root_train_index,4);
        for(size_t i = 0; i < matches.size(); i++)
        {
            cv::DMatch m = matches[i];
            if(!(m.queryIdx == root_query_index || m.trainIdx == root_train_index))
            {
                //armamos los arcos
                int neighbor_query_x = query_keypoints.at<double>(m.queryIdx,0);
                int neighbor_query_y = query_keypoints.at<double>(m.queryIdx,1);
                double neighbor_query_angle = query_keypoints.at<double>(m.queryIdx,4);
                Edge query_edge(root_query_x, root_query_y, root_query_angle, neighbor_query_x, neighbor_query_y, neighbor_query_angle);
                int neighbor_train_x = train_keypoints.at<double>(m.trainIdx,0);
                int neightbor_train_y = train_keypoints.at<double>(m.trainIdx,1);
                double neighbor_train_angle = train_keypoints.at<double>(m.trainIdx,4);
                Edge train_edge(root_train_x, root_train_y, root_train_angle, neighbor_train_x, neightbor_train_y, neighbor_train_angle);
                if(query_edge.dist > min_dist && train_edge.dist > min_dist)
                {
                    //comparamos los arcos
                    bool similares = false;
                    similares = train_edge.dist*ratio < query_edge.dist && query_edge.dist*ratio < train_edge.dist;
                    //                    && train_edge.alpha*ratio < query_edge.alpha && query_edge.alpha*ratio < train_edge.alpha
                    //                    && train_edge.beta*ratio < query_edge.beta && query_edge.beta*ratio < train_edge.beta;
                    qDebug() << train_edge.dist;
                    qDebug() << query_edge.dist;
                    qDebug() << "---";
                }
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
