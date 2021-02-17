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
        float root_angle = keypoints.at<float>(i,3);
        for(int j = i + 1; j < keypoints.rows; j++)
        {
            //armamos los arcos
            int neighbor_x = keypoints.at<float>(j,0);
            int neighbor_y = keypoints.at<float>(j,1);
            float neighbor_angle = keypoints.at<float>(j,3);
            Edge edge(root_x, root_y, root_angle, neighbor_x, neighbor_y, neighbor_angle);
            if(min_dist < edge.dist && edge.dist < min_dist*5)
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
            double min_dist = 15.0; // minima distancia que debe tener un arco
            std::vector<Edge> query_edges = get_edges(query_keypoints, min_dist);
            std::vector<Edge> train_edges = get_edges(train_keypoints, min_dist);

            //comparamos los arcos
            for(const Edge &e1 : query_edges)
            {
                qDebug() << "dist: " << e1.dist;
                for(const Edge &e2 : train_edges)
                {
                    comparation = e1.compare(e2, edge_dist, edge_angle);
                    if(comparation)qDebug() << "Comparation: " << comparation;
                }
            }
        }

        //filtramos los matches
        std::vector<cv::DMatch> good_matches = matches;
        std::sort(good_matches.begin(), good_matches.end(),
                  [](cv::DMatch const & a, cv::DMatch const & b) -> bool { return a.distance < b.distance; } );
        const float GOOD_MATCH_PERCENT = 0.5f;
        const int numGoodMatches = good_matches.size() * GOOD_MATCH_PERCENT;
        good_matches.erase(good_matches.begin()+numGoodMatches, good_matches.end());

        std::vector<cv::Point2f> q_points;
        std::vector<cv::Point2f> t_points;
        for(const cv::DMatch &m : good_matches)
        {
            cv::Point2f p1(
                        query_keypoints.at<float>(m.queryIdx,0),
                        query_keypoints.at<float>(m.queryIdx,1));
            cv::Point2f p2(
                        train_keypoints.at<float>(m.trainIdx,0),
                        train_keypoints.at<float>(m.trainIdx,1));
            q_points.push_back(p1);
            t_points.push_back(p2);
        }
        //registramos la huella dactilar, para que tenga la misma traslacion y rotacion
        if(q_points.size() > 3)
        {
            cv::Mat H = cv::findHomography(q_points,t_points,cv::RANSAC);

            if(!H.empty())
            {
                cv::Mat in = cv::imread("/home/nico/Proyecto-Final/res/FVC2002/DB1_B/101_2.tif",cv::IMREAD_GRAYSCALE);
                cv::Mat out;
                cv::warpPerspective(in,out,H,in.size());
                cv::imwrite("OUT.TIF",out);
                //            //obtenemos angulo de rotacion de la homografia
                //            //descomposicion SVD de la submatriz 2x2 superior izquierda
                //            cv::Mat w,u,vt;
                //            cv::SVD::compute(H(cv::Rect(0,0,2,2)),w,u,vt);
                //            cv::Mat R = u * vt;

                //            float angle = std::atan2(R.at<float>(1,0),R.at<float>(0,0)) * 180.0 / M_PI;
                float angle2 = - std::atan2(H.at<float>(0,1),H.at<float>(0,0)) * 180.0 / M_PI;
                qDebug() << "angle2: " << angle2;
            }
        }


        //metodo basico de matching, utilizando simplemente la cantidad de matches encontrados entre minutiae
        double score = (double)matches.size()/std::max(query_descriptors.rows,train_descriptors.rows);
        qDebug() << "score: " << score;
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
