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

    //analyzer
    keypoint_threshold = settings.value("keypoint_threshold").toInt();
    keypoint_extractor = string_to_enum(settings.value("keypoint_extractor").toString());
    feature_extractor = string_to_enum(settings.value("feature_extractor").toString());
    draw_over_output = settings.value("draw_features").toBool();

    //comparator
    max_match_dist = settings.value("max_match_dist").toInt();
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
    if(arg1 == "sift") return fp::SIFT;
    if(arg1 == "cn") return fp::CN;


    return not_found;
}
