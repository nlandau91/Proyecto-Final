#include "appsettings.h"

#include <QApplication>

int string_to_enum(const QString arg1);

AppSettings::AppSettings()
{

    QString file = QApplication::applicationDirPath()+"/settings.ini";
    QSettings settings(file, QSettings::IniFormat);

    enhancement_method = (fp::Preprocesser::EnhancementMethod)string_to_enum(settings.value("enhancement_method").toString());
    thinning_method = (fp::Preprocesser::ThinningMethod)string_to_enum(settings.value("thinning_method").toString());
    masking = settings.value("masking").toBool();
}

int string_to_enum(const QString arg1)
{
    int to_return = -1;

    if(arg1 == "gabor")
    {
        return fp::Preprocesser::EnhancementMethod::GABOR;
    }
    if(arg1 == "none")
    {
        return fp::Preprocesser::EnhancementMethod::NONE;
    }
    if(arg1 == "guohall")
    {
        return fp::Preprocesser::ThinningMethod::GUOHALL;
    }
    if(arg1 == "zhangsuen")
    {
        return fp::Preprocesser::ThinningMethod::ZHANGSUEN;
    }
    if(arg1 == "morph")
    {
        return fp::Preprocesser::ThinningMethod::MORPH;
    }
    return to_return;
}
