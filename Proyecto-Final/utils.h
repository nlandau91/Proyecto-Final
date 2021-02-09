#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QPixmap>
#include "common.h"

namespace fp
{

/*!
 * \brief cvMatToQImage convierte una imagen de tipo cv::Mat a una de tipo QImage
 * \param inMat imagen de tipo cv::Mat a convertir
 * \return imagen en formato QImage
 */
QImage cvMatToQImage(const cv::Mat &inMat);

/*!
 * \brief cvMatToQPixmap convierte una imagen de tipo cv::Mat a una de tipo QPixmap
 * \param inMat imagen de tipo cv::Mat a convertir
 * \return imagen en formato QPixmap
 */
QPixmap cvMatToQPixmap(const cv::Mat &inMat );

/*!
 * \brief draw_singularities dibuja sobre la imagen las singularidades pasadas por parametro
 * \param src imagen donde se dibujaran las singularidades
 * \param singularities vector de singularidades que se dibujaran
 * \return la imagen dibujada
 */
cv::Mat draw_singularities(const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities);

/*!
 * \brief draw_minutiae dibuja sobre la imagen las minucias pasadas por parametro
 * \param src imagen donde se dibujaran las minucias
 * \param mintiaes vector de minucias que se dibujaran
 * \return la imagen dibujada
 */
cv::Mat draw_minutiae(const cv::Mat &src, const std::vector<cv::KeyPoint> &mintiaes);

/*!
 * \brief mat_cos calcula el coseno de cada pixel de la imagen
 * \param src imagen de unico canal
 * \return la imagen con el coseno de sus componentes
 */
cv::Mat mat_cos(const cv::Mat &src);

/*!
 * \brief mat_sin calcula el seno de cada pixel de la imagen
 * \param src imagen de unico canal
 * \return la imagen con el seno de sus componentes
 */
cv::Mat mat_sin(const cv::Mat &src);

/*!
 * \brief mat_atan2 calcula el arcotangente de cada par de pixel de la imagen
 * \param src1 imagen cuyas componentes son el primer parametro del arcotangente
 * \param src2 imagen cuyas componentes son el segundo parametro del arcotangente
 * \return devuelve la imagen con sus compoentes igual al arcotangente de las imagenes pasadas
 */
cv::Mat mat_atan2(const cv::Mat &src1, const cv::Mat &src2);

/*!
 * \brief unique arma un vector con cada valor de la imagen una unica vez
 * \param input imagen donde se buscaran cada valor
 * \param sort flag que determina si se ordena la salida
 * \return vector con los valores de la imagen sin repetir
 */
std::vector<float> unique(const cv::Mat &input, bool sort = false);

/*!
 * \brief visualize_angles arma una imagen a partr de un mapa de orientacion
 * \param im imagen original
 * \param angles mapa de orientaciones
 * \return imagen donde se visualizan los angulos
 */
cv::Mat visualize_angles(const cv::Mat &im, const cv::Mat &angles);

/*!
 * \brief apply_mask aplica una mascara a una imagen
 * \param src imagen que sera enmascarada
 * \param mask mascara a aplicar
 * \return imagen luego de ser enmascarada
 */
cv::Mat apply_mask(const cv::Mat &src, const cv::Mat &mask);

/*!
 * \brief translate_mat realiza una translacion a la imagen
 * \param img imagen que sera transladada
 * \param offsetx offset horizontal en pixels
 * \param offsety offset vertical en pixels
 * \return imagen transladada
 */
cv::Mat translate_mat(const cv::Mat &img, int offsetx, int offsety);

/*!
 * \brief write_descriptor guarda un descriptor en disco
 * \param file nombre del archivo donde se guardara
 * \param descriptor descriptor a guardar
 * \return true si se realizo con exito
 */
bool serialize_mat(const std::string& file, const cv::Mat& mat, const std::string &name = "serialized_mat");

/*!
 * \brief read_descriptor lee un descriptor del disco
 * \param file nombre del archivo a leer
 * \param descriptor mat donde se copiara el descriptor leido
 * \return true si se realizo con exito
 */
bool serialize_mat(const std::string& file, cv::Mat& mat, const std::string &name = "serialized_mat");

}

#endif // UTILS_H
