#include "appsettings.h"

#include <QApplication>
#include <QSettings>
#include <QFile>

using namespace fp;

int string_to_enum(const QString arg1);

AppSettings::AppSettings()
{

    load_settings();
}

void AppSettings::load_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    if(!QFile::exists(file))
    {
        file = QApplication::applicationDirPath()+"/default.ini";
    }
    QSettings settings(file, QSettings::IniFormat);

    //preprocesser
    blk_size = settings.value("blk_size").toInt();
    enhancement_method = string_to_enum(settings.value("enhancement_method").toString());
    thinning_method = string_to_enum(settings.value("thinning_method").toString());
    segment = settings.value("segment").toBool();
    roi_threshold = settings.value("roi_threshold").toFloat();
    blk_freq = settings.value("blk_freq").toInt();
    blk_segm = settings.value("blk_segm").toInt();
    blk_orient = settings.value("blk_orient").toInt();
    gabor_kx = settings.value("gabor_kx").toFloat();
    gabor_ky = settings.value("gabor_ky").toFloat();

    //analyzer
    keypoint_threshold = settings.value("keypoint_threshold").toInt();
    keypoint_detector = string_to_enum(settings.value("keypoint_detector").toString());
    descriptor_method = string_to_enum(settings.value("feature_extractor").toString());
    draw_over_output = settings.value("draw_features").toBool();

    //comparator
    matcher_threshold = settings.value("match_thresh").toDouble();
    ransac_threshold = settings.value("ransac_threshold").toDouble();
    ransac_model = string_to_enum(settings.value("ransac_model").toString());

}

int string_to_enum(const QString arg1)
{
    int not_found = -1;

    if(arg1 == "gabor") return GABOR;
    if(arg1 == "guohall") return GUOHALL;
    if(arg1 == "zhangsuen") return ZHANGSUEN;
    if(arg1 == "morph") return MORPH;
    if(arg1 == "harris") return HARRIS;
    if(arg1 == "shitomasi") return SHITOMASI;
    if(arg1 == "orb") return ORB;
    if(arg1 == "surf") return SURF;
    if(arg1 == "custom") return CUSTOM;
    if(arg1 == "sift") return SIFT;
    if(arg1 == "cn") return CN;
    if(arg1 == "poincare") return POINCARE;
    if(arg1 == "similarity") return PARTIALAFFINE;
    if(arg1 == "affine") return AFFINE;
    if(arg1 == "projective") return HOMOGRAPHY;

    return not_found;
}

