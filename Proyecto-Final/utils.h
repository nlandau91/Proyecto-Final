#ifndef UTILS_H
#define UTILS_H

#include <opencv2/core.hpp>
#include <QImage>
#include <QPixmap>


namespace fp
{

/*!
 * \brief cvMatToQImage convierte una imagen de tipo cv::Mat a una de tipo QImage
 * \param inMat imagen de tipo cv::Mat a convertir
 * \return imagen en formato QImage
 */
QImage cv_mat_to_qimage(const cv::Mat &inMat);

/*!
 * \brief cvMatToQPixmap convierte una imagen de tipo cv::Mat a una de tipo QPixmap
 * \param inMat imagen de tipo cv::Mat a convertir
 * \return imagen en formato QPixmap
 */
QPixmap cv_mat_to_qpixmap(const cv::Mat &inMat );

/*!
 * \brief draw_singularities dibuja sobre la imagen las singularidades pasadas por parametro
 * \param src imagen donde se dibujaran las singularidades
 * \param singularities vector de singularidades que se dibujaran
 * \return la imagen dibujada
 */
cv::Mat draw_singularities(const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities);

/*!
 * \brief draw_keypoints dibuja sobre la imagen los keypoints pasadas por parametro
 * \param src imagen donde se dibujaran os keypoints
 * \param keypoints vector de keypoints que se dibujaran
 * \return la imagen dibujada
 */
cv::Mat draw_keypoints(const cv::Mat &src, const std::vector<cv::KeyPoint> &keypoints);

/*!
 * \brief mat_cos calcula el coseno de cada pixel de la imagen
 * \param src imagen de unico canal
 * \return la imagen con el coseno de sus componentes
 */
cv::Mat mat_cos(const cv::Mat &src);

/*!
 * \brief visualize_angles arma una imagen a partr de un mapa de orientacion
 * \param im imagen original
 * \param angles mapa de orientaciones
 * \return imagen donde se visualizan los angulos
 */
cv::Mat visualize_angles(const cv::Mat &im, const cv::Mat &angles);



}

#endif // UTILS_H
