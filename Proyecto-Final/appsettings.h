#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "common.h"

class AppSettings
{
public:
    AppSettings();
private:

    fp::Preprocesser::EnhancementMethod enhancement_method;
    fp::Preprocesser::ThinningMethod thinning_method;
    bool masking;

};

#endif // APPSETTINGS_H
