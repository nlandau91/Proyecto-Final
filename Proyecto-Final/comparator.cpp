#include "comparator.h"
#include <QDebug>
#include <iostream>

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

//metodo de edge matching
//arma arcos que contienen distancia y angulo de cada nodo(minutiae)
//en teoria es invariante a traslacion y rotacion, por lo que sirve como metodo de matching
int triangle_matching(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template,double triangle_min_edge, double triangle_max_edge,double edge_angle,double edge_dist)
{
    //armamos los arcos
    std::vector<Triangle> query_triangles = get_triangles(query_template.descriptors, triangle_min_edge, triangle_max_edge);
    std::vector<Triangle> train_triangles = get_triangles(train_template.descriptors, triangle_min_edge, triangle_max_edge);
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
    return positives;
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

std::vector<cv::DMatch> remove_outliers_ransac(const std::vector<cv::KeyPoint> &query_keypoints, const std::vector<cv::KeyPoint> &train_keypoints, const std::vector<cv::DMatch> &matches, double ransac_threshold = 3.0)
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
    cv::findHomography(src_points,dst_points,cv::RANSAC, ransac_threshold ,mask);

    //cv::estimateAffine2D(src_points,dst_points,mask,cv::RANSAC,ransac_threshold);

    for(int i = 0; i < mask.rows; i++)
    {
        if(mask.at<uchar>(i,0) > 0)
        {
            good_matches.push_back(matches[i]);
        }
    }
    return good_matches;
}

bool Comparator::compare(const FingerprintTemplate &query_template, const FingerprintTemplate &train_template)
{
    qDebug() << "Comparator: Realizando matches entre keypoints...";
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
    bool comparation = false;
    if(matches.size() > 1)
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
        inliners_median = remove_outliers_median(matches,2.5);
        std::vector<cv::DMatch> inliners_ransac;
        inliners_ransac = remove_outliers_ransac(query_template.keypoints,train_template.keypoints,inliners_median,ransac_threshold);
        std::vector<cv::DMatch> good_matches = inliners_ransac;

        //metodo basico de matching, utilizando simplemente la cantidad de matches encontrados entre minutiae
        double score = (double)good_matches.size()/std::max(query_template.descriptors.rows,train_template.descriptors.rows);
        double score2 = (double)good_matches.size()/((query_template.descriptors.rows+train_template.descriptors.rows)/2.0);
        qDebug() << "Comparator: score: " << score;
        qDebug() << "Comparator: score2: " << score2;
        comparation = score > match_threshold;

    }

    return comparation;
}

bool Comparator::compare(const FingerprintTemplate &query_template, const std::vector<FingerprintTemplate> &train_templates)
{
    bool comparation = false;
    //qDebug() << "Comparator: obteniendo matches...";
    int i = 0;
    for(const FingerprintTemplate &train_template : train_templates)
    {
        comparation = comparation || compare(query_template, train_template);
        i++;
    }

    return comparation;
}


