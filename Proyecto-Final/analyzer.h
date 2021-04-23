#ifndef ANALYZER_H
#define ANALYZER_H

#include "preprocessed.h"

#include "utils.h"
#include "fingerprinttemplate.h"
namespace fp
{
/*!
 * La clase Analyzer se encarga de extraer las caracteristicas de una
 * imagen de huella dactilar y armar el template a partir de las mismas
 */
class Analyzer
{
public:
    int keypoint_method;
    int keypoint_threshold;
    int descriptor_method;
    int max_match_dist;
    int l1_features_method;
    float poincare_tol;
    int blk_sze;

    /*!
     * \brief Analyzer constructor
     */
    Analyzer();

    /*!
     * \brief analize analiza la huella dactilar preprocesada
     * \param preprocessed estructura que contiene el preprocesamiento hecho a la huella dactilar
     * \return devuelve una estructura que contiene el analisis hecho
     */
    FingerprintTemplate analize(const Preprocessed &preprocessed);

private:

    /*!
     * \brief find_l1_features encuentra los puntos singulares de una huella
     * \param pre estructura que contiene el preprocesamiento hecho a la huella dactilar
     * \return vector con los keypoints donde se encuentran las singularidades encontradas
     */
    std::vector<cv::KeyPoint> find_l1_features(const Preprocessed &pre);

    /*!
     * \brief get_keypoints encuentra puntos de interes en la huella dactilar
     * \param pre estructura que contiene el preprocesamiento hecho a la huella dactilar
     * \returnvector con los keypoints donde se encuentran los puntos de interes encontrados
     */
    std::vector<cv::KeyPoint> get_keypoints(const Preprocessed &pre);

    /*!
     * \brief calcular_descriptors calcula los decriptores para los puntos claves
     * \param src imagen de la que se extraera informacion de los puntos clave
     * \param keypoints puntos de interes a los que se le calcularan sus descriptores
     * \return Mat que contiene los descriptores
     */
    cv::Mat calcular_descriptors(const cv::Mat &src, std::vector<cv::KeyPoint> &keypoints);
};
}
#endif // ANALYZER_H
