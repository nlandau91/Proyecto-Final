#include "appsettings.h"

#include <QApplication>
#include <QSettings>

int string_to_enum(const QString arg1);

AppSettings::AppSettings()
{

    load_settings();
}

void AppSettings::load_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);

    //preprocesser
    blk_size = settings.value("blk_size").toInt();
    enhancement_method = string_to_enum(settings.value("enhancement_method").toString());
    thinning_method = string_to_enum(settings.value("thinning_method").toString());
    segment = settings.value("segment").toBool();
    roi_threshold = settings.value("roi_threshold").toFloat();

    //analyzer
    keypoint_threshold = settings.value("keypoint_threshold").toInt();
    minutiae_method = string_to_enum(settings.value("keypoint_extractor").toString());
    descriptor_method = string_to_enum(settings.value("feature_extractor").toString());
    draw_over_output = settings.value("draw_features").toBool();

    //comparator
    matcher_threshold = settings.value("match_thresh").toDouble();
}

int string_to_enum(const QString arg1)
{
    int not_found = -1;

    if(arg1 == "gabor") return fp::GABOR;
    if(arg1 == "none") return fp::NONE;
    if(arg1 == "guohall") return fp::GUOHALL;
    if(arg1 == "zhangsuen") return fp::ZHANGSUEN;
    if(arg1 == "morph") return fp::MORPH;
    if(arg1 == "harris") return fp::HARRIS;
    if(arg1 == "shitomasi") return fp::SHITOMASI;
    if(arg1 == "orb") return fp::ORB;
    if(arg1 == "surf") return fp::SURF;
    if(arg1 == "custom") return fp::CUSTOM;
    if(arg1 == "sift") return fp::SIFT;
    if(arg1 == "cn") return fp::CN;
    if(arg1 == "poincare") return fp::POINCARE;


    return not_found;
}
