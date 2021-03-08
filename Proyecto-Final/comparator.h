#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "common.h"
#include "edge.h"
#include "triangle.h"

namespace fp{

class Comparator
{
public:
    Comparator();

    /*!
     * \brief compare compara los descriptores de consulta contra una lista de descriptores de comparacio
     * Adicionalmente utiliza keypoints personalizados para realizar un test de bordes para mejorar la deteccion
     * \param query_descriptors descriptores de consulta
     * \param train_descriptors_list descriptores de comparacion
     * \param query_keypoints keypoints personalizados de consulta
     * \param train_keypoints_list keypoints personalizados de comparacion
     * \param threshold minimo score de similitud necesario para considerar una comparacion positiva.
     * \return true si se consideran suficientemente iguales, falso en caso contrario
     */
    bool compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, const cv::Mat &query_keypoints, const std::vector<cv::Mat> &train_keypoints_list);

    bool compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, const cv::Mat &query_keypoints, const cv::Mat &train_keypoints, double threshold = 0.1);



    int matcher_method;

    double match_threshold;

    bool edge_matching;

    double edge_dist;

    double edge_angle;

    double triangle_max_edge;
    double triangle_min_edge;
};
}
#endif // COMPARATOR_H
