#ifndef FINGERPRINTTEMPLATE_H
#define FINGERPRINTTEMPLATE_H

#include <opencv2/core.hpp>
#include <string>

namespace fp
{
/*!
 * \brief The FingerprintTemplate class modela la plantilla de una huella dactilar
 */
class FingerprintTemplate
{
public:
    /*!
     * \brief FingerprintTemplate inicializador vacio, es necesario cargar los miembros
     * o utilizar la funcion deserialize
     */
    FingerprintTemplate();
    /*!
     * \brief FingerprintTemplate inicializa con la plantilla cargada desde file
     * \param file archivo desde el cual se cargara la plantilla
     */
    FingerprintTemplate(const std::string& file);
    /*!
     * \brief serialize almacena la plantilla en el archivo pasado por parametro
     * \param file archivo que contendra la plantilla
     * \return true si se pudo realizar correctamente, false en caso contrario
     */
    bool serialize(const std::string& file) const;
    /*!
     * \brief deserialize carga la plantilla almacenada en el archivo pasado por parametro
     * \param file archivo que contiene la plantilla a cargar
     * \return true si se realizo correctamente, false en caso contraro
     */
    bool deserialize(const std::string& file);

    cv::Mat descriptors;
    std::vector<cv::KeyPoint> keypoints;
    std::vector<cv::KeyPoint> singularities;
    std::vector<cv::KeyPoint> minutiaes;
};
}

#endif // FINGERPRINTTEMPLATE_H
