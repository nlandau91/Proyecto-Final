#include "appsettings.h"

#include <QApplication>

int string_to_enum(const QString arg1);

AppSettings::AppSettings()
{

    load_settings();
}

void AppSettings::load_settings()
{
    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);

    enhancement_method = (fp::Preprocesser::EnhancementMethod)string_to_enum(settings.value("enhancement_method").toString());
    thinning_method = (fp::Preprocesser::ThinningMethod)string_to_enum(settings.value("thinning_method").toString());
    masking = settings.value("masking").toBool();
    keypoint_threshold = settings.value("keypoint_threshold").toInt();
    max_match_dist = settings.value("max_match_dist").toInt();
    keypoint_extractor = (fp::Analyzer::KeypointExtractor)string_to_enum(settings.value("keypoint_extractor").toString());
    feature_extractor = (fp::Analyzer::FeatureExtractor)string_to_enum(settings.value("feature_extractor").toString());
}

int string_to_enum(const QString arg1)
{
    int not_found = -1;

    if(arg1 == "gabor") return fp::Preprocesser::EnhancementMethod::ENH_GABOR;
    if(arg1 == "none") return fp::Preprocesser::EnhancementMethod::ENH_NONE;
    if(arg1 == "guohall") return fp::Preprocesser::ThinningMethod::THI_GUOHALL;
    if(arg1 == "zhangsuen") return fp::Preprocesser::ThinningMethod::THI_ZHANGSUEN;
    if(arg1 == "morph") return fp::Preprocesser::ThinningMethod::THI_MORPH;
    if(arg1 == "harris") return fp::Analyzer::KeypointExtractor::HARRIS;
    if(arg1 == "shitomasi") return fp::Analyzer::KeypointExtractor::SHITOMASI;
    if(arg1 == "ksurf") return fp::Analyzer::KeypointExtractor::KSURF;
    if(arg1 == "orb") return fp::Analyzer::FeatureExtractor::ORB;
    if(arg1 == "dsurf") return fp::Analyzer::FeatureExtractor::DSURF;
    if(arg1 == "dsift") return fp::Analyzer::FeatureExtractor::DSIFT;


    return not_found;
}
