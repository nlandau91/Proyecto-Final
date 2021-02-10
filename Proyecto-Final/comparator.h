#ifndef COMPARATOR_H
#define COMPARATOR_H

#include "common.h"

namespace fp{

class Comparator
{
public:
    Comparator();

    /*!
     * \brief compare compara los descriptores de consulta contra una lista de descriptores de comparacion
     * \param query_descriptors descriptores de consulta
     * \param train_descriptors_list descriptores de comparacion
     * \param threshold minimo score de similitud necesario para considerar una comparacion positiva.
     * \return true si se consideran suficientemente iguales, falso en caso contrario
     */
    bool compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, double threshold = 0.1);

    bool compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, double threshold = 0.1);

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
    bool compare(const cv::Mat &query_descriptors, const std::vector<cv::Mat> &train_descriptors_list, const cv::Mat &query_keypoints, const std::vector<cv::Mat> &train_keypoints_list, double threshold);

    bool compare(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors, const cv::Mat &query_keypoints, const cv::Mat &train_keypoints, double threshold = 0.1);

    bool compare_edges(const cv::Mat &query_descriptors, const cv::Mat &train_descriptors);

    int matcher_method;
};
}
#endif // COMPARATOR_H
