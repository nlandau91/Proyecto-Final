#include "comparator.h"
#include <QDebug>
#include <iostream>

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
        for(int j = i + 1; j < keypoints.rows; j++)
        {
            //armamos los arcos
            int neighbor_x = keypoints.at<float>(j,0);
            int neighbor_y = keypoints.at<float>(j,1);
            Edge edge(root_x, root_y, neighbor_x, neighbor_y);
            if(min_dist < edge.length && edge.length < min_dist*5)
            {
                edges.push_back(edge);
            }
        }
    }
    return edges;
}

std::vector<Triangle> get_triangles(const cv::Mat &keypoints, double min_dist = 5.0, double max_dist = 50)
{
    std::vector<Triangle> triangles;
    for(int i = 0; i < keypoints.rows - 2; i++)
    {
        //calculamos la raiz
        int x0 = keypoints.at<float>(i,0);
        int y0 = keypoints.at<float>(i,1);
        for(int j = i + 1; j < keypoints.rows - 1; j++)
        {
            //armamos el primer arco
            int x1 = keypoints.at<float>(j,0);
            int y1 = keypoints.at<float>(j,1);
            Edge edge1(x0, y0, x1, y1);
            if(min_dist < edge1.length && edge1.length < max_dist)
            {
                for(int k = j + 1; k < keypoints.rows; k++)
                {
                    //armamos el segundo y tercer arco
                    int x2 = keypoints.at<float>(k,0);
                    int y2 = keypoints.at<float>(k,1);
                    Edge edge2(x0, y0, x2, y2);
                    Edge edge3(x1, y1, x2, y2);
                    if(min_dist < edge2.length && edge2.length < max_dist
                            && min_dist < edge3.length && edge3.length < max_dist)
                    {
                        triangles.push_back(Triangle(edge1,edge2,edge3));
                    }
                }
            }
        }
    }
    return triangles;
}


bool Comparator::compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, const cv::Mat &query_keypoints, const cv::Mat &train_keypoints, double threshold)
{
    std::vector<cv::DMatch> matches;
    switch(matcher_method)
    {
    case ORB:
    {
        //matches = find_matches(query_descriptors,train_descriptors,cv::NORM_HAMMING,0.75);
        matches = find_matches(query_descriptors,train_descriptors,cv::NORM_HAMMING);
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
        //metodo de edge matching
        //arma arcos que contienen distancia y angulo de cada nodo(minutiae)
        //en teoria es invariante a traslacion y rotacion, por lo que sirve como metodo de matching
        if(edge_matching)
        {
            //armamos los arcos
            std::vector<Triangle> query_triangles = get_triangles(query_keypoints, triangle_min_edge, triangle_max_edge);
            std::vector<Triangle> train_triangles = get_triangles(train_keypoints, triangle_min_edge, triangle_max_edge);
            qDebug() << "Comparator: query_triangles: " << query_triangles.size();

            //comparamos los arcos
            int positives = 0;
            for(const Triangle &t1 : query_triangles)
            {
                for(const Triangle &t2 : train_triangles)
                {
                    bool comparation = t1.compare(t2,edge_angle,edge_dist);
                    if(comparation)
                    {
                       positives ++;
                    }
                    else
                    {
                    }
                }
            }
            qDebug() << "Comparator: positives: " << positives;
        }

        //metodo basico de matching, utilizando simplemente la cantidad de matches encontrados entre minutiae
        //double score = (double)matches.size()/std::max(query_descriptors.rows,train_descriptors.rows);
        double score = (double)matches.size()/((query_descriptors.rows+train_descriptors.rows)/2.0);
        qDebug() << "Comparator: score avg: " << score;
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
