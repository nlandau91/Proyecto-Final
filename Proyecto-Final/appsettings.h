#ifndef APPSETTINGS_H
#define APPSETTINGS_H


namespace fp{
/*!
 * \brief El enum Params define las opciones de configuracion disponibles
 * para los metodos configurables del programa
 */
enum Params
{
    GABOR,
    ZHANGSUEN,
    MORPH,
    GUOHALL,
    HARRIS,
    SHITOMASI,
    SURF,
    ORB,
    CUSTOM,
    SIFT,
    BRIEF,
    BRUTEFORCE,
    FLANN,
    CN,
    ENDING,
    BIFURCATION,
    POINCARE,
    LOOP,
    DELTA,
    WHORL,
    ARCH,
    HOMOGRAPHY,
    PARTIALAFFINE,
    AFFINE,

};
/*!
 * \brief La clase AppSettings se encarga de almacenar la configuracion de cargar y almacenar
 * la configuracion externa del programa.
 */
class AppSettings
{
public:
    /*!
     * \brief AppSettings constructor
     */
    AppSettings();

    /*!
     * \brief load_settings carga la configuracion del archivo de configuracion
     */
    void load_settings();

    //preprocess
    int enhancement_method;
    int thinning_method;
    bool segment;
    int blk_size;
    float roi_threshold;
    int blk_freq;
    int blk_segm;
    int blk_orient;
    float gabor_kx;
    float gabor_ky;

    //analyzer
    int keypoint_detector;
    int keypoint_threshold;
    int descriptor_method;

    //comparator
    int matcher_method;
    double matcher_threshold;
    double ransac_threshold;
    int ransac_model;

    bool draw_over_output;

};
}

#endif // APPSETTINGS_H
